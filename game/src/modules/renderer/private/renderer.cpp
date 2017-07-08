#include "renderer.h"

/*Constants
 * 1 Encoded block = 128 bits
 * 1 Decoded block = 15 * 32 = 480 bits
 * 1 Chunck = 2^21 blocks * 480 bits / (8 * 1024 * 1024) = 120MB
 * 1 Complete = 2^32 blocks * 480 bytes / (8 * 1024 * 1024 * 1024) = 240TB
 * 1 Light = 9 * 32 = 288 bits
 */

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
  BlockData.DispatchSize[0] = 2;
  BlockData.DispatchSize[1] = 1;
  BlockData.DispatchSize[2] = 1;
  BlockData.DispatchVolume = BlockData.DispatchSize[0] * BlockData.DispatchSize[1] * BlockData.DispatchSize[2];
  BlockData.InputBuffer.Blocks = 512 * BlockData.DispatchVolume; // Local dispatch size = 8x8x8 = 512
  BlockData.InputBuffer.Bytes = BlockData.InputBuffer.Blocks * 128 / 8; // 128 bits / 8 bits(one byte)
  BlockData.ComputeShaderOffset = BlockData.InputBuffer.Blocks;
  BlockData.StorageBuffer.Blocks = 2097152;
  BlockData.StorageBuffer.Bytes = 2097152 * 480 / 8;
  BlockData.LightBuffer.Blocks = 10;
  BlockData.LightBuffer.Bytes = 10 * 288 / 8;
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
  BlockData.FileIn.open("content/blockarray", std::ios::in | std::ios::binary);
  if (!BlockData.FileIn)
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
  glDispatchCompute(BlockData.DispatchSize[0], BlockData.DispatchSize[1], BlockData.DispatchSize[2]); // Max 8 x 8 x 8
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
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
  ProgramManager.createShader("intermediate/compute.comp", GL_COMPUTE_SHADER);
  ProgramManager.createProgram("Compute");
  ProgramManager.addShader("Compute", "intermediate/compute.comp");
  ProgramManager.linkProgram("Compute");

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

  ProgramManager.createShader("intermediate/skydome.vert", GL_VERTEX_SHADER);
  ProgramManager.createShader("intermediate/skydome.tesc", GL_TESS_CONTROL_SHADER);
  ProgramManager.createShader("intermediate/skydome.tese", GL_TESS_EVALUATION_SHADER);
  ProgramManager.createShader("intermediate/skydome.frag", GL_FRAGMENT_SHADER);
  ProgramManager.createProgram("Skydome");
  ProgramManager.addShader("Skydome", "intermediate/skydome.vert", "intermediate/skydome.tesc", "intermediate/skydome.tese", "intermediate/skydome.frag");
  ProgramManager.linkProgram("Skydome");

  ProgramManager.createShader("intermediate/texture_pass.vert", GL_VERTEX_SHADER);
  ProgramManager.createShader("intermediate/bloom.frag", GL_FRAGMENT_SHADER);
  ProgramManager.createProgram("Bloom");
  ProgramManager.addShader("Bloom", "intermediate/texture_pass.vert", "intermediate/bloom.frag");
  ProgramManager.linkProgram("Bloom");

  ProgramManager.createShader("intermediate/texture_pass.frag", GL_FRAGMENT_SHADER);
  ProgramManager.createProgram("TexturePass");
  ProgramManager.addShader("TexturePass", "intermediate/texture_pass.vert", "intermediate/texture_pass.frag");
  ProgramManager.linkProgram("TexturePass");
}

