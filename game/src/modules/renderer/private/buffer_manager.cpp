#include "classes/buffer_manager.h"

buffermanager::buffermanager() {
  LogString += '\n';
}

void buffermanager::setBuffer(const std::string BufferName, const GLenum BufferType, int Size, const void* Data, GLenum Usage) {
  auto BufferIterator = findBuffer(BufferName);
  if (BufferIterator != BufferArray.end()) {
    BufferIterator->Type = BufferType;
    glNamedBufferData(BufferIterator->Handle, Size, Data, Usage);
    LogString += "Buffer '" + BufferName + "' set.\n";
  }
}

void buffermanager::bindBuffer(const std::string BufferName) {
  auto BufferIterator = findBuffer(BufferName);
  if (BufferIterator != BufferArray.end()) {
    glBindBuffer(BufferIterator->Type, BufferIterator->Handle);
    LogString += "Buffer '" + BufferName + "' bound to context.\n";
  }
}

void buffermanager::bindBuffer(const std::string BufferName, GLuint BindingPoint) {
  auto BufferIterator = findBuffer(BufferName);
  if (BufferIterator != BufferArray.end()) {
    glBindBuffer(BufferIterator->Type, BufferIterator->Handle);
    glBindBufferBase(BufferIterator->Type, BindingPoint, BufferIterator->Handle);
    LogString += "Buffer '" + BufferName + "' bound to binding point " + std::to_string(BindingPoint) + ".\n";
  }
}

void buffermanager::bindBuffer(const std::string BufferName, GLenum CustomType, GLuint BindingPoint) {
  auto BufferIterator = findBuffer(BufferName);
  if (BufferIterator != BufferArray.end()) {
    glBindBuffer(BufferIterator->Type, BufferIterator->Handle);
    glBindBufferBase(CustomType, BindingPoint, BufferIterator->Handle);
    LogString += "Buffer '" + BufferName + "' bound to binding point " + std::to_string(BindingPoint) + ".\n";
  }
}

void buffermanager::getBuffer(const std::string BufferName, int Offset, int Size, void* Data) {
  auto BufferIterator = findBuffer(BufferName);
  if (BufferIterator != BufferArray.end()) {
    glGetNamedBufferSubData(BufferIterator->Handle, Offset, Size, Data);
    LogString += "Buffer '" + BufferName + "' read.\n";
  }
}

void buffermanager::setRenderBuffer(const std::string RenderBufferName, int Samples, GLenum InternalFormat, int Width, int Height) {
  auto RenderBufferIterator = findRenderBuffer(RenderBufferName);
  if (RenderBufferIterator != RenderBufferArray.end()) {
    glNamedRenderbufferStorageMultisample(RenderBufferIterator->Handle, Samples, InternalFormat, Width, Height);
    LogString += "Multisample renderbuffer '" + RenderBufferName + "' set.\n";
  }
}

void buffermanager::setRenderBuffer(const std::string RenderBufferName, GLenum InternalFormat, int Width, int Height) {
  auto RenderBufferIterator = findRenderBuffer(RenderBufferName);
  if (RenderBufferIterator != RenderBufferArray.end()) {
    glNamedRenderbufferStorage(RenderBufferIterator->Handle, InternalFormat, Width, Height);
    LogString += "Renderbuffer '" + RenderBufferName + "' set.\n";
  }
}

void buffermanager::bindRenderBuffer(const std::string RenderBufferName) {
  auto RenderBufferIterator = findRenderBuffer(RenderBufferName);
  if (RenderBufferIterator != RenderBufferArray.end()) {
    glBindRenderbuffer(GL_RENDERBUFFER, RenderBufferIterator->Handle);
    LogString += "RenderBuffer '" + RenderBufferName + "' bound to context.\n";
  }
}

void buffermanager::setFrameBuffer(const std::string FrameBufferName, const std::string RenderBufferName) {
  auto FrameBufferIterator = findFrameBuffer(FrameBufferName);
  auto RenderBufferIterator = findRenderBuffer(RenderBufferName);
  if (FrameBufferIterator != FrameBufferArray.end() && RenderBufferIterator != RenderBufferArray.end()) {
    glNamedFramebufferRenderbuffer(FrameBufferIterator->Handle, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RenderBufferIterator->Handle);
    LogString += "Renderbuffer '"  + RenderBufferName + "' attached to framebuffer '" + FrameBufferName + "'.\n";
  }
}

