#ifndef _GAME_ENGINE_SRC_MODULE_H_
#define _GAME_ENGINE_SRC_MODULE_H_

#include <csignal>

#include <utility>
#include <vector>
#include <tuple>
#include <string>
#include <algorithm>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <future>

//#include "external/include/concurrentqueue.h"
#include "external/include/readerwriterqueue.h"

class module {
 public:
  class eventbase {
   public:
    virtual void dispatch(module*) = 0;
    std::string ModuleName;
    std::string FunctionName;
    bool UseMainThread;
  };
  template<typename... args>
  class event : public eventbase {
   public:
    event(std::string Module, std::string Function, bool MainThread, args... Args) { 
      ModuleName = Module;
      FunctionName = Function;
      UseMainThread = MainThread;
      Arguments = std::make_tuple(Args...);
    }
    virtual void dispatch(module* Module) {
      Module->call(FunctionName, Arguments);
    }
    std::tuple<args...> Arguments;
  };
  module() : Available(true) {
    //signal(SIGSEGV, module::abort);
  }
  void prestart(std::string* Name) {
    ModuleName = Name;
    start();
  }
  bool isAvailable() {
    //Available.compare_exchange_strong(True, False); // Not working
    if (Available.load() == true) {
      Available.store(false);
      return true;
    } else return false;
  }
  template<typename... args>
    void call(std::string FunctionName, std::tuple<args...> Arguments) {
      auto FunctionIterator = std::find_if(FunctionArray.begin(), FunctionArray.end(),
          [&](const std::unique_ptr<functionbase>& Function) { return Function->FunctionName == FunctionName; });
      if (FunctionIterator != FunctionArray.end()) {
        if (!(*FunctionIterator)->IsBusy.load()) {
          (*FunctionIterator)->IsBusy.store(true);
          (*FunctionIterator)->Arguments = reinterpret_cast<std::tuple<>*>(&Arguments);
          ((*FunctionIterator)->execute)();
          (*FunctionIterator)->IsBusy.store(false);
      }} else {
        std::string Log = "Function '" + FunctionName + "' not found.";
        request("Logger", "log", *ModuleName, Log);
      }
      setAvailable();
      Condition.notify_one();
    }
  std::unique_ptr<eventbase> getRequest() {
    std::unique_ptr<eventbase> NextEvent;
    if (EventQueue.try_dequeue(NextEvent))
      return NextEvent;
    else return nullptr;
    //if (!EventQueue.empty()) {
      //std::unique_ptr<eventbase> NextEvent = std::move(EventQueue.front());
      //EventQueue.pop();
      //return std::move(NextEvent);
    //} else return nullptr;
  }
  std::unique_ptr<eventbase> getImmediateRequest() {
    std::unique_ptr<eventbase> NextEvent;
    if (ImmediateQueue.try_dequeue(NextEvent))
      return NextEvent;
    else return nullptr;
  }
  void preShutDown() {
    Condition.notify_all();
    shutDown();
  }
  virtual ~module() {
  }
 protected:
  virtual void start() =0;
  template<typename modulechild, typename... args>
    void addFunction(std::string FunctionName, void(modulechild::* FunctionAddress)(args...)) {
      std::unique_ptr<functionbase> Function(new function<modulechild, args...>(reinterpret_cast<modulechild*>(staticPointer(nullptr)), FunctionName, FunctionAddress));
      FunctionArray.push_back(std::move(Function));
    }
  template<typename ...args>
    void request(const std::string ModuleName, const std::string FunctionName, args... Arguments) {
      std::unique_ptr<eventbase> EventVar(new event<args...>(ModuleName, FunctionName, false, Arguments...));
      //EventQueue.push(std::move(EventVar));
      EventQueue.enqueue(std::move(EventVar));
    }
  template<typename T, typename ...args>
    void immediateRequest(const std::string ModuleName, const std::string FunctionName, T* Value, args... Arguments) {
      std::promise<T> Promise;
      std::future<T> Future = Promise.get_future();
      immediateRequestHelper(ModuleName, FunctionName, &Promise, Arguments...);
      std::unique_lock<std::mutex> Lock(Mutex);
      setAvailable();
      *Value = Future.get();
      Condition.wait(Lock, [&]{return Available.compare_exchange_strong(True, False);});
    }
  template<typename ...args>
    void immediateRequestHelper(const std::string ModuleName, const std::string FunctionName, args... Arguments) {
      std::unique_ptr<eventbase> EventVar(new event<args...>(ModuleName, FunctionName, false, Arguments...));
      ImmediateQueue.enqueue(std::move(EventVar));
    }
  template<typename modulechild>
    void setName(std::string Name, modulechild* Pointer) {
      *ModuleName = Name;
      staticPointer(reinterpret_cast<module*>(Pointer));
    }

  // Think about it. Useless except for logging.
  std::string getName() {
    return *ModuleName;
  }
  void setAvailable() {
    Available.store(true);
  }
  void setUnavailable() {
    Available.store(false);
  }
  virtual void shutDown() =0;
 private:
  class functionbase {
   public:
    virtual void execute() =0;

    std::string FunctionName;
    std::tuple<>* Arguments;
    std::atomic<bool> IsBusy;
  };
  template <typename modulechild, typename ...args>
    class function : public functionbase {
     public:
      function(modulechild* Pointer, std::string Name, void(modulechild::* Address)(args...)) {
        FunctionName = Name;
        ModulePointer = Pointer;
        FunctionPointer = Address;
      }
      virtual void execute() {
        dispatch(*reinterpret_cast<std::tuple<args...>*>(Arguments));
      }
     private:
      template<template<typename...> class params, std::size_t... I>
        void dispatchHelper(params<args...> const& Parameters, std::index_sequence<I...>) {
          (ModulePointer->*FunctionPointer)(std::get<I>(Parameters)...);
          Arguments = nullptr; //Just in case.
        }
      template<template<typename...> class params>
        void dispatch(params<args...> const& Parameters) {
          dispatchHelper(Parameters, std::index_sequence_for<args...>{});
        }

      modulechild* ModulePointer;
      void (modulechild::* FunctionPointer)(args...);
    };
    static module* staticPointer(module* ChildPointer) {
      static module* ModulePointer;
      if (ChildPointer != nullptr)
        ModulePointer = ChildPointer;
      else return ModulePointer;
    }
/*  static void abort(int SigNum) {
    std::string Log = "Crashed with interrupt signal " + std::to_string(SigNum) + ".";
    staticPointer(nullptr)->request("Logger", "log", *staticPointer(nullptr)->ModuleName, Log);
    *staticPointer(nullptr)->ModuleName = " ";
    staticPointer(nullptr)->Available = true;
    staticPointer(nullptr)->request("Window", "quit");
    std::this_thread.detach();
  }*/

  std::atomic<bool> Available;
  bool True = true;
  bool False = false;
  std::mutex Mutex;
  std::condition_variable Condition;
  std::string* ModuleName;
  //std::queue<std::unique_ptr<eventbase>> EventQueue;
  //moodycamel::ConcurrentQueue<std::unique_ptr<eventbase>> EventQueue;
  moodycamel::ReaderWriterQueue<std::unique_ptr<eventbase>> EventQueue;
  moodycamel::ReaderWriterQueue<std::unique_ptr<eventbase>> ImmediateQueue;
  std::vector<std::unique_ptr<functionbase>> FunctionArray;
};

#endif // _GAME_ENGINE_SRC_MODULE_H_