void renderer::prepareTextures() {
  GLint Location;
  UniformManager.createTextures(GL_TEXTURE_2D, 12, "Shadows", "Tint1", "Tint2", "Sun", "Moon", "Clouds1", "Clouds2", "MainPass", "BrightColour", "BloomVertical", "BloomHorizontal");
 
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
 
  ProgramManager.installProgram("TexturePass");
  Location = ProgramManager.getResourceLocation("TexturePass", GL_UNIFORM, "Texture");
  UniformManager.setBlankTexture("MainPass", GL_RGBA16F, WindowData.WindowWidth, WindowData.WindowHeight, Location);
  UniformManager.setDefaultParameters("MainPass", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
  UniformManager.setBlankTexture("BrightColour", GL_RGBA16F, WindowData.WindowWidth, WindowData.WindowHeight, -1);
  UniformManager.setDefaultParameters("BrightColour", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

  Location = ProgramManager.getResourceLocation("TexturePass", GL_UNIFORM, "Bloom");
  UniformManager.setBlankTexture("BloomVertical", GL_RGBA16F, WindowData.WindowWidth, WindowData.WindowHeight, Location);
  UniformManager.setDefaultParameters("BloomVertical", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
  UniformManager.setBlankTexture("BloomHorizontal", GL_RGBA16F, WindowData.WindowWidth, WindowData.WindowHeight, -1);
  UniformManager.setDefaultParameters("BloomHorizontal", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

void renderer::prepareBuffers() {
  BufferManager.createBuffers(6, "InputBuffer", "StorageBuffer", "ColourBuffer", "LightBuffer", "LightCount", "DrawBuffer");

  GLuint InputBindingPoint = 1;
  std::vector<uint64_t> Vector(BlockData.InputBuffer.Blocks * 2, 0); // 1 block = 2X 64-bit
  if (BlockData.FileIn)
    BlockData.FileIn.read((char*)Vector.data(), BlockData.InputBuffer.Bytes);
  ProgramManager.setBinding("Compute", GL_SHADER_STORAGE_BLOCK, "InputBuffer", InputBindingPoint);
  BufferManager.setBuffer("InputBuffer", GL_SHADER_STORAGE_BUFFER, BlockData.InputBuffer.Bytes, Vector.data(), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT); // Actual Size: 4194304
  BufferManager.mapBuffer("InputBuffer", 0, BlockData.InputBuffer.Bytes, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  BufferManager.bindBuffer("InputBuffer", InputBindingPoint);

  GLuint StorageBindingPoint = 2;
  ProgramManager.setBinding("Compute", GL_SHADER_STORAGE_BLOCK, "StorageBuffer", StorageBindingPoint);
  ProgramManager.setBinding("Shadows", GL_SHADER_STORAGE_BLOCK, "StorageBuffer", StorageBindingPoint);
  ProgramManager.setBinding("Main", GL_SHADER_STORAGE_BLOCK, "StorageBuffer", StorageBindingPoint);
  BufferManager.setBuffer("StorageBuffer", GL_SHADER_STORAGE_BUFFER, BlockData.StorageBuffer.Bytes, nullptr, 0);
  BufferManager.bindBuffer("StorageBuffer", StorageBindingPoint);

  GLuint ColourBindingPoint = 3;
  ProgramManager.setBinding("Compute", GL_UNIFORM_BLOCK, "ColourUniform", ColourBindingPoint);
  BufferManager.setBuffer("ColourBuffer", GL_UNIFORM_BUFFER, sizeof(ColourArray), ColourArray, 0);
  BufferManager.bindBuffer("ColourBuffer", ColourBindingPoint);

  GLuint LightBindingPoint = 4;
  ProgramManager.setBinding("Main", GL_SHADER_STORAGE_BLOCK, "LightBuffer", LightBindingPoint);
  ProgramManager.setBinding("Compute", GL_SHADER_STORAGE_BLOCK, "LightBuffer", LightBindingPoint);
  BufferManager.setBuffer("LightBuffer", GL_SHADER_STORAGE_BUFFER, BlockData.LightBuffer.Bytes, nullptr, 0);
  BufferManager.bindBuffer("LightBuffer", LightBindingPoint);

  GLuint LightCountBindingPoint = 0;
  BufferManager.setBuffer("LightCount", GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &LightCountBindingPoint, 0);
  BufferManager.bindBuffer("LightCount", LightCountBindingPoint);

  GLuint DrawBindingPoint = 6;
  indirectstruct Indirect;
  Indirect.Count = BlockData.InputBuffer.Blocks * 36; //75497472;
  Indirect.InstanceCount = 1;
  Indirect.First = 0;
  Indirect.BaseInstance = 0;
  ProgramManager.setBinding("Compute", GL_SHADER_STORAGE_BLOCK, "DrawBuffer", DrawBindingPoint);
  BufferManager.setBuffer("DrawBuffer", GL_DRAW_INDIRECT_BUFFER, sizeof(indirectstruct), &Indirect, 0);
  BufferManager.bindBuffer("DrawBuffer", GL_SHADER_STORAGE_BUFFER, DrawBindingPoint);

  BufferManager.createFrameBuffers(4, "Shadows", "MainPass", "BloomVertical", "BloomHorizontal");
  GLuint Texture;

  BufferManager.createRenderBuffers(1, "RenderBufferPass");

  Texture = UniformManager.getTexture("Shadows");
  BufferManager.setFrameBuffer("Shadows", GL_DEPTH_ATTACHMENT, Texture, 0);
  BufferManager.setFrameBuffer("Shadows", GL_COLOR_ATTACHMENT0, 0, 0);
  BufferManager.checkFrameBuffer("Shadows");

  Texture = UniformManager.getTexture("MainPass");
  BufferManager.setRenderBuffer("RenderBufferPass", GL_DEPTH_COMPONENT, WindowData.WindowWidth, WindowData.WindowHeight);
  BufferManager.setFrameBuffer("MainPass", "RenderBufferPass");
  BufferManager.setFrameBuffer("MainPass", GL_COLOR_ATTACHMENT0, Texture, 0);
  Texture = UniformManager.getTexture("BrightColour");
  BufferManager.setFrameBuffer("MainPass", GL_COLOR_ATTACHMENT1, Texture, 0);
  GLenum Attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  BufferManager.setDrawBuffer("MainPass", 2, Attachments);
  BufferManager.checkFrameBuffer("MainPass");

  Texture = UniformManager.getTexture("BloomVertical");
  BufferManager.setFrameBuffer("BloomVertical", GL_DEPTH_ATTACHMENT, 0, 0);
  BufferManager.setFrameBuffer("BloomVertical", GL_COLOR_ATTACHMENT0, Texture, 0);
  BufferManager.checkFrameBuffer("BloomVertical");

  Texture = UniformManager.getTexture("BloomHorizontal");
  BufferManager.setFrameBuffer("BloomHorizontal", GL_DEPTH_ATTACHMENT, 0, 0);
  BufferManager.setFrameBuffer("BloomHorizontal", GL_COLOR_ATTACHMENT0, Texture, 0);
  BufferManager.checkFrameBuffer("BloomHorizontal");
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

  Location = ProgramManager.getResourceLocation("Main", GL_UNIFORM, "CameraPosition");
  UniformManager.createUniform("CameraPosition", 3, Location);

  Location = ProgramManager.getResourceLocation("Main", GL_UNIFORM, "SunPosition");
  UniformManager.createUniform("SunPositionMain", 3, Location);
  
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

  Location = ProgramManager.getResourceLocation("Bloom", GL_UNIFORM, "Texture");
  UniformManager.createUniform("BloomTexture", 1, Location);
  Location = ProgramManager.getResourceLocation("Bloom", GL_UNIFORM, "Horizontal");
  UniformManager.createUniform("Horizontal", 1, Location);
}

void renderer::prepareState() {
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

  BufferManager.bindFrameBuffer("MainPass", GL_FRAMEBUFFER);
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

  bool Horizontal = true;
  bool First = true;
  int BlurAmount = 10;
  int Texture;
  ProgramManager.installProgram("Bloom");
  BufferManager.bindFrameBuffer("BloomVertical", GL_FRAMEBUFFER);
  for (int i = 0; i < BlurAmount; i++) {
    UniformManager.setUniform("Horizontal", Horizontal);
    if (First) {
      Texture = UniformManager.getTextureUnit("BrightColour");
      First = false;
    } else if (Horizontal) {
      BufferManager.bindFrameBuffer("BloomVertical", GL_FRAMEBUFFER);
      Texture = UniformManager.getTextureUnit("BloomHorizontal");
    } else {
      BufferManager.bindFrameBuffer("BloomHorizontal", GL_FRAMEBUFFER);
      Texture = UniformManager.getTextureUnit("BloomVertical");
    }

    UniformManager.setUniform("BloomTexture", Texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    Horizontal = !Horizontal;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ProgramManager.installProgram("TexturePass");
  glDrawArrays(GL_TRIANGLES, 0, 6);

  swapBuffers();
}

void renderer::dispatchEncoders() {
  if (BlockData.FileIn) {
    ProgramManager.installProgram("Compute");
    UniformManager.setUniform("Offset", BlockData.ComputeShaderOffset);
    char* Pointer = (char*)BufferManager.getBufferAddress("InputBuffer");
    if (BlockData.SyncObject) {
      GLenum waitReturn = GL_UNSIGNALED;
      while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
        waitReturn = glClientWaitSync(BlockData.SyncObject, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
      BlockData.FileIn.read(Pointer, BlockData.InputBuffer.Bytes);
      glDispatchCompute(BlockData.DispatchSize[0], BlockData.DispatchSize[1], BlockData.DispatchSize[2]);
      BlockData.ComputeShaderOffset += BlockData.InputBuffer.Blocks;
    }
    glDeleteSync(BlockData.SyncObject);
    BlockData.SyncObject = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  }
}

void renderer::cleanUp() {
  BlockData.FileIn.close();
  UniformManager.cleanUp();
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
