#ifndef _GAME_ENGINE_SRC_MODULE_RENDERER_H_
#define _GAME_ENGINE_SRC_MODULE_RENDERER_H_

#include <memory>
#include <atomic>
#include <cstdio>

#include "modules/module.h"
#include "modules/window/window.h"

#include "external/include/glad.h"
#include "external/include/stb_image.h"

#include "content/colourArray.h"
#include "content/blockarray.h"
#include "classes/program_manager.h"
#include "classes/buffer_manager.h"
#include "classes/uniform_manager.h"

class renderer : public module {
 public:
  renderer();
  virtual void start();
  void prepare();
  void draw();
  void cleanUp();
  virtual void shutDown();
  virtual ~renderer();

 private:
  struct windowstruct {
    window* ObjectPointer;
    void(window::* SwapBuffers)();
    int WindowHeight;
    int WindowWidth;
    std::atomic<bool> DataIsReady;
  };

  struct camerastruct {
    float CSMatrix[16];
    float WSMatrix[16];
    float SkydomeMatrix[16];
    float Position[3];
    std::atomic<bool> DataIsReady;
  };

  void preparePrograms();
  void prepareBuffers();
  void prepareUniforms();
  void prepareSkydome();
  void prepareState();

  void swapBuffers();
  friend void quitCallback(void* Pointer);
  friend void openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
  void quit();

  windowstruct WindowData;
  camerastruct CurrentCameraData;
  camerastruct NewCameraData;
  programmanager ProgramManager;
  buffermanager BufferManager;
  uniformmanager UniformManager;

  static unsigned int BlankVertexArray;
  static renderer* ThisPointer;
};

#endif // _GAME_ENGINE_SRC_MODULE_RENDERER_H_
