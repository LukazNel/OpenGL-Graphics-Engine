#ifndef _MODULE_RENDERER_BUFFER_MANAGER_H_
#define _MODULE_RENDERER_BUFFER_MANAGER_H_

#include <vector>
#include <string>
#include <algorithm>
#include <memory>

#include "external/include/glad.h"

enum resourcetype {BUFFER, VERTEXARRAY};

class buffermanager {
 public:
  buffermanager();
  template<typename ...args>
    void createBuffers(int Amount, const std::string BufferName, args... Arguments) {
      GLuint Buffers[Amount];
      //glGenBuffers(Amount, Buffers);
      glCreateBuffers(Amount, Buffers);
      int Index = Amount;
      createBufferHelper(Buffers, Index, BufferName, Arguments...);
    }
  template<typename ...args>
    void createVertexArrays(int Amount, const std::string VertexArrayName, args... Arguments) {
      GLuint VertexArrays[Amount];
      glGenVertexArrays(Amount, VertexArrays);
      int Index = Amount;
      createVertexArrayHelper(VertexArrays, Index, VertexArrayName, Arguments...);
    }
  void setBuffer(const std::string BufferName, const GLenum BufferType, int Size, const void* Data, GLenum Usage);
  void setVertexArray(const std::string VertexArrayName, int AttributeIndex, int ComponentSize, const GLenum DataType, bool Normalised, int Stride, const void* Offset);
  template<typename ...args>
    void installVertexArrays(const std::string VertexArrayName, args... Arguments) {
      installVertexArrays(VertexArrayName);
      installVertexArrays(Arguments...);
    }
  void installVertexArrays(const std::string VertexArrayName) {
      auto VertexArrayIterator = findResource(VertexArrayName, VERTEXARRAY);
      if (VertexArrayIterator != VertexArrayArray.end())
        glEnableVertexAttribArray(VertexArrayIterator->Location);
  }
  void bindBuffer(const std::string BufferName);
  void bindBuffer(const std::string BufferName, GLuint BindingPoint);
  std::string getLog();
  void cleanUp();
  ~buffermanager();

 private:
  struct resource {
    std::string Name;
    GLenum Type;
    int Location;
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
    void createVertexArrayHelper(GLuint* VertexArrays, int& Index, const std::string VertexArrayName, args... Arguments) {
      //Index--;
      createVertexArrayHelper(VertexArrays, Index, VertexArrayName);
      createVertexArrayHelper(VertexArrays, Index, Arguments...);
    }
  void createVertexArrayHelper(GLuint* VertexArrays, int& Index, const std::string VertexArrayName);

  std::vector<resource>::iterator findResource(const std::string ResourceName, resourcetype ResourceType);

  std::vector<resource> BufferArray;
  std::vector<resource> VertexArrayArray;
  std::string LogString;
};

#endif // _MODULE_RENDERER_BUFFER_MANAGER_H_
