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
  void swapBuffers();
  void begin();
  void refresh();
  virtual void shutDown();

 private:
  struct windowconfig {
    std::string ProgramName;
    int WindowWidth;
    int WindowHeight;
  };

  void makeContextCurrent();
  void setOpenGLAttributes();
  void frameCount();
 
  std::atomic<bool>* ExitGame;
  windowconfig Configuration;
  SDL_Window* MainWindow;
  SDL_GLContext MainContext;
  SDL_Event Event;
};

#endif // _GAME_ENGINE_SRC_MODULE_WINDOW_H_
