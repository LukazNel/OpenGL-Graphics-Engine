#ifndef _MODULE_RENDERER_PROGRAM_MANAGER_H_
#define _MODULE_RENDERER_PROGRAM_MANAGER_H_

#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

#include "external/include/glad.h"

class programmanager {
 public:
  programmanager(void(*QuitCallbackPointer)(void*));
  void createShader(const std::string ShaderName, const GLenum ShaderType);

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
  GLint getResourceIndex(const std::string ProgramName, GLenum ResourceType, const std::string ResourceName);
  GLint getResourceLocation(const std::string ProgramName, GLenum ResourceType, const std::string ResourceName);
  void setBinding(const std::string ProgramName, GLenum ResourceType, const std::string ResourceName, int BindingPoint);

  std::string getLog();
  void cleanUp();
  ~programmanager();

 private:
  struct program {
    std::string Name;
    GLuint Handle;
  };
  struct shader {
    std::string Name;
    GLuint Handle;
  };

  std::vector<program>::iterator findProgram(const std::string ProgramName);
  std::vector<shader>::iterator findShader(const std::string ShaderName);
  std::string parseShader(const std::string FileName);
  void logShader(const GLuint& ShaderHandle, const GLenum ShaderType);
  void logProgram(const GLuint& ProgramHandle);

  std::vector<program> ProgramArray;
  std::vector<shader> ShaderArray;
  std::string LogString;
  std::ifstream ShaderIn;
  void(*Quit)(void*);
};

#endif // _MODULE_RENDERER_PROGRAM_MANAGER_H_
