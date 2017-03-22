#include "window.h"

window::window() :
  ExitGame(nullptr),
  MainWindow(nullptr),
  MainContext(nullptr) {
  Configuration = {"A Block's League", 640, 480};
}

void window::start() {
  setName("Window", this);
  addFunction("setInterrupt", &window::setInterrupt);
  addFunction("quit", &window::quit);
  addFunction("getWindowData", &window::getWindowData);
  addFunction("begin", &window::begin);
  addFunction("refresh", &window::refresh);
}

void window::setInterrupt(std::atomic<bool>* ManagerLoopInterrupt) {
  ExitGame = ManagerLoopInterrupt;
}

void window::quit() {
  ExitGame->store(true);
}

void window::getWindowData(window** WindowPointer, void(window::** SwapPointer)(), int* WindowWidth, int* WindowHeight, std::atomic<bool>* DataIsReady) {
  *WindowPointer = this;
  *SwapPointer = &window::swapBuffers;
  *WindowWidth = Configuration.WindowWidth;
  *WindowHeight = Configuration.WindowHeight;
  DataIsReady->store(true);
}

void window::swapBuffers() {
  SDL_GL_SwapWindow(MainWindow);
}

void window::begin() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
    std::string Log = "SDL initialization failed.";
    request("Logger", "log", getName(), Log);
    quit();
  }
  SDL_GL_LoadLibrary(nullptr);
  setOpenGLAttributes();
  MainWindow = SDL_CreateWindow(Configuration.ProgramName.c_str(), 0, 0, 640, 480, SDL_WINDOW_OPENGL);
  if (!MainWindow) {
    std::string Log = "Window creation failed.";
    request("Logger", "log", getName(), Log);
    quit();
  }
  MainContext = SDL_GL_CreateContext(MainWindow);
  SDL_GL_MakeCurrent(MainWindow, MainContext);
  gladLoadGLLoader(GLADloadproc (SDL_GL_GetProcAddress));
  SDL_GL_SetSwapInterval(1);
}

void window::refresh() {
  frameCount();
  while(SDL_PollEvent(&Event)) {
    request("Input", "updateEvents", Event);
/*if (Event.type == SDL_QUIT)
  quit();
if (Event.type == SDL_KEYDOWN)
  quit();*/
  }
}

void window::shutDown() {
  //SDL_DeleteContext(MainContext);
  SDL_DestroyWindow(MainWindow);
  SDL_Quit();
}

void window::setOpenGLAttributes() {
  SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}

void window::frameCount() {
  static int NFrames;
  static double LastTime;
  double CurrentTime = SDL_GetTicks();
  double ElapsedTime = CurrentTime - LastTime;
  if (ElapsedTime > 500) {
    double Fps = NFrames / ElapsedTime * 1000;
    std::string FpsTitle = Configuration.ProgramName + " @ " + std::to_string(Fps) + " Frames per Second";
    SDL_SetWindowTitle(MainWindow, FpsTitle.c_str());
    LastTime = CurrentTime;
    NFrames = 0;
  }
  NFrames++;
}
