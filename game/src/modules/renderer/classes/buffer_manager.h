#ifndef _MODULE_RENDERER_BUFFER_MANAGER_H_
#define _MODULE_RENDERER_BUFFER_MANAGER_H_

#include <vector>
#include <string>
#include <algorithm>
#include <memory>

#include "external/include/glad.h"

class buffermanager {
 public:
  buffermanager();

  template<typename ...args>
    void createBuffers(int Amount, const std::string BufferName, args... Arguments) {
      GLuint* Buffers = new GLuint[Amount];
      //glGenBuffers(Amount, Buffers);
      glCreateBuffers(Amount, Buffers);
      createBufferHelper(Buffers, Amount, BufferName, Arguments...);
      delete [] Buffers;
    }

  void setBuffer(const std::string BufferName, const GLenum BufferType, int Size, const void* Data, GLbitfield Usage);
  void bindBuffer(const std::string BufferName);
  void bindBuffer(const std::string BufferName, GLuint BindingPoint);
  void bindBuffer(const std::string BufferName, GLenum CustomType, GLuint BindingPoint);
  void getBuffer(const std::string BufferName, int Offset, int Size, void* Data);
  void mapBuffer(const std::string BufferName, int Offset, int Size, const GLbitfield Access);
  void resetBuffer(const std::string BufferName, int Offset, int Size, const void* Data);
  void* getBufferAddress(const std::string BufferName);

  template<typename ...args>
    void createRenderBuffers(int Amount, const std::string RenderBufferName, args... Arguments) {
      GLuint* RenderBuffers = new GLuint[Amount];
      //glGenRenderBuffers(Amount, RenderBuffers);
      glCreateRenderbuffers(Amount, RenderBuffers);
      createRenderBufferHelper(RenderBuffers, Amount, RenderBufferName, Arguments...);
      delete [] RenderBuffers;
    }

  void setRenderBuffer(const std::string RenderBufferName, int Samples, GLenum InternalFormat, int Width, int Height);
  void setRenderBuffer(const std::string RenderBufferName, GLenum InternalFormat, int Width, int Height);
  void bindRenderBuffer(const std::string RenderBufferName);

  template<typename ...args>
    void createFrameBuffers(int Amount, const std::string FrameBufferName, args... Arguments) {
      GLuint* FrameBuffers = new GLuint[Amount];
      //glGenFrameBuffers(Amount, FrameBuffers);
      glCreateFramebuffers(Amount, FrameBuffers);
      createFrameBufferHelper(FrameBuffers, Amount, FrameBufferName, Arguments...);
      delete [] FrameBuffers;
    }

  void setFrameBuffer(const std::string FrameBufferName, const std::string RenderBufferName);
  void setFrameBuffer(const std::string FrameBufferName, GLenum Attachment, GLuint TextureHandle, int Level);
  void setFrameBuffer(const std::string FrameBufferName, GLenum Attachment, GLuint TextureHandle, int Level, int Layer);
  void setDrawBuffer(const std::string FrameBufferName, GLenum DrawBuffer);
  void setDrawBuffer(const std::string FrameBufferName, int Size, GLenum* DrawBuffer);
  void setReadBuffer(const std::string FrameBufferName, GLenum ReadBuffer);
  void checkFrameBuffer(std::string Framebuffer); // INCOMPLETE!!
  void bindFrameBuffer(const std::string FrameBufferName, GLenum FrameBufferType);

  std::string getLog();
  void cleanUp();
  ~buffermanager();

 private:
  struct buffer {
    std::string Name;
    GLenum Type;
    void* BufferAddress;
    GLuint Handle;
  };
  struct renderbuffer {
    std::string Name;
    GLuint Handle;
  };
  struct framebuffer {
    std::string Name;
    GLuint Handle;
  };

  template<typename ...args>
    void createBufferHelper(GLuint* Buffers, int& Index, const std::string BufferName, args... Arguments) {
      //Index--;
      createBufferHelper(Buffers, Index, BufferName);
      createBufferHelper(Buffers, Index, Arguments...);
    }
  void createBufferHelper(GLuint* Buffers, int& Index, const std::string BufferName);

  template<typename ...args>
    void createRenderBufferHelper(GLuint* RenderBuffers, int& Index, const std::string RenderBufferName, args... Arguments) {
      //Index--;
      createRenderBufferHelper(RenderBuffers, Index, RenderBufferName);
      createRenderBufferHelper(RenderBuffers, Index, Arguments...);
    }
  void createRenderBufferHelper(GLuint* RenderBuffers, int& Index, const std::string RenderBufferName);

  template<typename ...args>
    void createFrameBufferHelper(GLuint* FrameBuffers, int& Index, const std::string FrameBufferName, args... Arguments) {
      //Index--;
      createFrameBufferHelper(FrameBuffers, Index, FrameBufferName);
      createFrameBufferHelper(FrameBuffers, Index, Arguments...);
    }
  void createFrameBufferHelper(GLuint* FrameBuffers, int& Index, const std::string FrameBufferName);

  std::vector<buffer>::iterator findBuffer(const std::string BufferName);
  std::vector<renderbuffer>::iterator findRenderBuffer(const std::string RenderBufferName);
  std::vector<framebuffer>::iterator findFrameBuffer(const std::string FrameBufferName);

  std::vector<buffer> BufferArray;
  std::vector<renderbuffer> RenderBufferArray;
  std::vector<framebuffer> FrameBufferArray;

  std::string LogString;
};

#endif // _MODULE_RENDERER_BUFFER_MANAGER_H_
