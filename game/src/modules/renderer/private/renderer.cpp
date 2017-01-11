#include "renderer.h"

#include <iostream>

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

int points = sizeof(indices)/(sizeof(GLfloat)*4);
renderer* renderer::ThisPointer;

void quitCallback(void* Pointer) {
  renderer::ThisPointer->quit();
}

void APIENTRY openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
  /*(void)source; (void)type; (void)id; 
  (void)severity; (void)length; (void)userParam;*/
  renderer::ThisPointer->request("Logger", "log", renderer::ThisPointer->getName(), std::string(message)); 
  if (severity == GL_DEBUG_SEVERITY_HIGH) {
    std::string Log = std::string(message) + "Aborting...\n";
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

  // Enable the debug callback
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(openglCallbackFunction, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);

  std::string Log = ProgramManager.getLog() + BufferManager.getLog() + UniformManager.getLog();
  request("Logger", "log", getName(), Log);

  GLint Size;
  glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &Size);
  std::cerr << Size;
}

void renderer::preparePrograms() {
  ProgramManager.createShader("intermediate/vertex.vert.glsl", GL_VERTEX_SHADER);
  ProgramManager.createShader("intermediate/voxelator.geo.glsl", GL_GEOMETRY_SHADER);
  ProgramManager.createShader("intermediate/fragment.frag.glsl", GL_FRAGMENT_SHADER);

  ProgramManager.createProgram("Program");
  ProgramManager.addShader("Program", "intermediate/vertex.vert.glsl", "intermediate/voxelator.geo.glsl", "intermediate/fragment.frag.glsl");
  ProgramManager.linkProgram("Program");
  ProgramManager.installProgram("Program");
}

GLuint FrameBufferObject, FrameBufferTexture, RenderBufferObject;

void renderer::prepareBuffers() {
  int ElementSize = 5 * sizeof(GLfloat);
  BufferManager.createBuffers(3, "ColourBuffer", "LightBuffer", "VertexBuffer");
  BufferManager.createVertexArrays(3, "PointCoordinates", "Colour", "BlockLevel");

  GLuint ColourBindingPoint = 1;
  ProgramManager.setUniformBinding("Program", INDEX, GL_UNIFORM_BLOCK, "ColourUniform", ColourBindingPoint);
  BufferManager.setBuffer("ColourBuffer", GL_UNIFORM_BUFFER, sizeof(ColourArray), ColourArray, GL_STATIC_DRAW);
  BufferManager.bindBuffer("ColourBuffer", ColourBindingPoint);

  struct lightstruct {
    float Position[4];
    float Intensity[3];
    float Attenuation;
    float AmbientCoefficent;
  };

  lightstruct LightArray[] = {
    {{2.0, 2.0, 2.0, 1.0}, {1.0, 1.0, 1.0}, 0.2, 0.005},
    {{-2.0, -2.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 0.2, 0.005},
    {{0.0, -13.0, 0.0, 0.0}, {0.0, 1.0, 1.0}, 0.2, 0.005},
    {{-2.0, 3.0, 1.0, 0.0}, {0.5, 0.5, 0.5}, 0.2, 0.005}
  };

  GLuint LightBindingPoint = 2;
  ProgramManager.setUniformBinding("Program", INDEX, GL_UNIFORM_BLOCK, "LightUniform", LightBindingPoint);
  BufferManager.setBuffer("LightBuffer", GL_UNIFORM_BUFFER, sizeof(LightArray), LightArray, GL_DYNAMIC_DRAW);
  BufferManager.bindBuffer("LightBuffer", LightBindingPoint);

  BufferManager.setBuffer("VertexBuffer", GL_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
  BufferManager.bindBuffer("VertexBuffer");

  BufferManager.setVertexArray("PointCoordinates", 0, 3, GL_FLOAT, GL_FALSE, ElementSize, nullptr);
  BufferManager.setVertexArray("Colour", 1, 1, GL_FLOAT, GL_TRUE, ElementSize, (const GLvoid*)(3 * sizeof(GLfloat)));
  BufferManager.setVertexArray("BlockLevel", 2, 1, GL_FLOAT, GL_TRUE, ElementSize, (const void*)(4 * sizeof(GLfloat)));

  int num_samples = 32;
  glGenTextures( 1, &FrameBufferTexture );
  glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, FrameBufferTexture );
  glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, num_samples, GL_RGBA8, WindowData.WindowWidth, WindowData.WindowHeight, true );
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	
  glGenRenderbuffers(1, &RenderBufferObject);
  glBindRenderbuffer(GL_RENDERBUFFER, RenderBufferObject);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, num_samples, GL_DEPTH24_STENCIL8, WindowData.WindowWidth, WindowData.WindowHeight);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glGenFramebuffers( 1, &FrameBufferObject );
  glBindFramebuffer( GL_FRAMEBUFFER, FrameBufferObject );
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, FrameBufferTexture, 0 );
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RenderBufferObject);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderer::prepareUniforms() {
  GLint ModelUniform = ProgramManager.getResource("Program", LOCATION, GL_UNIFORM, "ModelMatrix");
  GLint WorldUniform = ProgramManager.getResource("Program", LOCATION, GL_UNIFORM, "WorldMatrix");
  GLint CameraUniform = ProgramManager.getResource("Program", LOCATION, GL_UNIFORM, "CameraPosition");
  GLint NumLights = ProgramManager.getResource("Program", LOCATION, GL_UNIFORM, "NumLights");
  
  UniformManager.createUniform("ModelUniform", uniformmanager::UNIFORM_MATRIX, 4, ModelUniform);
  UniformManager.createUniform("WorldUniform", uniformmanager::UNIFORM_MATRIX, 4, WorldUniform);
  UniformManager.createUniform("CameraUniform", uniformmanager::UNIFORM, 3, CameraUniform);
  UniformManager.createUniform("NumLights", uniformmanager::UNIFORM, 1, NumLights);

  UniformManager.setUniform("NumLights", 2);
}

