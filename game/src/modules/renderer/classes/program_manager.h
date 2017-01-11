#ifndef _MODULE_RENDERER_PROGRAM_MANAGER_H_
#define _MODULE_RENDERER_PROGRAM_MANAGER_H_

#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

#include "external/include/glad.h"

enum locationindex {INDEX, LOCATION};

class programmanager {
 public:
  void createShader(const std::string ShaderName, const GLenum ShaderType);
  programmanager(void(*QuitCallbackPointer)(void*));
  template<typename ...args>
    void createProgram(const std::string ProgramName, args... Arguments) {
      createProgram(ProgramName);
      createProgram(Arguments...);
    }
  void createProgram(const std::string ProgramName);
  template<typename ...args>
    void addShader(const std::string ProgramName, const std::string ShaderName, args... Arguments) {
      addShader(ProgramName, ShaderName);
      addShader(ProgramName, Arguments...);
    }
  void addShader(const std::string ProgramName, const std::string ShaderName);
  void linkProgram(const std::string ProgramName);
  void installProgram(const std::string ProgramName);
  GLint getResource(const std::string ProgramName, locationindex LocationIndex, GLenum ResourceType, const std::string ResourceName);
  void setUniformBinding(const std::string ProgramName, locationindex LocationIndex, GLenum ResourceType, const std::string ResourceName, int BindingPoint);
  std::string getLog();
  void cleanUp();
  ~programmanager();

 private:
  struct resource {
    std::string Name;
    GLuint Handle;
  };
  enum resourcetype {SHADER, PROGRAM};

  std::vector<resource>::iterator findResource(const std::string ResourceName, resourcetype ResourceType);
  std::string parseShader(const std::string FileName);
  void logShader(const GLuint& ShaderHandle, const GLenum ShaderType);
  void logProgram(const GLuint& ProgramHandle);

  std::vector<resource> ProgramArray;
  std::vector<resource> ShaderArray;
  std::string LogString;
  std::ifstream ShaderIn;
  void(*Quit)(void*);
};

#endif // _MODULE_RENDERER_PROGRAM_MANAGER_H_
