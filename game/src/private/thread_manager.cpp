#include "classes/thread_manager.h"

threadmanager::threadmanager() : StopThreads(false) {
  MainThread = std::thread([&] {
      std::unique_ptr<module::eventbase> Event;
      module* Module;
      while(true) {{
        std::unique_lock<std::mutex> Lock(MT_Mutex);
        MT_Condition.wait(Lock, [&]{ return StopThreads.load() || MT_EventPointer != nullptr; });
        if(StopThreads.load() && MT_EventPointer == nullptr)
          return;
        Event = std::move(MT_EventPointer);
        Module = MT_ModulePointer;
      }
      Event->dispatch(Module);}});
  for(int i = 0; i < 4; i++)
    ThreadPool.emplace_back([&] {
      std::unique_ptr<module::eventbase> Event;
      module* Module;
      while(true) {{
        std::unique_lock<std::mutex> Lock(TP_Mutex);
        TP_Condition.wait(Lock, [&]{ return StopThreads.load() || TP_EventPointer != nullptr; });
        if(StopThreads.load() && TP_EventPointer == nullptr)
          return;
        Event = std::move(TP_EventPointer);
        Module = TP_ModulePointer;
      }
      Event->dispatch(Module);}});
}

bool threadmanager::TPisFull() {
  std::lock_guard<std::mutex> Lock(TP_Mutex);
  if (TP_EventPointer != nullptr) {
    TP_Condition.notify_one();
    return true;
  }
  else return false;
}

bool threadmanager::MTisBusy() {
  std::lock_guard<std::mutex> Lock(MT_Mutex);
  if (MT_EventPointer != nullptr) {
    MT_Condition.notify_one();
    return true;
  }
  else return false;
}

void threadmanager::dispatch(std::unique_ptr<module::eventbase> Event, module* Module) {{
      std::lock_guard<std::mutex> Lock(TP_Mutex);
      TP_EventPointer = std::move(Event);
      TP_ModulePointer = Module;
    }
    TP_Condition.notify_one();
}

void threadmanager::dispatchToMain(std::unique_ptr<module::eventbase> Event, module* Module) {{
      std::lock_guard<std::mutex> Lock(MT_Mutex);
      MT_EventPointer = std::move(Event);
      MT_ModulePointer = Module;
    }
    MT_Condition.notify_all();
}

void threadmanager::stop() {
  StopThreads.store(true);
  MT_Condition.notify_all();
  TP_Condition.notify_all();
  MainThread.join();
  for(std::thread& Worker : ThreadPool)
    Worker.join();
}

threadmanager::~threadmanager() {}
