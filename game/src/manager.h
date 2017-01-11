#ifndef _GAME_ENGINE_SRC_MANAGER_H_
#define _GAME_ENGINE_SRC_MANAGER_H_

#include <cassert>

#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <tuple>
#include <queue>
#include <atomic>

#include "modules/module.h"
#include "classes/thread_manager.h"

class manager {
 public:
  manager();
  void start();
  void addModule(std::unique_ptr<module>&);
  template<typename ...args>
    void request(const std::string ModuleName, const std::string FunctionName, bool UseMainThread, args... Arguments) {
      std::unique_ptr<module::eventbase> EventVar(new module::event<args...>(ModuleName, FunctionName, UseMainThread, Arguments...));
      EventQueue.push(std::move(EventVar));
    }
  void shutDown();
  ~manager();
 private:
  struct modulestruct {
    std::unique_ptr<module> ModulePointer;
    std::string ModuleName;
  };

  void managerLoop();
  void getRequests();

  std::vector<modulestruct> ModuleArray;
  std::queue<std::unique_ptr<module::eventbase>> EventQueue;
  threadmanager ThreadManager;

  bool AllowAddModule;
  std::atomic<bool> ManagerLoopInterrupt;
  
  static bool IsInstantiated;
};

#endif // _GAME_ENGINE_SRC_MANAGER_H_
