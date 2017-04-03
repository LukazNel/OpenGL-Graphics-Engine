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
  {{1.0, 1.0, 1.0, 1.0}, {0.75, 1.0, 1.0}, 0.8, 0.05},
  {{-1.0, 1.0, 1.0, 1.0}, {0.0, 1.0, 1.0}, 0.2, 0.005},
  {{0.0, -13.0, 0.0, 1.0}, {1.0, 1.0, 1.0}, 0.2, 0.005},
  {{-2.0, 3.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 0.2, 0.005}
};

//uint64_t BlockArray[][2] {{5771136619255974480, 17592186044417}, {129354334032, 8796093022209}, {65376, 3}, {21568, 1}, {19952, 4194305}, {56448, 8388609}};
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
    std::string Log = message;
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
  request("Client", "setCameraPointers", (float*)NewCameraData.CSMatrix, (float*)NewCameraData.WSMatrix, (float*)NewCameraData.SkyboxMatrix, (float*)NewCameraData.Position, &NewCameraData.DataIsReady);
}

void renderer::prepare() {
  // NOTE Acessing Windowdata may be unsafe if data is changed after atomic is true.
  while (!WindowData.DataIsReady.load())
    continue;
  glViewport(0, 0, WindowData.WindowWidth, WindowData.WindowHeight);
  preparePrograms();
  prepareBuffers();
  prepareUniforms();
  prepareSkybox();
  prepareState();
 
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
  ProgramManager.createProgram("Program");
  ProgramManager.addShader("Program", "intermediate/vertex.vert.glsl", "intermediate/fragment.frag.glsl");
  ProgramManager.linkProgram("Program");

  ProgramManager.createShader("intermediate/compute.comp.glsl", GL_COMPUTE_SHADER);
  ProgramManager.createProgram("ComputeProgram");
  ProgramManager.addShader("ComputeProgram", "intermediate/compute.comp.glsl");
  ProgramManager.linkProgram("ComputeProgram");

  ProgramManager.createShader("intermediate/skybox.vert.glsl", GL_VERTEX_SHADER);
  ProgramManager.createShader("intermediate/skybox.frag.glsl", GL_FRAGMENT_SHADER);
  ProgramManager.createProgram("Skybox");
  ProgramManager.addShader("Skybox", "intermediate/skybox.vert.glsl", "intermediate/skybox.frag.glsl");
  ProgramManager.linkProgram("Skybox");
}

void renderer::prepareBuffers() {
  BufferManager.createBuffers(4, "InputBuffer", "StorageBuffer", "ColourBuffer", "LightBuffer");

  GLuint InputBindingPoint = 1;
  ProgramManager.setBinding("ComputeProgram", GL_UNIFORM_BLOCK, "InputBuffer", InputBindingPoint);
  BufferManager.setBuffer("InputBuffer", GL_UNIFORM_BUFFER, sizeof(BlockArray), BlockArray, GL_STATIC_DRAW); // Actual Size: 4194304
  BufferManager.bindBuffer("InputBuffer", InputBindingPoint);

  GLuint StorageBindingPoint = 2;
  ProgramManager.setBinding("ComputeProgram", GL_SHADER_STORAGE_BLOCK, "StorageBuffer", StorageBindingPoint);
  ProgramManager.setBinding("Program", GL_SHADER_STORAGE_BLOCK, "StorageBuffer", StorageBindingPoint);
  BufferManager.setBuffer("StorageBuffer", GL_SHADER_STORAGE_BUFFER, sizeof(block) * 10000, nullptr, GL_DYNAMIC_COPY);
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
  //glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

  BufferManager.createRenderBuffers(1, "RenderBuffer");
  BufferManager.setRenderBuffer("RenderBuffer", num_samples, GL_DEPTH24_STENCIL8, WindowData.WindowWidth, WindowData.WindowHeight);

  BufferManager.createFrameBuffers(1, "FrameBuffer");
  BufferManager.setFrameBuffer("FrameBuffer", GL_COLOR_ATTACHMENT0, FrameBufferTexture, 0);
  BufferManager.setFrameBuffer("FrameBuffer", "RenderBuffer");
}

