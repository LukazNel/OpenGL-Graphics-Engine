#include "window.h"

window::window() :
  ExitGame(nullptr) {
  WindowData = {"A Block's League", 640, 480, nullptr, nullptr};
}

void window::start() {
  setName("Window", this);
  addFunction("setInterrupt", &window::setInterrupt);
  addFunction("quit", &window::quit);
  addFunction("getWindowData", &window::getWindowData);
  addFunction("getDataImmediate", &window::getDataImmediate);
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
  *WindowWidth = WindowData.WindowWidth;
  *WindowHeight = WindowData.WindowHeight;
  DataIsReady->store(true);
}

void window::getDataImmediate(std::promise<bool>* DataIsReady, int* WindowWidth, int* WindowHeight, float* DeltaTime) {
  *WindowWidth = WindowData.WindowWidth;
  *WindowHeight = WindowData.WindowHeight;
  *DeltaTime = TimeData.DeltaTime;
  DataIsReady->set_value(true);
}

void window::swapBuffers() {
  SDL_GL_SwapWindow(WindowData.MainWindow);
}

void window::begin() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
    std::string Log = "SDL initialization failed.";
    request("Logger", "log", getName(), Log);
    quit();
  }
  SDL_GL_LoadLibrary(nullptr);
  setOpenGLAttributes();
  WindowData.MainWindow = SDL_CreateWindow(WindowData.ProgramName.c_str(), 0, 0, WindowData.WindowWidth, WindowData.WindowHeight, SDL_WINDOW_OPENGL);
  if (!WindowData.MainWindow) {
    std::string Log = "Window creation failed.";
    request("Logger", "log", getName(), Log);
    quit();
  }
  WindowData.MainContext = SDL_GL_CreateContext(WindowData.MainWindow);
  SDL_GL_MakeCurrent(WindowData.MainWindow, WindowData.MainContext);
  gladLoadGLLoader(GLADloadproc (SDL_GL_GetProcAddress));
  SDL_GL_SetSwapInterval(1);
}

void window::refresh() {
  frameCount();
  while(SDL_PollEvent(&Event)) {
    request("Input", "updateEvents", Event);
  }
}

void window::shutDown() {
  //SDL_DeleteContext(MainContext);
  SDL_DestroyWindow(WindowData.MainWindow);
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
  double CurrentTime = SDL_GetTicks();
  TimeData.DeltaTime = CurrentTime - TimeData.LastTime;
  if (TimeData.DeltaTime > 500) {
    float Fps = TimeData.DeltaTime / TimeData.NFrames;
    std::string FpsTitle = WindowData.ProgramName + " @ " + std::to_string(Fps) + " Seconds per Frame.";
    SDL_SetWindowTitle(WindowData.MainWindow, FpsTitle.c_str());
    TimeData.LastTime = CurrentTime;
    TimeData.NFrames = 0;
  }
  TimeData.NFrames++;
}
