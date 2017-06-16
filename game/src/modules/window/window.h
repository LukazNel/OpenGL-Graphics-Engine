#ifndef _GAME_ENGINE_SRC_MODULE_WINDOW_H_
#define _GAME_ENGINE_SRC_MODULE_WINDOW_H_

#include <string>
#include <memory>
#include <atomic>

#include "modules/module.h"
#include "external/include/glad.h"
#include "external/include/SDL2/SDL.h"

class window : public module {
 public:
  window();
  virtual void start();
  void setInterrupt(std::atomic<bool>*);
  void quit();
  void getWindowData(window**, void(window::** SwapPointer)(), int* WindowWidth, int* WindowHeight, std::atomic<bool>*);
  void getDataImmediate(std::promise<bool>* DataIsReady, int* WindowWidth, int* WindowHeight, float* DeltaTime);
  void swapBuffers();
  void begin();
  void refresh();
  virtual void shutDown();

 private:
  struct windowstruct {
    std::string ProgramName;
    int WindowWidth;
    int WindowHeight;
    SDL_Window* MainWindow;
    SDL_GLContext MainContext;
  };
  struct timestruct {
    int NFrames;
    float LastTime;
    float TimeDifference;
    float DeltaTime;
  };

  void makeContextCurrent();
  void setOpenGLAttributes();
  void frameCount();
 
  std::atomic<bool>* ExitGame;
  windowstruct WindowData;
  timestruct TimeData;
  SDL_Event Event;
};

#endif // _GAME_ENGINE_SRC_MODULE_WINDOW_H_
