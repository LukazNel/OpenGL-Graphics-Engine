#include "renderer.h"

//      --To be removed--
static GLfloat indices[] = {
          2.0f, 0.0f, 0.0f, 0.0f, 2.0f,
          0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
          1.0f, 0.0f, 0.0f, 9.0f, 0.0f,
          1.0f, 0.0f, 1.0f, 32.0f, 0.0f,
          -1.0f, 0.0f, 1.0f, 441.0f, 3.0f,
          -1.0f, 0.0f, -1.0f, 2.0f, 0.0f,
          -1.0f, 1.0f, -1.0f, 555.0f, 2.0f,
          -1.0f, 2.0f, -1.0f, 1002.0f, 0.0f
};

struct lightstruct {
  float Position[4];
  float Intensity[3];
  float Attenuation;
  float AmbientCoefficent;
};

lightstruct LightArray[] = {
  {{-1.0, 13.0, 2.0, 0.0}, {0.5, 1.0, 1.0}, 0.2, 0.005},
  {{-2.0, -2.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 0.2, 0.005},
  {{0.0, -13.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 0.2, 0.005},
  {{-2.0, 3.0, 1.0, 0.0}, {0.5, 0.5, 0.5}, 0.2, 0.005}
};

uint64_t InputArray[1][2] {{32960, 1}};
//      -- --

unsigned int renderer::BlankVertexArray;
renderer* renderer::ThisPointer;

void quitCallback(void* Pointer) {
  renderer::ThisPointer->quit();
}

void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
  /*(void)source; (void)type; (void)id; 
  (void)severity; (void)length; (void)userParam;*/
  renderer::ThisPointer->request("Logger", "log", renderer::ThisPointer->getName(), std::string(message)); 
  if (severity == GL_DEBUG_SEVERITY_HIGH) {
    std::string Log = "Aborting...\n";
    renderer::ThisPointer->request("Logger", "log", renderer::ThisPointer->getName(), Log);
    renderer::ThisPointer->request("Window", "quit");
  }
}

renderer::renderer() :
  ProgramManager(&quitCallback) {
  WindowData.DataIsReady.store(false);
  ThisPointer = this;
}

void renderer::start() {
  setName("Renderer", this);
  addFunction("prepare", &renderer::prepare);
  addFunction("draw", &renderer::draw);
  addFunction("cleanUp", &renderer::cleanUp);
  request("Window", "getWindowData", &WindowData.ObjectPointer, &WindowData.SwapBuffers, &WindowData.WindowWidth, &WindowData.WindowHeight, &WindowData.DataIsReady);
}

void renderer::prepare() {
  while (!WindowData.DataIsReady.load())
    continue;
  glViewport(0, 0, WindowData.WindowWidth, WindowData.WindowHeight);
  preparePrograms();
  prepareBuffers();
  prepareUniforms();
 
  ProgramManager.installProgram("ComputeProgram");
  glDispatchCompute(1, 1, 1); // Max 8 x 8 x 8
  glGenVertexArrays(1, &BlankVertexArray);
  glBindVertexArray(BlankVertexArray);

  // Enable the debug callback
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(openglCallbackFunction, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);

  std::string Log = ProgramManager.getLog() + BufferManager.getLog() + UniformManager.getLog();
  request("Logger", "log", getName(), Log);
}

void renderer::preparePrograms() {
  ProgramManager.createShader("intermediate/vertex.vert.glsl", GL_VERTEX_SHADER);
  ProgramManager.createShader("intermediate/fragment.frag.glsl", GL_FRAGMENT_SHADER);
  ProgramManager.createShader("intermediate/startup_block_decoder.comp.glsl", GL_COMPUTE_SHADER);

  ProgramManager.createProgram("ComputeProgram");
  ProgramManager.addShader("ComputeProgram", "intermediate/startup_block_decoder.comp.glsl");
  ProgramManager.linkProgram("ComputeProgram");

  ProgramManager.createProgram("Program");
  ProgramManager.addShader("Program", "intermediate/vertex.vert.glsl", "intermediate/fragment.frag.glsl");
  ProgramManager.linkProgram("Program");
  ProgramManager.installProgram("Program");
}

void renderer::prepareBuffers() {
  BufferManager.createBuffers(4, "InputBuffer", "StorageBuffer", "ColourBuffer", "LightBuffer");

  GLuint InputBindingPoint = 1;
  ProgramManager.setBinding("ComputeProgram", GL_UNIFORM_BLOCK, "InputBuffer", InputBindingPoint);
  BufferManager.setBuffer("InputBuffer", GL_UNIFORM_BUFFER, sizeof(InputArray), InputArray, GL_STATIC_DRAW); // Actual Size: 4194304
  BufferManager.bindBuffer("InputBuffer", InputBindingPoint);

  GLuint StorageBindingPoint = 2;
  ProgramManager.setBinding("ComputeProgram", GL_SHADER_STORAGE_BLOCK, "StorageBuffer", StorageBindingPoint);
  ProgramManager.setBinding("Program", GL_SHADER_STORAGE_BLOCK, "StorageBuffer", StorageBindingPoint);
  BufferManager.setBuffer("StorageBuffer", GL_SHADER_STORAGE_BLOCK, 60000000, nullptr, GL_STATIC_DRAW);
  BufferManager.bindBuffer("StorageBuffer", StorageBindingPoint);

  GLuint ColourBindingPoint = 3;
  ProgramManager.setBinding("Program", GL_UNIFORM_BLOCK, "ColourUniform", ColourBindingPoint);
  BufferManager.setBuffer("ColourBuffer", GL_UNIFORM_BUFFER, sizeof(ColourArray), ColourArray, GL_STATIC_DRAW);
  BufferManager.bindBuffer("ColourBuffer", ColourBindingPoint);

  GLuint LightBindingPoint = 4;
  ProgramManager.setBinding("Program", GL_UNIFORM_BLOCK, "LightUniform", LightBindingPoint);
  BufferManager.setBuffer("LightBuffer", GL_UNIFORM_BUFFER, sizeof(LightArray), LightArray, GL_DYNAMIC_DRAW);
  BufferManager.bindBuffer("LightBuffer", LightBindingPoint);

  GLuint FrameBufferTexture;
  int num_samples = 32;
  glGenTextures( 1, &FrameBufferTexture );
  glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, FrameBufferTexture );
  glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, num_samples, GL_RGBA8, WindowData.WindowWidth, WindowData.WindowHeight, true );
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

  BufferManager.createRenderBuffers(1, "RenderBuffer");
  BufferManager.setRenderBuffer("RenderBuffer", num_samples, GL_DEPTH24_STENCIL8, WindowData.WindowWidth, WindowData.WindowHeight);

  BufferManager.createFrameBuffers(1, "FrameBuffer");
  BufferManager.setFrameBuffer("FrameBuffer", GL_COLOR_ATTACHMENT0, FrameBufferTexture, 0);
  BufferManager.setFrameBuffer("FrameBuffer", "RenderBuffer");
}

