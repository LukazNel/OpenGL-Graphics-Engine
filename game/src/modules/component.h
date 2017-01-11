#ifndef _GAME_ENGINE_SRC_MODULE_COMPONENT_H_
#define _GAME_ENGINE_SRC_MODULE_COMPONENT_H_

#include <string>
#include <memory>

#include "external/include/readerwriterqueue.h"

class component {
 public:
  component();
  void initialize(moodycamel::ReaderWriterQueue<std::unique_ptr<module::eventbase>>* Queue) {
    EventQueue = Queue;
  }
 protected:
  template<typename ...args>
    void request(const std::string ModuleName, const std::string FunctionName, args... Arguments) {
      std::unique_ptr<module::eventbase> EventVar(new module::event<args...>(ModuleName, FunctionName, false, Arguments...));
      //EventQueue.push(std::move(EventVar));
      EventQueue->enqueue(std::move(EventVar));
    }
 private:

  moodycamel::ReaderWriterQueue<std::unique_ptr<eventbase>>* EventQueue;
};

#endif
