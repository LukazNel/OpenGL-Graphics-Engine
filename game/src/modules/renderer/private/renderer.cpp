#include "renderer.h"

//      --To be removed--
struct lightstruct {
  float Position[4];
  float Intensity[3];
  float Attenuation;
  float AmbientCoefficient;
};

lightstruct LightArray[] = {
  {{50.0, 100.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 0.2, 0.005},
  {{-40.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 0.2, 0.005},
  {{5.0, 13.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, 0.2, 0.005},
  {{-2.0, 3.0, 20.0, 1.0}, {1.0, 1.0, 1.0}, 0.2, 0.005}
};

//      -- --

struct indirectstruct {
   GLuint Count;
   GLuint InstanceCount;
   GLuint First;
   GLuint BaseInstance;
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
  EncoderData.DispatchSize[0] = 2;
  EncoderData.DispatchSize[1] = 1;
  EncoderData.DispatchSize[2] = 1;
  EncoderData.DispatchVolume = EncoderData.DispatchSize[0] * EncoderData.DispatchSize[1] * EncoderData.DispatchSize[2];
  EncoderData.BufferSizeBlocks = 512 * EncoderData.DispatchVolume; // Local dispatch size = 8x8x8 = 512
  EncoderData.BufferSizeBytes = EncoderData.BufferSizeBlocks * 128 / 8; // 128 bits / 8 bits(one byte)
  EncoderData.Offset = EncoderData.BufferSizeBlocks;
}

void renderer::start() {
  setName("Renderer", this);
  addFunction("prepare", &renderer::prepare);
  addFunction("draw", &renderer::draw);
  addFunction("cleanUp", &renderer::cleanUp);
  request("Window", "getWindowData", &WindowData.ObjectPointer, &WindowData.SwapBuffers, &WindowData.WindowWidth, &WindowData.WindowHeight, &WindowData.DataIsReady);
  request("Client", "setCameraPointers", (float*)NewCameraData.SSMatrix, (float*)NewCameraData.CSMatrix, (float*)NewCameraData.WSMatrix, (float*)NewCameraData.Position, (float*)NewCameraData.SkydomeMatrix, (float*)NewCameraData.StarMatrix, (float*)NewCameraData.SunPosition, (float*)&NewCameraData.Weather, (float*)&NewCameraData.Time, &NewCameraData.DataIsReady);
}

void renderer::prepare() {
  EncoderData.FileIn.open("content/blockarray", std::ios::in | std::ios::binary);
  if (!EncoderData.FileIn)
    request("Logger", "log", getName(), std::string("Warning: Input file not found!"));
  // NOTE Acessing Windowdata may be unsafe if data is changed after atomic is true.
  while (!WindowData.DataIsReady.load())
    continue;
  preparePrograms();
  prepareTextures();
  prepareBuffers();
  prepareUniforms();
  prepareState();
 
  ProgramManager.installProgram("Compute");
  glDispatchCompute(EncoderData.DispatchSize[0], EncoderData.DispatchSize[1], EncoderData.DispatchSize[2]); // Max 8 x 8 x 8
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
  ProgramManager.createShader("intermediate/shadows.vert", GL_VERTEX_SHADER);
  ProgramManager.createShader("intermediate/shadows.frag", GL_FRAGMENT_SHADER);
  ProgramManager.createProgram("Shadows");
  ProgramManager.addShader("Shadows", "intermediate/shadows.vert", "intermediate/shadows.frag");
  ProgramManager.linkProgram("Shadows");

  ProgramManager.createShader("intermediate/main.vert", GL_VERTEX_SHADER);
  ProgramManager.createShader("intermediate/main.frag", GL_FRAGMENT_SHADER);
  ProgramManager.createProgram("Main");
  ProgramManager.addShader("Main", "intermediate/main.vert", "intermediate/main.frag");
  ProgramManager.linkProgram("Main");

  ProgramManager.createShader("intermediate/compute.comp", GL_COMPUTE_SHADER);
  ProgramManager.createProgram("Compute");
  ProgramManager.addShader("Compute", "intermediate/compute.comp");
  ProgramManager.linkProgram("Compute");

  ProgramManager.createShader("intermediate/skydome.vert", GL_VERTEX_SHADER);
  ProgramManager.createShader("intermediate/skydome.tesc", GL_TESS_CONTROL_SHADER);
  ProgramManager.createShader("intermediate/skydome.tese", GL_TESS_EVALUATION_SHADER);
  ProgramManager.createShader("intermediate/skydome.frag", GL_FRAGMENT_SHADER);
  ProgramManager.createProgram("Skydome");
  ProgramManager.addShader("Skydome", "intermediate/skydome.vert", "intermediate/skydome.tesc", "intermediate/skydome.tese", "intermediate/skydome.frag");
  ProgramManager.linkProgram("Skydome");
}

void renderer::prepareTextures() {
  GLint Location;
  UniformManager.createTextures(GL_TEXTURE_2D, 7, "Shadows", "Tint1", "Tint2", "Sun", "Moon", "Clouds1", "Clouds2");
 
  ProgramManager.installProgram("Main");
  Location = ProgramManager.getResourceLocation("Main", GL_UNIFORM, "ShadowDepth");
  UniformManager.setBlankTexture("Shadows", GL_DEPTH_COMPONENT16, 1024, 1024, Location);
  UniformManager.setDefaultParameters("Shadows", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
  float BorderColour[] {1.0f, 1.0f, 1.0f, 1.0f};
  UniformManager.setTextureParameter("Shadows", GL_TEXTURE_BORDER_COLOR, BorderColour);

  ProgramManager.installProgram("Skydome");

  Location = ProgramManager.getResourceLocation("Skydome", GL_UNIFORM, "tint");
  UniformManager.setTexture("Tint1", "content/skydome/tint.tga", Location);
  UniformManager.setDefaultParameters("Tint1", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

  Location = ProgramManager.getResourceLocation("Skydome", GL_UNIFORM, "tint1");
  UniformManager.setTexture("Tint2", "content/skydome/tint2.tga", Location);
  UniformManager.setDefaultParameters("Tint2", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

  Location = ProgramManager.getResourceLocation("Skydome", GL_UNIFORM, "sun");
  UniformManager.setTexture("Sun", "content/skydome/sun.tga", Location);
  UniformManager.setDefaultParameters("Sun", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

  Location = ProgramManager.getResourceLocation("Skydome", GL_UNIFORM, "moon");
  UniformManager.setTexture("Moon", "content/skydome/moon.tga", Location);
  UniformManager.setDefaultParameters("Moon", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

  Location = ProgramManager.getResourceLocation("Skydome", GL_UNIFORM, "clouds1");
  UniformManager.setTexture("Clouds1", "content/skydome/clouds1.tga", Location);
  UniformManager.setDefaultParameters("Clouds1", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

  /*Location = ProgramManager.getResourceLocation("Skydome", GL_UNIFORM, "clouds2");
  UniformManager.setTexture("Clouds2", "content/skydome/clouds1.tga", Location);
  UniformManager.setDefaultParameters("Clouds2", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);*/

  Location = ProgramManager.getResourceLocation("Skydome", GL_UNIFORM, "tint");
  UniformManager.setTexture("Tint1", "content/skydome/tint.tga", Location);
  UniformManager.setDefaultParameters("Tint1", GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

void renderer::prepareBuffers() {
  BufferManager.createBuffers(5, "InputBuffer", "StorageBuffer", "ColourBuffer", "LightBuffer", "DrawBuffer");

  GLuint InputBindingPoint = 1;
  std::vector<uint64_t> Vector(EncoderData.BufferSizeBlocks * 2, 0); // 1 block = 2X 64-bit
  if (EncoderData.FileIn)
    EncoderData.FileIn.read((char*)Vector.data(), EncoderData.BufferSizeBytes);
  ProgramManager.setBinding("Compute", GL_SHADER_STORAGE_BLOCK, "InputBuffer", InputBindingPoint);
  BufferManager.setBuffer("InputBuffer", GL_SHADER_STORAGE_BUFFER, EncoderData.BufferSizeBytes, Vector.data(), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT); // Actual Size: 4194304
  BufferManager.mapBuffer("InputBuffer", 0, EncoderData.BufferSizeBytes, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  BufferManager.bindBuffer("InputBuffer", InputBindingPoint);

  GLuint StorageBindingPoint = 2;
  ProgramManager.setBinding("Compute", GL_SHADER_STORAGE_BLOCK, "StorageBuffer", StorageBindingPoint);
  ProgramManager.setBinding("Shadows", GL_SHADER_STORAGE_BLOCK, "StorageBuffer", StorageBindingPoint);
  ProgramManager.setBinding("Main", GL_SHADER_STORAGE_BLOCK, "StorageBuffer", StorageBindingPoint);
  BufferManager.setBuffer("StorageBuffer", GL_SHADER_STORAGE_BUFFER, 117440512, nullptr, 0);
  BufferManager.bindBuffer("StorageBuffer", StorageBindingPoint);

  GLuint ColourBindingPoint = 3;
  ProgramManager.setBinding("Compute", GL_UNIFORM_BLOCK, "ColourUniform", ColourBindingPoint);
  BufferManager.setBuffer("ColourBuffer", GL_UNIFORM_BUFFER, sizeof(ColourArray), ColourArray, 0);
  BufferManager.bindBuffer("ColourBuffer", ColourBindingPoint);

  GLuint LightBindingPoint = 4;
  ProgramManager.setBinding("Main", GL_UNIFORM_BLOCK, "LightUniform", LightBindingPoint);
  BufferManager.setBuffer("LightBuffer", GL_UNIFORM_BUFFER, sizeof(LightArray), LightArray, GL_DYNAMIC_STORAGE_BIT);
  BufferManager.bindBuffer("LightBuffer", LightBindingPoint);

  GLuint DrawBindingPoint = 5;
  indirectstruct Indirect;
  Indirect.Count = EncoderData.BufferSizeBlocks * 36; //75497472;
  Indirect.InstanceCount = 1;
  Indirect.First = 0;
  Indirect.BaseInstance = 0;
  ProgramManager.setBinding("Compute", GL_SHADER_STORAGE_BLOCK, "DrawBuffer", DrawBindingPoint);
  BufferManager.setBuffer("DrawBuffer", GL_DRAW_INDIRECT_BUFFER, sizeof(indirectstruct), &Indirect, 0);
  BufferManager.bindBuffer("DrawBuffer", GL_SHADER_STORAGE_BUFFER, DrawBindingPoint);

  BufferManager.createFrameBuffers(2, "Multisample", "Shadows");
  GLuint Texture;

  int num_samples = 32;
  UniformManager.createTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, "Multisample");
  UniformManager.setMultisampleTexture("Multisample", num_samples, GL_RGBA8, WindowData.WindowWidth, WindowData.WindowHeight, true);
  //glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  BufferManager.createRenderBuffers(1, "RenderBuffer");
  BufferManager.setRenderBuffer("RenderBuffer", num_samples, GL_DEPTH24_STENCIL8, WindowData.WindowWidth, WindowData.WindowHeight);
  Texture = UniformManager.getTexture("Multisample");
  BufferManager.setFrameBuffer("Multisample", GL_COLOR_ATTACHMENT0, Texture, 0);
  BufferManager.setFrameBuffer("Multisample", "RenderBuffer");

  Texture = UniformManager.getTexture("Shadows");
  BufferManager.setFrameBuffer("Shadows", GL_DEPTH_ATTACHMENT, Texture, 0);
  BufferManager.setFrameBuffer("Shadows", GL_COLOR_ATTACHMENT0, 0, 0);
  BufferManager.checkFrameBuffer("Shadows");
}

void renderer::prepareUniforms() {
  GLint Location;

  Location = ProgramManager.getResourceLocation("Shadows", GL_UNIFORM, "SSMatrix");
  UniformManager.createUniformMatrix("SSMatrixShadows", 4, Location);

  Location = ProgramManager.getResourceLocation("Shadows", GL_UNIFORM, "WSMatrix");
  UniformManager.createUniformMatrix("WSMatrixShadows", 4, Location);

  Location = ProgramManager.getResourceLocation("Main", GL_UNIFORM, "SSMatrix");
  UniformManager.createUniformMatrix("SSMatrixMain", 4, Location);

  Location = ProgramManager.getResourceLocation("Main", GL_UNIFORM, "CSMatrix");
  UniformManager.createUniformMatrix("CSMatrixMain", 4, Location);

  Location = ProgramManager.getResourceLocation("Main", GL_UNIFORM, "WSMatrix");
  UniformManager.createUniformMatrix("WSMatrixMain", 4, Location);
  
  Location = ProgramManager.getResourceLocation("Main", GL_UNIFORM, "NumLights");
  UniformManager.createUniform("NumLights", 1, Location);

  Location = ProgramManager.getResourceLocation("Main", GL_UNIFORM, "CameraPosition");
  UniformManager.createUniform("CameraPosition", 3, Location);

  Location = ProgramManager.getResourceLocation("Main", GL_UNIFORM, "SunPosition");
  UniformManager.createUniform("SunPositionMain", 3, Location);

  ProgramManager.installProgram("Main");
  UniformManager.setUniform("NumLights", 2);
  
  Location = ProgramManager.getResourceLocation("Skydome", GL_UNIFORM, "SkydomeMatrix");
  UniformManager.createUniformMatrix("SkydomeMatrix", 4, Location);

  Location = ProgramManager.getResourceLocation("Skydome", GL_UNIFORM, "StarMatrix");
  UniformManager.createUniformMatrix("StarMatrix", 4, Location);

  Location = ProgramManager.getResourceLocation("Skydome", GL_UNIFORM, "SunPosition");
  UniformManager.createUniform("SunPositionSkydome", 3, Location);

  Location = ProgramManager.getResourceLocation("Skydome", GL_UNIFORM, "weather");
  UniformManager.createUniform("Weather", 1, Location);

  Location = ProgramManager.getResourceLocation("Skydome", GL_UNIFORM, "time");
  UniformManager.createUniform("Time", 1, Location);

  Location = ProgramManager.getResourceLocation("Compute", GL_UNIFORM, "Offset");
  UniformManager.createUniform("Offset", 1, Location);
}

void renderer::prepareState() {
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);
  //glDepthFunc(GL_LEQUAL); //glDepthFunc(GL_LESS);
  glClearColor(0.0, 0.0, 0.0, 1.0);
}

void renderer::draw() {
  dispatchEncoders();
  if (NewCameraData.DataIsReady.load()) {
    std::copy(NewCameraData.SSMatrix, NewCameraData.SSMatrix + 16, CurrentCameraData.SSMatrix);
    std::copy(NewCameraData.CSMatrix, NewCameraData.CSMatrix + 16, CurrentCameraData.CSMatrix);
    std::copy(NewCameraData.WSMatrix, NewCameraData.WSMatrix + 16, CurrentCameraData.WSMatrix);
    std::copy(NewCameraData.Position, NewCameraData.Position + 3, CurrentCameraData.Position);
    std::copy(NewCameraData.SkydomeMatrix, NewCameraData.SkydomeMatrix + 16, CurrentCameraData.SkydomeMatrix);
    std::copy(NewCameraData.StarMatrix, NewCameraData.StarMatrix + 16, CurrentCameraData.StarMatrix);
    std::copy(NewCameraData.SunPosition, NewCameraData.SunPosition + 3, CurrentCameraData.SunPosition);
    CurrentCameraData.Weather = NewCameraData.Weather;
    CurrentCameraData.Time = NewCameraData.Time;
    NewCameraData.DataIsReady.store(false); // Handshaking: State says true and Renderer changes data, Renderer says false and State changed data.
  }

  BufferManager.bindFrameBuffer("Shadows", GL_FRAMEBUFFER);
  glViewport(0, 0, 1024, 1024);
  glClear(GL_DEPTH_BUFFER_BIT);
  //glCullFace(GL_FRONT);
  ProgramManager.installProgram("Shadows");
  UniformManager.setUniform("SSMatrixShadows", (GLfloat*)(CurrentCameraData.SSMatrix));
  UniformManager.setUniform("WSMatrixShadows", (GLfloat*)(CurrentCameraData.WSMatrix));
  glDrawArraysIndirect(GL_TRIANGLES, 0); //36

  BufferManager.bindFrameBuffer("Multisample", GL_FRAMEBUFFER);
  glViewport(0, 0, WindowData.WindowWidth, WindowData.WindowHeight);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //glCullFace(GL_BACK);
  
  glDepthFunc(GL_LESS);
  ProgramManager.installProgram("Main");
  UniformManager.setUniform("SSMatrixMain", (GLfloat*)(CurrentCameraData.SSMatrix));
  UniformManager.setUniform("CSMatrixMain", (GLfloat*)(CurrentCameraData.CSMatrix));
  UniformManager.setUniform("WSMatrixMain", (GLfloat*)(CurrentCameraData.WSMatrix));
  UniformManager.setUniform("CameraPosition", (GLfloat*)(CurrentCameraData.Position));
  UniformManager.setUniform("SunPositionMain", (GLfloat*)(CurrentCameraData.SunPosition));
  glDrawArraysIndirect(GL_TRIANGLES, 0); //36
  //glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 1110, 6876, 1, 0);
  //glDrawArrays(GL_TRIANGLES, 0, 7000);

  glDepthFunc(GL_LEQUAL);
  ProgramManager.installProgram("Skydome");
  UniformManager.setUniform("SkydomeMatrix", (GLfloat*)(CurrentCameraData.SkydomeMatrix));
  UniformManager.setUniform("StarMatrix", (GLfloat*)(CurrentCameraData.StarMatrix));
  UniformManager.setUniform("SunPositionSkydome", (GLfloat*)(CurrentCameraData.SunPosition));
  UniformManager.setUniform("Weather", CurrentCameraData.Weather);
  UniformManager.setUniform("Time", CurrentCameraData.Time);
  glDrawArrays(GL_PATCHES, 0, 12);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  BufferManager.bindFrameBuffer("Multisample", GL_READ_FRAMEBUFFER);
  glDrawBuffer(GL_BACK);
  glBlitFramebuffer(0, 0, WindowData.WindowWidth, WindowData.WindowHeight, 0, 0, WindowData.WindowWidth, WindowData.WindowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  swapBuffers();
}

void renderer::dispatchEncoders() {
  if (EncoderData.FileIn) {
    ProgramManager.installProgram("Compute");
    UniformManager.setUniform("Offset", EncoderData.Offset);
    char* Pointer = (char*)BufferManager.getBufferAddress("InputBuffer");
    if (EncoderData.SyncObject) {
      GLenum waitReturn = GL_UNSIGNALED;
      while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
        waitReturn = glClientWaitSync(EncoderData.SyncObject, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
      EncoderData.FileIn.read(Pointer, EncoderData.BufferSizeBytes);
      glDispatchCompute(EncoderData.DispatchSize[0], EncoderData.DispatchSize[1], EncoderData.DispatchSize[2]); // Max 8 x 8 x 8
      EncoderData.Offset += EncoderData.BufferSizeBlocks;
    }
    glDeleteSync(EncoderData.SyncObject);
    EncoderData.SyncObject = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  }
}

void renderer::cleanUp() {
  EncoderData.FileIn.close();
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