void renderer::prepareUniforms() {
  GLint CSMatrixUniform = ProgramManager.getResourceLocation("Program", GL_UNIFORM, "CSMatrix");
  UniformManager.createUniformMatrix("CSMatrixMain", 4, CSMatrixUniform);

  GLint WSMatrixUniform = ProgramManager.getResourceLocation("Program", GL_UNIFORM, "WSMatrix");
  UniformManager.createUniformMatrix("WSMatrixMain", 4, WSMatrixUniform);
  
  GLint NumLights = ProgramManager.getResourceLocation("Program", GL_UNIFORM, "NumLights");
  UniformManager.createUniform("NumLights", 1, NumLights);

  GLint CameraPosition = ProgramManager.getResourceLocation("Program", GL_UNIFORM, "CameraPosition");
  UniformManager.createUniform("CameraPosition", 3, CameraPosition);

  ProgramManager.installProgram("Program");
  UniformManager.setUniform("NumLights", 2);
  
  CSMatrixUniform = ProgramManager.getResourceLocation("Skybox", GL_UNIFORM, "SkyboxMatrix");
  UniformManager.createUniformMatrix("CSMatrixSkybox", 4, CSMatrixUniform);

}

void renderer::prepareSkybox() {
  GLuint SkyboxTexture;
  glGenTextures(1, &SkyboxTexture);
  glActiveTexture(GL_TEXTURE0);

  int Width, Height, Channels;
  unsigned char* Image;
  
  glBindTexture(GL_TEXTURE_CUBE_MAP, SkyboxTexture);
  std::string Suffix[6] = {"_RT", "_LF", "_UP", "_DN", "_BK", "_FR"};
  for (int i = 0; i < 6; i++) {
    Image = stbi_load(std::string("content/skybox/sky8" + Suffix[i] + ".jpg").c_str(), &Width, &Height, &Channels, 3);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Image);
    stbi_image_free(Image);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  //glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void renderer::prepareState() {
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);
  //glDepthFunc(GL_LEQUAL); //glDepthFunc(GL_LESS);
  glClearColor(0.0, 0.5, 1.0, 1.0);
}

void renderer::draw() {
  if (NewCameraData.DataIsReady.load()) {
    std::copy(NewCameraData.CSMatrix, NewCameraData.CSMatrix + 16, CurrentCameraData.CSMatrix);
    std::copy(NewCameraData.WSMatrix, NewCameraData.WSMatrix + 16, CurrentCameraData.WSMatrix);
    std::copy(NewCameraData.SkyboxMatrix, NewCameraData.SkyboxMatrix + 16, CurrentCameraData.SkyboxMatrix);
    std::copy(NewCameraData.Position, NewCameraData.Position + 3, CurrentCameraData.Position);
    NewCameraData.DataIsReady.store(false); // Handshaking: State says true and Renderer changes data, Renderer says false and State changed data.
  }

  BufferManager.bindFrameBuffer("FrameBuffer", GL_FRAMEBUFFER);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glDepthFunc(GL_LESS);
  ProgramManager.installProgram("Program");
  UniformManager.setUniform("CSMatrixMain", (GLfloat*)(CurrentCameraData.CSMatrix));
  UniformManager.setUniform("WSMatrixMain", (GLfloat*)(CurrentCameraData.WSMatrix));
  UniformManager.setUniform("CameraPosition", (GLfloat*)(CurrentCameraData.Position));
  glDrawArrays(GL_TRIANGLES, 0, 12288); //24

  glDepthFunc(GL_LEQUAL);
  ProgramManager.installProgram("Skybox");
  UniformManager.setUniform("CSMatrixSkybox", (GLfloat*)(CurrentCameraData.SkyboxMatrix));
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
