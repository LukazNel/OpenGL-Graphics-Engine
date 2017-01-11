#ifndef _MODULE_RENDERER_UNIFORM_MANAGER_H_
#define _MODULE_RENDERER_UNIFORM_MANAGER_H_

#include <string>
#include <vector>
#include <algorithm>

#include "external/include/glad.h"

class uniformmanager {
 public:
  enum uniformtype {UNIFORM, UNIFORM_MATRIX};

  uniformmanager();
  void halt(bool Halt);
  void createUniform(const std::string UniformName, uniformtype UniformType, int Size, GLuint UniformLocation);
  void setUniform(const std::string UniformName, const GLfloat* ValuePointer);
  void setUniform(const std::string UniformName, const GLfloat ValueOne, const GLfloat ValueTwo = 0.0, const GLfloat ValueThree = 0.0, const GLfloat ValueFour = 0.0);
  void setUniform(const std::string UniformName, const GLint ValueOne, const GLint ValueTwo = 0.0, const GLint ValueThree = 0.0, const GLint ValueFour = 0.0);
  std::string getLog();
  void cleanUp();
  ~uniformmanager();

 private:
  struct uniformstruct {
    std::string UniformName;
    uniformtype UniformType;
    int UniformSize;
    GLuint UniformHandle;
  };

  std::vector<uniformstruct>::iterator findUniform(const std::string UniformName, bool& UniformFound);
  
  std::vector<uniformstruct> UniformArray;
  std::string LogString;
  bool HaltBool;
};

#endif
