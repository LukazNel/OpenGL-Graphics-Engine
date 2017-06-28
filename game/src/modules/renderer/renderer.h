#ifndef _GAME_ENGINE_SRC_MODULE_RENDERER_H_
#define _GAME_ENGINE_SRC_MODULE_RENDERER_H_

#include <memory>
#include <atomic>
#include <cstdio>
#include <algorithm>
#include <fstream>

#include "modules/module.h"
#include "modules/window/window.h"

#include "external/include/glad.h"

#include "content/colourArray.h"
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
    float SSMatrix[16];
    float CSMatrix[16];
    float WSMatrix[16];
    float Position[3];
    
    float SkydomeMatrix[16];
    float StarMatrix[16];
    float SunPosition[3];
    float Weather;
    float Time;
    std::atomic<bool> DataIsReady;
  };

  struct encoderstruct {
    int DispatchSize[3];
    int DispatchVolume;
    int BufferSizeBlocks;
    int BufferSizeBytes;
    int Offset;
    GLsync SyncObject;
    std::ifstream FileIn;
  };

  void preparePrograms();
  void prepareTextures();
  void prepareBuffers();
  void prepareUniforms();
  void prepareState();
  void dispatchEncoders();

  void swapBuffers();
  friend void quitCallback(void* Pointer);
  friend void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
  void quit();

  windowstruct WindowData;
  camerastruct CurrentCameraData;
  camerastruct NewCameraData;
  encoderstruct EncoderData;
  programmanager ProgramManager;
  buffermanager BufferManager;
  uniformmanager UniformManager;

  static unsigned int BlankVertexArray;
  static renderer* ThisPointer;
};

#endif // _GAME_ENGINE_SRC_MODULE_RENDERER_H_
