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
    ModuleIterator.ModulePointer->preShutDown();
  }
  AllowAddModule = true;
}

manager::~manager() {
  IsInstantiated = false;
}

void manager::managerLoop() {
  bool Once = true;
  bool FrameStart = true;
  bool FrameEnd = false;
  std::unique_ptr<module::eventbase> NextEvent;
  std::vector<modulestruct>::iterator EventIterator;
  while (/*!ManagerLoopInterrupt*/ true) {
    if (NextEvent == nullptr) {
      if (!ImmediateQueue.empty()) {
        NextEvent = std::move(ImmediateQueue.front());
        ImmediateQueue.pop();
        //if (NextEvent->FunctionName == "getDataImmediate")
          //FrameStart = true;
        EventIterator = std::find_if(ModuleArray.begin(), ModuleArray.end(), 
                [&] (const modulestruct& ModuleStructVar) {
                  return ModuleStructVar.ModuleName == NextEvent->ModuleName;});
      } else if (!EventQueue.empty()) {
          NextEvent = std::move(EventQueue.front());
          EventQueue.pop();
          if (NextEvent->ModuleName == "Blank") {
            NextEvent.reset();
            continue;
          }
          EventIterator = std::find_if(ModuleArray.begin(), ModuleArray.end(), 
                  [&] (const modulestruct& ModuleStructVar) {
                    return ModuleStructVar.ModuleName == NextEvent->ModuleName;});
      } else if (FrameStart && !ManagerLoopInterrupt.load()) {
            FrameStart = false;
            request("Renderer", "draw", true);
            request("Blank", "blank", false);
            request("Window", "refresh", false);
            FrameEnd = true;
            continue;
        }
    }
    if (EventIterator != ModuleArray.end() && NextEvent != nullptr) {
      if (NextEvent->UseMainThread && !ThreadManager.MTisBusy() && EventIterator->ModulePointer->isAvailable()) {
        ThreadManager.dispatchToMain(std::move(NextEvent), EventIterator->ModulePointer.get());
      }
      else if (!NextEvent->UseMainThread && !ThreadManager.TPisFull() && EventIterator->ModulePointer->isAvailable())
        ThreadManager.dispatch(std::move(NextEvent), EventIterator->ModulePointer.get());
      else continue;
    } else if (NextEvent != nullptr) {
      std::string Log = "Module '" + NextEvent->ModuleName + "' not found.";
      request("Logger", "log", false, std::string("Manager"), Log);
      NextEvent.reset();
    }
    getRequests();
    if (!ManagerLoopInterrupt.load()) {
      if (FrameEnd && EventQueue.empty()) {
        FrameEnd = false;
        request("Client", "update", false);
        FrameStart = true;
      }
    } else {
      if (Once) {
        request("Renderer", "cleanUp", true);
        Once = false;
      }
      if (EventQueue.empty() && ImmediateQueue.empty())
        break;
    }
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
    NextEvent = std::move(ModuleIterator.ModulePointer->getImmediateRequest());
    if (NextEvent != nullptr)
      ImmediateQueue.push(std::move(NextEvent)); 
  }
}