#include <cmath>
void renderer::draw() {
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  // glDisable(GL_CULL_FACE);
  glDepthFunc(GL_LESS);
  ProgramManager.installProgram("Program");
  BufferManager.installVertexArrays("PointCoordinates", "Colour", "BlockLevel");
  glClearColor(0.0, 0.5, 1.0, 1.0);
  glm::mat4 Perspective = glm::perspective(glm::radians(135.0f), 1.0f, 0.1f, 100.0f);
  glm::vec3 CameraPosition(2, 2, 2);
  // glm::mat4 Rotation = glm::rotate(glm::mat4(1), glm::mediump_float(DeltaTime), glm::vec3(1, 0, 0));
  // glm::vec4 Light = glm::vec4(2, 3, -1, 1);
  // glm::vec3 Camera = glm::vec3(Rotation * glm::vec4(CameraPosition, 1));
  glm::mat4 View = glm::lookAt(CameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
  glm::mat4 ModelMatrix = Perspective * View * Model;
  UniformManager.setUniform("ModelUniform", glm::value_ptr(ModelMatrix));
  UniformManager.setUniform("WorldUniform", glm::value_ptr(Model));
  UniformManager.setUniform("CameraUniform", glm::value_ptr(CameraPosition));
  // UniformManager.setUniform("LightPosition", glm::value_ptr(Light));
  glBindFramebuffer(GL_FRAMEBUFFER, FrameBufferObject);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glDrawArrays(GL_POINTS, 0, points);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, FrameBufferObject);
  glDrawBuffer(GL_BACK);
  glBlitFramebuffer(0, 0, WindowData.WindowWidth, WindowData.WindowHeight, 0, 0, WindowData.WindowWidth, WindowData.WindowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  
  swapBuffers();
}

void renderer::cleanUp() {
  glDeleteRenderbuffers(1, &RenderBufferObject);
  glDeleteTextures(1, &FrameBufferTexture);
  glDeleteFramebuffers(1, &FrameBufferObject);
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
