#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "modules/module.h"

class threadmanager {
public:
    threadmanager();
    bool TPisFull();
    bool MTisBusy();
    void dispatch(std::unique_ptr<module::eventbase>, module*);
    void dispatchToMain(std::unique_ptr<module::eventbase>, module*);
    void stop();
    ~threadmanager();
private:
  static void mainThreadLoop();

  std::vector<std::thread> ThreadPool;
  std::thread MainThread;
  std::unique_ptr<module::eventbase> TP_EventPointer;
  module* TP_ModulePointer;
  std::unique_ptr<module::eventbase> MT_EventPointer;
  module* MT_ModulePointer;
  std::mutex TP_Mutex;
  std::mutex MT_Mutex;
  std::condition_variable TP_Condition;
  std::condition_variable MT_Condition;
  std::atomic<bool> StopThreads;

  static threadmanager* ThisPointer;
};

#endif
