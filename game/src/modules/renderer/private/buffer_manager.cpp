#include "classes/buffer_manager.h"

buffermanager::buffermanager() {
  LogString += '\n';
}

void buffermanager::bindBuffer(const std::string BufferName) {
  auto BufferIterator = findResource(BufferName, BUFFER);
  if (BufferIterator != BufferArray.end()) {
    glBindBuffer(BufferIterator->Type, BufferIterator->Handle);
    LogString += "Buffer '" + BufferName + "' bound to context.\n";
  }
}

void buffermanager::bindBuffer(const std::string BufferName, GLuint BindingPoint) {
  auto BufferIterator = findResource(BufferName, BUFFER);
  if (BufferIterator != BufferArray.end()) {
    glBindBuffer(BufferIterator->Type, BufferIterator->Handle);
    glBindBufferBase(BufferIterator->Type, BindingPoint, BufferIterator->Handle);
    LogString += "Buffer '" + BufferName + "' bound to binding point " + std::to_string(BindingPoint) + ".\n";
  }
}

void buffermanager::setVertexArray(const std::string VertexArrayName, int AttributeIndex, int ComponentSize, const GLenum DataType, bool Normalised, int Stride, const void* Offset) {
  auto VertexArrayIterator = findResource(VertexArrayName, VERTEXARRAY);
  if (VertexArrayIterator != VertexArrayArray.end()) {
    VertexArrayIterator->Location = AttributeIndex;
    //glEnableVertexArrayAttrib(VertexIterator->Handle, AttributeIndex);
    //glVertexArrayAttribFormat(VertexIterator->Handle, AttributeIndex, )
    //glBindVertexArray(VertexArrayIterator->Handle);
    glVertexAttribPointer(AttributeIndex, ComponentSize, DataType, Normalised, Stride, Offset);
    LogString += "VertexArray '" + VertexArrayName + "' set.\n";
  }
}

std::string buffermanager::getLog() {
  std::string Log = LogString;
  LogString = '\n';
  return Log;
}

void buffermanager::cleanUp() {
  for (auto BufferIterator : BufferArray) {
    glDeleteBuffers(1, &BufferIterator.Handle);
  }
  for (auto VertexArrayIterator : VertexArrayArray) {
    glDeleteVertexArrays(1, &VertexArrayIterator.Handle);
  }
}

buffermanager::~buffermanager() {
}

void buffermanager::createBufferHelper(GLuint* Buffers, int& Index, const std::string BufferName) {
  Index--;
  resource ResourceVar;
  ResourceVar.Name = BufferName;
  ResourceVar.Handle = Buffers[Index];
  BufferArray.push_back(ResourceVar);
  LogString += "Buffer '" + BufferName + "' created.\n";
}

void buffermanager::setBuffer(const std::string BufferName, const GLenum BufferType, int Size, const void* Data, GLenum Usage) {
  auto BufferIterator = findResource(BufferName, BUFFER);
  if (BufferIterator != BufferArray.end()) {
    BufferIterator->Type = BufferType;
    //glBindBuffer(BufferType, BufferIterator->Handle);
    //glBufferData(BufferType, Size, Data, Usage);
    glNamedBufferData(BufferIterator->Handle, Size, Data, Usage);
    LogString += "Buffer '" + BufferName + "' set.\n";
  }
}

/*void buffermanager::setRenderBuffer(const std::string RenderBufferName, int Samples, GLenum InternalFormat, int Width, int Height) {
  auto RenderBufferIterator = findResource(RenderBufferName, BUFFER);
  if (RenderBufferIterator != BufferArray.end()) {
    glNamedRenderbufferStorageMultisample(RenderBufferIterator->Handle, Samples, InternalFormat, Width, Height);
    LogString += "Multisample renderbuffer '" + RenderBufferName + "' set.\n";
  }
}

void buffermanager::setRenderBuffer(const std::string RenderBufferName, GLenum InternalFormat, int Width, int Height) {
  auto RenderBufferIterator = findResource(RenderBufferName, BUFFER);
  if (RenderBufferIterator != BufferArray.end()) {
    glNamedRenderbufferStorage(RenderBufferIterator->Handle, InternalFormat, Width, Height);
    LogString += "Renderbuffer '" + RenderBufferName + "' set.\n";
  }
}

void buffermanager::setFrameBuffer(const std::string FrameBufferName, GLenum Target, GLenum Attachment, GLenum ResourceTarget, const std::string RenderBufferName) {
  auto FrameBufferIterator = findResource(FrameBufferName, BUFFER);
  auto RenderBufferIterator = findResource(RenderBufferName, BUFFER);
  if (FrameBufferIterator != BufferArray.end() && RenderBufferIterator != BufferArray.end()) {
    glNamedFramebufferRenderbuffer(FrameBufferIterator->Handle, Attachment, ResourceTarget, RenderBufferIterator->Handle);
    LogString += "Renderbuffer '"  + RenderBufferName + "' attached to framebuffer '" + FrameBufferName + "'.\n";
  }
}

void buffermanager::setFrameBuffer(const std::string FrameBufferName, GLenum Target, GLenum Attachment, GLenum ResourceTarget, GLuint TextureHandle, int Level) {
  auto FrameBufferIterator = findResource(FrameBufferName, BUFFER);
  if (FrameBufferIterator != BufferArray.end()) {
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBufferIterator->Handle);
    switch (ResourceTarget) {
      case GL_TEXTURE_1D: glFramebufferTexture1D(Target, Attachment, ResourceTarget, TextureHandle, Level);
        break;
      default: glFramebufferTexture2D(Target, Attachment, ResourceTarget, TextureHandle, Level);
    LogString += "Texture attached to framebuffer '" + FrameBufferName + "'.\n";
    }
  }
}

void buffermanager::setFrameBuffer(const std::string FrameBufferName, GLenum Target, GLenum Attachment, GLenum ResourceTarget, GLuint TextureHandle, int Level, int Layer) {
  auto FrameBufferIterator = findResource(FrameBufferName, BUFFER);
  if (FrameBufferIterator != BufferArray.end()) {
    glBindFramebuffer(GL_FRAMEBUFFER, FrameBufferIterator->Handle);
    glFramebufferTexture1D(Target, Attachment, ResourceTarget, TextureHandle, Level);
    LogString += "Texture attached to framebuffer '" + FrameBufferName + "'.\n";
  }
}*/
  
void buffermanager::createVertexArrayHelper(GLuint* VertexArrays, int& Index, const std::string VertexArrayName) {
  Index--;
  resource ResourceVar;
  ResourceVar.Name = VertexArrayName;
  ResourceVar.Handle = VertexArrays[Index];
  VertexArrayArray.push_back(ResourceVar);
  glBindVertexArray(VertexArrays[Index]);
  LogString += "Vertex Array '" + VertexArrayName + "' created.\n";
}

auto buffermanager::findResource(const std::string ResourceName, resourcetype ResourceType) -> std::vector<resource>::iterator {
  std::vector<resource>* ResourceArray;
  switch (ResourceType) {
    case BUFFER: ResourceArray = &BufferArray;
      break;
    case VERTEXARRAY: ResourceArray = &VertexArrayArray;
  }
  auto ResourceIterator = std::find_if(ResourceArray->begin(), ResourceArray->end(), 
      [&] (const resource& ResourceVar) {return ResourceVar.Name == ResourceName;});
  if (ResourceIterator != ResourceArray->end()) {
    return ResourceIterator;
  } else {
    LogString = "Warning: Resource '" + ResourceName + "' not found.\n";
    return ResourceIterator;
  }
}
