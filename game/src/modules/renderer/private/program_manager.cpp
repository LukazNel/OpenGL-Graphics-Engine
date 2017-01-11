#include "classes/program_manager.h"

programmanager::programmanager(void(*QuitCallbackPointer)(void*)) {
  Quit = QuitCallbackPointer;
  LogString += '\n';
}

void programmanager::createShader(const std::string ShaderName, const GLenum ShaderType) {
  GLuint ShaderVar = glCreateShader(ShaderType);
  std::string ParsedShaderString = parseShader(ShaderName);
  const GLchar* ParsedShaderChar = ParsedShaderString.c_str();
  if (*ParsedShaderChar == ' ')
    return;
  int Length = ParsedShaderString.length()/sizeof(GLchar);
  glShaderSource(ShaderVar, 1, &ParsedShaderChar, &Length);
  glCompileShader(ShaderVar);
  LogString += ShaderName + ": ";
  logShader(ShaderVar, ShaderType);
  resource ResourceVar;
  ResourceVar.Name = ShaderName;
  ResourceVar.Handle = ShaderVar;
  ShaderArray.push_back(ResourceVar);
}

void programmanager::createProgram(const std::string ProgramName) {
  resource ResourceVar;
  ResourceVar.Name = ProgramName;
  ResourceVar.Handle = glCreateProgram();
  ProgramArray.push_back(ResourceVar);
  LogString += "Program '" + ProgramName + "' successfully created.\n";
}

void programmanager::addShader(const std::string ProgramName, const std::string ShaderName) {
  auto ProgramIterator = findResource(ProgramName, PROGRAM);
  auto ShaderIterator = findResource(ShaderName, SHADER);
  if (ProgramIterator != ProgramArray.end() && ShaderIterator != ShaderArray.end()) {
    glAttachShader(ProgramIterator->Handle, ShaderIterator->Handle);
    LogString += "Shader '" + ShaderName + "' added to program '" + ProgramName + "'.\n";
  }
}

void programmanager::linkProgram(const std::string ProgramName) {
  auto ProgramIterator = findResource(ProgramName, PROGRAM);
  if (ProgramIterator != ProgramArray.end()) {
    glLinkProgram(ProgramIterator->Handle);
    LogString += ProgramName + ": ";
    logProgram(ProgramIterator->Handle);
  }
}

void programmanager::installProgram(const std::string ProgramName) {
  auto ProgramIterator = findResource(ProgramName, PROGRAM);
  if (ProgramIterator != ProgramArray.end()) {
    glUseProgram(ProgramIterator->Handle);
    LogString += "Program '" + ProgramName + "' bound to context.\n";
  }
}

GLint programmanager::getResource(const std::string ProgramName, locationindex LocationIndex, GLenum UniformType, const std::string UniformName) {
  auto ProgramIterator = findResource(ProgramName, PROGRAM);
  if (ProgramIterator != ProgramArray.end()) {
    GLint Index;
    switch (LocationIndex) {
      case INDEX: Index = glGetProgramResourceIndex(ProgramIterator->Handle, UniformType, UniformName.c_str());
      break;
      case LOCATION: Index = glGetProgramResourceLocation(ProgramIterator->Handle, UniformType, UniformName.c_str());
      break;
    }
    if (Index == GL_INVALID_INDEX || Index == -1)
      LogString += "Uniform '" + UniformName + "' not found in program '" + ProgramName + "'.\n";
    else LogString += "Uniform '" + UniformName + "' found in program '" + ProgramName + "'.\n";
    return Index;
  }
}

void programmanager::setUniformBinding(const std::string ProgramName, locationindex LocationIndex, GLenum UniformType, const std::string UniformName, int BindingPoint) {
  auto ProgramIterator = findResource(ProgramName, PROGRAM);
  if (ProgramIterator != ProgramArray.end()) {
    switch (LocationIndex) {
      case INDEX: glUniformBlockBinding(ProgramIterator->Handle, glGetProgramResourceIndex(ProgramIterator->Handle, UniformType, UniformName.c_str()), BindingPoint);
      break;
      case LOCATION: glUniformBlockBinding(ProgramIterator->Handle, glGetProgramResourceLocation(ProgramIterator->Handle, UniformType, UniformName.c_str()), BindingPoint);
      break;
    }
    LogString += "Uniform block index " + UniformName + " in program '" + ProgramName + "' bound to binding point " + std::to_string(BindingPoint) + ".\n";
  }
}

/*void programmanager::setUniformBinding(const std::string ProgramName, GLuint UniformIndex, GLuint BindingPoint) {
  bool ProgramFound;
  auto ProgramIterator = findResource(ProgramName, PROGRAM);
  if (ProgramIterator != ProgramArray.end()) {
    glUniformBlockBinding(ProgramIterator->Handle, UniformIndex, BindingPoint);
    LogString += "Uniform block index " + std::to_string(UniformIndex) + " in program '" + ProgramName + "' bound to binding point " + std::to_string(BindingPoint) + ".\n";
  }
}*/

std::string programmanager::getLog() {
  std::string Log = LogString;
  LogString = '\n';
  return Log;
}

void programmanager::cleanUp() {
  for (auto ShaderIterator : ShaderArray) {
    glDeleteShader(ShaderIterator.Handle);
  }
  for (auto ProgramIterator : ProgramArray) {
    glDeleteProgram(ProgramIterator.Handle);
  }
}

programmanager::~programmanager() {
}

auto programmanager::findResource(const std::string ResourceName, resourcetype ResourceType) -> std::vector<resource>::iterator {
  std::vector<resource>* ResourceArray;
  switch (ResourceType) {
    case SHADER: ResourceArray = &ShaderArray;
      break;
    case PROGRAM: ResourceArray = &ProgramArray;
      break;
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

std::string programmanager::parseShader(const std::string ShaderName) {
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

void programmanager::logShader(const GLuint& ShaderHandle, const GLenum ShaderType) {
  GLint LogSize;
  glGetShaderiv(ShaderHandle, GL_INFO_LOG_LENGTH, &LogSize);
  if (LogSize == 0) {
    LogString += "Compilation Successful.\n";
    return;
  } else {
    GLchar LogChar[LogSize];
    glGetShaderInfoLog(ShaderHandle, LogSize, nullptr, LogChar);
    LogString += '\n' + LogChar + '\n';
  }
}

void programmanager::logProgram(const GLuint& ProgramHandle) {
  GLint LogSize;
  glGetProgramiv(ProgramHandle, GL_INFO_LOG_LENGTH, &LogSize);
  if (LogSize == 0) {
    LogString += "Linking Successful.\n";
    return;
  } else {
    GLchar LogChar[LogSize];
    glGetProgramInfoLog(ProgramHandle, LogSize, nullptr, LogChar);
    LogString += '\n' + LogChar + '\n';
    Quit(nullptr);
  }
}
