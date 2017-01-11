#ifndef _MODULE_RENDERER_SHADER_MANAGER_H_
#define _MODULE_RENDERER_SHADER_MANAGER_H_

#include <vector>
#include <string>
#include <fstream>

#include "external/include/glad.h"

class shadermanager {
 public:
  shadermanager(void(*QuitCallbackPointer)(void*));
  GLuint createShader(const GLenum ShaderType, const std::string ShaderName);
  std::string getLog();
  void cleanUp();
  virtual ~shadermanager();

 private:
  struct shaderstruct {
    std::string ShaderName;
    GLuint ShaderHandle;
  };


  std::string parseShader(const std::string);
  bool logShader(const GLenum ShaderType, const GLuint& ShaderHandle);

  std::vector<shaderstruct> ShaderArray;
  std::string LogString;
  std::ifstream ShaderIn;
  void(*QuitCallback)(void*);
};

#endif // _MODULE_RENDERER_SHADER_MANAGER_H_