void renderer::prepareUniforms() {
  GLint ModelUniform = ProgramManager.getResourceLocation("Program", GL_UNIFORM, "ModelMatrix");
  GLint WorldUniform = ProgramManager.getResourceLocation("Program", GL_UNIFORM, "WorldMatrix");
  GLint CameraUniform = ProgramManager.getResourceLocation("Program", GL_UNIFORM, "CameraPosition");
  GLint NumLights = ProgramManager.getResourceLocation("Program", GL_UNIFORM, "NumLights");
  
  UniformManager.createUniform("ModelUniform", uniformmanager::UNIFORM_MATRIX, 4, ModelUniform);
  UniformManager.createUniform("WorldUniform", uniformmanager::UNIFORM_MATRIX, 4, WorldUniform);
  UniformManager.createUniform("CameraUniform", uniformmanager::UNIFORM, 3, CameraUniform);
  UniformManager.createUniform("NumLights", uniformmanager::UNIFORM, 1, NumLights);

  UniformManager.setUniform("NumLights", 1);
}

#include <cmath>
void renderer::draw() {
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);

  // glDisable(GL_CULL_FACE);
  glDepthFunc(GL_LESS);
  ProgramManager.installProgram("Program");
  glClearColor(0.0, 0.5, 1.0, 1.0);
  glm::mat4 Perspective = glm::perspective(glm::radians(135.0f), 1.0f, 0.1f, 100.0f);
  glm::vec3 CameraPosition(-1, 2, 1);
  // glm::mat4 Rotation = glm::rotate(glm::mat4(1), glm::mediump_float(DeltaTime), glm::vec3(1, 0, 0));
  // glm::vec4 Light = glm::vec4(2, 3, -1, 1);
  // glm::vec3 Camera = glm::vec3(Rotation * glm::vec4(CameraPosition, 1));
  glm::mat4 View = glm::lookAt(CameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.03125f));
  glm::mat4 ModelMatrix = Perspective * View * Model;
  UniformManager.setUniform("ModelUniform", glm::value_ptr(ModelMatrix));
  UniformManager.setUniform("WorldUniform", glm::value_ptr(Model));
  UniformManager.setUniform("CameraUniform", glm::value_ptr(CameraPosition));
  // UniformManager.setUniform("LightPosition", glm::value_ptr(Light));
  
  BufferManager.bindFrameBuffer("FrameBuffer", GL_FRAMEBUFFER);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(BlankVertexArray);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  BufferManager.bindFrameBuffer("FrameBuffer", GL_READ_FRAMEBUFFER);
  glDrawBuffer(GL_BACK);
  glBlitFramebuffer(0, 0, WindowData.WindowWidth, WindowData.WindowHeight, 0, 0, WindowData.WindowWidth, WindowData.WindowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  swapBuffers();
}

void renderer::cleanUp() {
  BufferManager.cleanUp();
  ProgramManager.cleanUp();
}

void renderer::shutDown() {
}

renderer::~renderer() {
}

void renderer::swapBuffers() {
  if (WindowData.ObjectPointer != nullptr && WindowData.SwapBuffers != nullptr) {
    (WindowData.ObjectPointer->*WindowData.SwapBuffers)();
  } else {
    std::string Log = "Cannot swap buffers: Pointers to Window not initialized";
    request("Logger", "log", getName(), Log);
  }
}

void renderer::quit() {
  std::string Log = ProgramManager.getLog() + BufferManager.getLog() + UniformManager.getLog();
  request("Logger", "log", getName(), Log);
  request("Window", "quit");
}