void buffermanager::setFrameBuffer(const std::string FrameBufferName, GLenum Attachment, GLuint TextureHandle, int Level) {
  auto FrameBufferIterator = findFrameBuffer(FrameBufferName);
  if (FrameBufferIterator != FrameBufferArray.end()) {
      glNamedFramebufferTexture(FrameBufferIterator->Handle, Attachment, TextureHandle, Level);
    LogString += "Texture attached to framebuffer '" + FrameBufferName + "'.\n";
  }
}

void buffermanager::setFrameBuffer(const std::string FrameBufferName, GLenum Attachment, GLuint TextureHandle, int Level, int Layer) {
  auto FrameBufferIterator = findFrameBuffer(FrameBufferName);
  if (FrameBufferIterator != FrameBufferArray.end()) {
    glNamedFramebufferTextureLayer(FrameBufferIterator->Handle, Attachment, TextureHandle, Level, Layer);
    LogString += "Texture attached to framebuffer '" + FrameBufferName + "'.\n";
  }
}
  
void buffermanager::bindFrameBuffer(const std::string FrameBufferName, GLenum FrameBufferType) {
  auto FrameBufferIterator = findFrameBuffer(FrameBufferName);
  if (FrameBufferIterator != FrameBufferArray.end()) {
    glBindFramebuffer(FrameBufferType, FrameBufferIterator->Handle);
    LogString += "FrameBuffer '" + FrameBufferName + "' bound to context.\n";
  }
}

std::string buffermanager::getLog() {
  std::string Log = LogString;
  LogString = '\n';
  return Log;
}

void buffermanager::cleanUp() {
  for (auto BufferIterator : BufferArray)
    glDeleteBuffers(1, &BufferIterator.Handle); // Cannot be done all at once, because buffer is a struct not an int.
  for (auto BufferIterator : RenderBufferArray)
    glDeleteRenderbuffers(1, &BufferIterator.Handle);
  for (auto BufferIterator : FrameBufferArray)
    glDeleteFramebuffers(1, &BufferIterator.Handle);
}

buffermanager::~buffermanager() {
}

void buffermanager::createBufferHelper(GLuint* Buffers, int& Index, const std::string BufferName) {
  Index--;
  buffer BufferVar;
  BufferVar.Name = BufferName;
  BufferVar.Handle = Buffers[Index];
  BufferArray.push_back(BufferVar);
  LogString += "Buffer '" + BufferName + "' created.\n";
}

void buffermanager::createRenderBufferHelper(GLuint* RenderBuffers, int& Index, const std::string RenderBufferName) {
  Index--;
  renderbuffer RenderBufferVar;
  RenderBufferVar.Name = RenderBufferName;
  RenderBufferVar.Handle = RenderBuffers[Index];
  RenderBufferArray.push_back(RenderBufferVar);
  LogString += "Renderbuffer '" + RenderBufferName + "' created.\n";
}

void buffermanager::createFrameBufferHelper(GLuint* FrameBuffers, int& Index, const std::string FrameBufferName) {
  Index--;
  framebuffer FrameBufferVar;
  FrameBufferVar.Name = FrameBufferName;
  FrameBufferVar.Handle = FrameBuffers[Index];
  FrameBufferArray.push_back(FrameBufferVar);
  LogString += "Framebuffer '" + FrameBufferName + "' created.\n";
}

auto buffermanager::findBuffer(const std::string BufferName) -> std::vector<buffer>::iterator {
  auto BufferIterator = std::find_if(BufferArray.begin(), BufferArray.end(), 
      [&] (const buffer& BufferVar) {return BufferVar.Name == BufferName;});
  if (BufferIterator == BufferArray.end())
    LogString = "Warning: Buffer '" + BufferName + "' not found.\n";
  return BufferIterator;
}

auto buffermanager::findRenderBuffer(const std::string RenderBufferName) -> std::vector<renderbuffer>::iterator {
  auto RenderBufferIterator = std::find_if(RenderBufferArray.begin(), RenderBufferArray.end(), 
      [&] (const renderbuffer& RenderBufferVar) {return RenderBufferVar.Name == RenderBufferName;});
  if (RenderBufferIterator == RenderBufferArray.end())
    LogString = "Warning: RenderBuffer '" + RenderBufferName + "' not found.\n";
  return RenderBufferIterator;
}

auto buffermanager::findFrameBuffer(const std::string FrameBufferName) -> std::vector<framebuffer>::iterator {
  auto FrameBufferIterator = std::find_if(FrameBufferArray.begin(), FrameBufferArray.end(), 
      [&] (const framebuffer& FrameBufferVar) {return FrameBufferVar.Name == FrameBufferName;});
  if (FrameBufferIterator == FrameBufferArray.end())
    LogString = "Warning: FrameBuffer '" + FrameBufferName + "' not found.\n";
  return FrameBufferIterator;
}
