#include "classes/shader_manager.h"

shadermanager::shadermanager(void(*QuitCallbackPointer)(void*)) {
  QuitCallback = QuitCallbackPointer;
  LogString += '\n';
}

GLuint shadermanager::createShader(GLenum Type, const std::string ShaderName) {
  GLuint ShaderVar = glCreateShader(Type);
  std::string ParsedShaderString = parseShader(ShaderName);
  const GLchar* ParsedShaderChar = ParsedShaderString.c_str();
  if (*ParsedShaderChar == ' ')
    return 0;
  int Length = ParsedShaderString.length()/sizeof(GLchar);
  glShaderSource(ShaderVar, 1, &ParsedShaderChar, &Length);
  glCompileShader(ShaderVar);
  LogString += ShaderName + ": ";
  if (!logShader(Type, ShaderVar))
    QuitCallback(nullptr);
  shaderstruct ShaderStructVar;
  ShaderStructVar.ShaderName = ShaderName;
  ShaderStructVar.ShaderHandle = ShaderVar;
  ShaderArray.push_back(ShaderStructVar);
  return ShaderVar;
}

std::string shadermanager::getLog() {
  std::string Log = LogString;
  LogString = '\n';
  return Log;
}

void shadermanager::cleanUp() {
  for (auto ShaderIterator : ShaderArray) {
    glDeleteShader(ShaderIterator.ShaderHandle);
  }
}

shadermanager::~shadermanager() {
}

std::string shadermanager::parseShader(const std::string ShaderName) {
  ShaderIn.open(ShaderName);
  std::string ShaderString;
  if (ShaderIn.good()) {
    getline(ShaderIn, ShaderString, '\0');
  } else {
    LogString = "Warning: File '" + ShaderName + "' not found.\n";
    ShaderString = ' ';
  }
  ShaderIn.close();
  return ShaderString;
}

bool shadermanager::logShader(const GLenum ShaderType, const GLuint& ShaderHandle) {
  GLint LogSize;
  glGetShaderiv(ShaderHandle, GL_INFO_LOG_LENGTH, &LogSize);
  if (LogSize == 1) {
    LogString += "Compilation Successful.\n";
    return true;
  } else {
    GLchar LogChar[LogSize];
    glGetShaderInfoLog(ShaderHandle, LogSize, nullptr, LogChar);
    LogString += '\n' + LogChar + '\n';
    return false;
  }
}
