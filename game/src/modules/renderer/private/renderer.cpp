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
  {{1.0, 0.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 0.2, 0.005},
  {{-1.0, 1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, 0.2, 0.005},
  {{0.0, -13.0, 0.0, 1.0}, {1.0, 1.0, 1.0}, 0.2, 0.005},
  {{-2.0, 3.0, 1.0, 0.0}, {1.0, 1.0, 1.0}, 0.2, 0.005}
};

uint64_t InputArray[][2] {{5771136619255974480, 17592186044417}, {129354334032, 8796093022209}, {65376, 3}, {21568, 1}, {19952, 4194305}, {56448, 8388609}};
//      -- --

struct block {
  int Coordinates[3];
  float Colour[3];
  unsigned int Level;
  int Rotation[3];
  int Offset[3];
};

unsigned int renderer::BlankVertexArray;
renderer* renderer::ThisPointer;

void quitCallback(void* Pointer) {
  renderer::ThisPointer->quit();
}

void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
  /*(void)source; (void)type; (void)id; 
  (void)severity; (void)length; (void)userParam;*/

  //renderer::ThisPointer->request("Logger", "log", renderer::ThisPointer->getName(), std::string(message)); 
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
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glFinish();

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
  ProgramManager.createShader("intermediate/compute.comp.glsl", GL_COMPUTE_SHADER);

  ProgramManager.createProgram("ComputeProgram");
  ProgramManager.addShader("ComputeProgram", "intermediate/compute.comp.glsl");
  ProgramManager.linkProgram("ComputeProgram");

  ProgramManager.createProgram("Program");
  ProgramManager.addShader("Program", "intermediate/vertex.vert.glsl", "intermediate/fragment.frag.glsl");
  ProgramManager.linkProgram("Program");
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
  BufferManager.setBuffer("StorageBuffer", GL_SHADER_STORAGE_BUFFER, sizeof(block) * 6, nullptr, GL_DYNAMIC_COPY);
  BufferManager.bindBuffer("StorageBuffer", StorageBindingPoint);

  GLuint ColourBindingPoint = 3;
  ProgramManager.setBinding("ComputeProgram", GL_UNIFORM_BLOCK, "ColourUniform", ColourBindingPoint);
  BufferManager.setBuffer("ColourBuffer", GL_UNIFORM_BUFFER, sizeof(ColourArray), ColourArray, GL_STATIC_DRAW);
  BufferManager.bindBuffer("ColourBuffer", ColourBindingPoint);

  GLuint LightBindingPoint = 4;
  ProgramManager.setBinding("Program", GL_UNIFORM_BLOCK, "LightUniform", LightBindingPoint);
  BufferManager.setBuffer("LightBuffer", GL_UNIFORM_BUFFER, sizeof(LightArray), LightArray, GL_STATIC_DRAW);
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
  GLint CSMatrixUniform = ProgramManager.getResourceLocation("Program", GL_UNIFORM, "CSMatrix");
  UniformManager.createUniform("CSMatrixUniform", uniformmanager::UNIFORM_MATRIX, 4, CSMatrixUniform);

  GLint WSMatrixUniform = ProgramManager.getResourceLocation("Program", GL_UNIFORM, "WSMatrix");
  UniformManager.createUniform("WSMatrixUniform", uniformmanager::UNIFORM_MATRIX, 4, WSMatrixUniform);
  
  GLint NumLights = ProgramManager.getResourceLocation("Program", GL_UNIFORM, "NumLights");
  UniformManager.createUniform("NumLights", uniformmanager::UNIFORM, 1, NumLights);

  GLint CameraPosition = ProgramManager.getResourceLocation("Program", GL_UNIFORM, "CameraPosition");
  UniformManager.createUniform("CameraPosition", uniformmanager::UNIFORM, 3, CameraPosition);

  ProgramManager.installProgram("Program");
  UniformManager.setUniform("NumLights", 2);
}

void renderer::draw() {
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);
  glDepthFunc(GL_LEQUAL); //glDepthFunc(GL_LESS);
  glClearColor(0.0, 0.5, 1.0, 1.0);

  glm::vec3 CameraPosition(1, 1, 1);
  glm::mat4 PerspectiveMatrix = glm::perspective(glm::radians(45.0f), (float)(WindowData.WindowWidth / WindowData.WindowHeight), 0.1f, 100.0f);
  glm::mat4 CameraMatrix = glm::lookAt(CameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  glm::mat4 WSMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.125)); // 0.03125
  glm::mat4 CSMatrix = PerspectiveMatrix * CameraMatrix;
  
  ProgramManager.installProgram("Program");
  UniformManager.setUniform("CSMatrixUniform", glm::value_ptr(CSMatrix));
  UniformManager.setUniform("WSMatrixUniform", glm::value_ptr(WSMatrix));
  UniformManager.setUniform("CameraPosition", glm::value_ptr(CameraPosition));
  
  BufferManager.bindFrameBuffer("FrameBuffer", GL_FRAMEBUFFER);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(BlankVertexArray);
  glDrawArrays(GL_TRIANGLES, 0, 144);

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
