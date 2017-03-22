#include "manager.h"

bool manager::IsInstantiated = false;

manager::manager() : ThreadManager() {
  assert(!IsInstantiated);
  IsInstantiated = true;
  AllowAddModule = true;
  ManagerLoopInterrupt = false;
}

void manager::start() {
  AllowAddModule = false;
  for (auto& ModuleIterator : ModuleArray) {
    ModuleIterator.ModulePointer->prestart(&ModuleIterator.ModuleName);
  }
  request("Window", "setInterrupt", false, &ManagerLoopInterrupt);
  managerLoop();
}

void manager::addModule(std::unique_ptr<module>& ModulePointer) {
  if (AllowAddModule) {
    modulestruct ModuleStructVar;
    ModuleStructVar.ModulePointer = std::move(ModulePointer);
    ModuleArray.push_back(std::move(ModuleStructVar));
  }
}

void manager::shutDown() {
  ThreadManager.stop();
  for (auto& ModuleIterator : ModuleArray) {
    ModuleIterator.ModulePointer->shutDown();
  }
  AllowAddModule = true;
}

manager::~manager() {
  IsInstantiated = false;
}

void manager::managerLoop() {
  bool Once = true;
  std::unique_ptr<module::eventbase> NextEvent;
  std::vector<modulestruct>::iterator EventIterator;
  while (/*!ManagerLoopInterrupt*/ true) {
    if (NextEvent == nullptr && !EventQueue.empty()) {
      NextEvent = std::move(EventQueue.front());
      EventQueue.pop();
      EventIterator = std::find_if(ModuleArray.begin(), ModuleArray.end(), 
              [&] (const modulestruct& ModuleStructVar) {
                return ModuleStructVar.ModuleName == NextEvent->ModuleName;});
    } else if (EventQueue.empty()) {
        request("Renderer", "draw", true);
        request("Window", "refresh", false);
        request("Client", "update", false); //It has to happen at the beginning of frame, to update everything in time.
        continue;
    }
    if (EventIterator != ModuleArray.end()) {
      if (NextEvent->UseMainThread && !ThreadManager.MTisBusy() && EventIterator->ModulePointer->isAvailable()) {
        ThreadManager.dispatchToMain(std::move(NextEvent), EventIterator->ModulePointer.get());
      }
      else if (!NextEvent->UseMainThread && !ThreadManager.TPisFull() && EventIterator->ModulePointer->isAvailable())
        ThreadManager.dispatch(std::move(NextEvent), EventIterator->ModulePointer.get());
      else continue;
    } else {
      std::string Log = "Module '" + NextEvent->ModuleName + "' not found.";
      request("Logger", "log", false, std::string("Manager"), Log);
      NextEvent.reset();
    }
    getRequests();
    if (ManagerLoopInterrupt.load() && Once) {
      request("Renderer", "cleanUp", true);
      Once = false;
    }
    if (ManagerLoopInterrupt.load() && EventQueue.empty())
      break;
/*char ch;
std::cin >> ch;
if (ch == 'q')
  ManagerLoopInterrupt.store(true);*/
  }
}

void manager::getRequests() {
      for (auto& ModuleIterator : ModuleArray) {
        std::unique_ptr<module::eventbase> NextEvent = std::move(ModuleIterator.ModulePointer->getRequest());
        if (NextEvent != nullptr)
          EventQueue.push(std::move(NextEvent)); 
      }
}
