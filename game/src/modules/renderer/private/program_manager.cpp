#include "classes/program_manager.h"

programmanager::programmanager(void(*QuitCallbackPointer)(void*)) {
  Quit = QuitCallbackPointer;
  LogString += '\n';
}

void programmanager::createShader(const std::string ShaderName, const GLenum ShaderType) {
  GLuint ShaderHandle = glCreateShader(ShaderType);
  std::string ParsedShaderString = parseShader(ShaderName);
  const GLchar* ParsedShaderChar = ParsedShaderString.c_str();
  if (*ParsedShaderChar == ' ')
    return;
  int Length = ParsedShaderString.length()/sizeof(GLchar);
  glShaderSource(ShaderHandle, 1, &ParsedShaderChar, &Length);
  glCompileShader(ShaderHandle);
  LogString += ShaderName + ": ";
  logShader(ShaderHandle, ShaderType);
  shader ShaderVar;
  ShaderVar.Name = ShaderName;
  ShaderVar.Handle = ShaderHandle;
  ShaderArray.push_back(ShaderVar);
}

void programmanager::createProgram(const std::string ProgramName) {
  program ProgramVar;
  ProgramVar.Name = ProgramName;
  ProgramVar.Handle = glCreateProgram();
  ProgramArray.push_back(ProgramVar);
  LogString += "Program '" + ProgramName + "' successfully created.\n";
}

void programmanager::addShader(const std::string ProgramName, const std::string ShaderName) {
  auto ProgramIterator = findProgram(ProgramName);
  auto ShaderIterator = findShader(ShaderName);
  if (ProgramIterator != ProgramArray.end() && ShaderIterator != ShaderArray.end()) {
    glAttachShader(ProgramIterator->Handle, ShaderIterator->Handle);
    LogString += "Shader '" + ShaderName + "' added to program '" + ProgramName + "'.\n";
  }
}

void programmanager::linkProgram(const std::string ProgramName) {
  auto ProgramIterator = findProgram(ProgramName);
  if (ProgramIterator != ProgramArray.end()) {
    glLinkProgram(ProgramIterator->Handle);
    LogString += ProgramName + ": ";
    logProgram(ProgramIterator->Handle);
  }
}

void programmanager::installProgram(const std::string ProgramName) {
  auto ProgramIterator = findProgram(ProgramName);
  if (ProgramIterator != ProgramArray.end()) {
    glUseProgram(ProgramIterator->Handle);
    LogString += "Program '" + ProgramName + "' bound to context.\n";
  }
}

GLint programmanager::getResourceIndex(const std::string ProgramName, GLenum ResourceType, const std::string ResourceName) {
  auto ProgramIterator = findProgram(ProgramName);
  if (ProgramIterator != ProgramArray.end()) {
    GLint Index;
      Index = glGetProgramResourceIndex(ProgramIterator->Handle, ResourceType, ResourceName.c_str());
    if (Index == GL_INVALID_INDEX || Index == -1)
      LogString += "Uniform '" + ResourceName + "' not found in program '" + ProgramName + "' with index " + std::to_string(Index) + ".\n";
    else LogString += "Uniform '" + ResourceName + "' found in program '" + ProgramName + "' with index " + std::to_string(Index) + ".\n";
    return Index;
  }
}

GLint programmanager::getResourceLocation(const std::string ProgramName, GLenum ResourceType, const std::string ResourceName) {
  auto ProgramIterator = findProgram(ProgramName);
  if (ProgramIterator != ProgramArray.end()) {
    GLint Location = glGetProgramResourceLocation(ProgramIterator->Handle, ResourceType, ResourceName.c_str());
    if (Location == GL_INVALID_INDEX || Location == -1)
      LogString += "Resource '" + ResourceName + "' not found in program '" + ProgramName + "' with location " + std::to_string(Location) + ".\n";
    else LogString += "Resource '" + ResourceName + "' found in program '" + ProgramName + "' with location " + std::to_string(Location) + ".\n";
    return Location;
  }
}

void programmanager::setBinding(const std::string ProgramName, GLenum ResourceType, const std::string ResourceName, int BindingPoint) {
  auto ProgramIterator = findProgram(ProgramName);
  if (ProgramIterator != ProgramArray.end()) {
    glUseProgram(ProgramIterator->Handle);
    if (ResourceType == GL_UNIFORM || ResourceType == GL_UNIFORM_BLOCK)
      glUniformBlockBinding(ProgramIterator->Handle, glGetProgramResourceIndex(ProgramIterator->Handle, ResourceType, ResourceName.c_str()), BindingPoint);
    else if (ResourceType == GL_SHADER_STORAGE_BLOCK)
      glShaderStorageBlockBinding(ProgramIterator->Handle, glGetProgramResourceIndex(ProgramIterator->Handle, ResourceType, ResourceName.c_str()), BindingPoint);
    else LogString += "Warning: Cannot bind resource '" + ResourceName + "': incompatible type.";
      LogString += "Resource " + ResourceName + " in program '" + ProgramName + "' bound to binding point " + std::to_string(BindingPoint) + ".\n";
      glUseProgram(0);
  }
}

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

auto programmanager::findProgram(const std::string ProgramName) -> std::vector<program>::iterator {
  auto ProgramIterator = std::find_if(ProgramArray.begin(), ProgramArray.end(), 
      [&] (const program& ProgramVar) {return ProgramVar.Name == ProgramName;});
  if (ProgramIterator == ProgramArray.end())
    LogString = "Warning: Program '" + ProgramName + "' not found.\n";
  return ProgramIterator;
}

auto programmanager::findShader(const std::string ShaderName) -> std::vector<shader>::iterator {
  auto ShaderIterator = std::find_if(ShaderArray.begin(), ShaderArray.end(), 
      [&] (const shader& ShaderVar) {return ShaderVar.Name == ShaderName;});
  if (ShaderIterator == ShaderArray.end())
    LogString = "Warning: Shader '" + ShaderName + "' not found.\n";
  return ShaderIterator;
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
    //Quit(nullptr);
  }
}
