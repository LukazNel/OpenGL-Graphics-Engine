#ifndef _MODULE_RENDERER_UNIFORM_MANAGER_H_
#define _MODULE_RENDERER_UNIFORM_MANAGER_H_

#include <string>
#include <vector>
#include <algorithm>

#include "external/include/glad.h"

class uniformmanager {
 public:
  uniformmanager();
  void createUniform(const std::string UniformName, int Size, GLuint UniformLocation);
  void createUniformMatrix(const std::string UniformName, int Size, GLuint UniformLocation);
  void setUniform(const std::string UniformName, const GLfloat* ValuePointer);
  void setUniform(const std::string UniformName, const GLfloat ValueOne, const GLfloat ValueTwo = 0.0, const GLfloat ValueThree = 0.0, const GLfloat ValueFour = 0.0);
  void setUniform(const std::string UniformName, const GLint ValueOne, const GLint ValueTwo = 0.0, const GLint ValueThree = 0.0, const GLint ValueFour = 0.0);
  std::string getLog();
  void cleanUp();
  ~uniformmanager();

 private:
  enum uniformtype {UNIFORM, UNIFORM_MATRIX};

  struct uniformstruct {
    std::string Name;
    uniformtype Type;
    int Size;
    GLuint Handle;
  };

  std::vector<uniformstruct>::iterator findUniform(const std::string UniformName);
  
  std::vector<uniformstruct> UniformArray;
  std::string LogString;
};

#endif
