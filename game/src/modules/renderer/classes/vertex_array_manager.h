#ifndef _MODULE_RENDERER_VERTEX_ARRAY_MANAGER_H_
#define _MODULE_RENDERER_VERTEX_ARRAY_MANAGER_H_

#include <string>
#include <vector>
#include <algorithm>

#include "external/include/glad.h"

class vertexarraymanager {
 public:
  vertexarraymanager();
  //void createVertexArray;
  void createVertexArray(const std::string VertexArrayName, int Location, int AttributeSize, GLenum AttributeType, bool Normalize, int Stride, const GLvoid* StartPosition);
  void installVertexArray(const std::string VertexArrayName);
  std::string getLog();
  void cleanUp();
  ~vertexarraymanager();

 private:
  struct vertexarraystruct {
    std::string VertexArrayName;
    int VertexArrayLocation;
    GLuint VertexArrayHandle;
  };

  std::vector<vertexarraystruct>::iterator findVertexArray(const std::string VertexArrayName, bool& VertexArrayFound);

  std::vector<vertexarraystruct> VertexArrayArray;
  std::string LogString;
};

#endif
