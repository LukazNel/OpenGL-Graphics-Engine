#include "classes/uniform_manager.h"

uniformmanager::uniformmanager() {
  LogString += '\n';
}

void uniformmanager::createUniform(const std::string UniformName, int Size, GLuint UniformLocation) {
  uniformstruct UniformStructVar;
  UniformStructVar.Name = UniformName;
  UniformStructVar.Type = UNIFORM;
  UniformStructVar.Size = Size;
  UniformStructVar.Handle = UniformLocation;
  UniformArray.push_back(UniformStructVar);
  LogString += "Uniform '" + UniformName + "' successfully created.\n";
}

void uniformmanager::createUniformMatrix(const std::string UniformName, int Size, GLuint UniformLocation) {
  uniformstruct UniformStructVar;
  UniformStructVar.Name = UniformName;
  UniformStructVar.Type = UNIFORM_MATRIX;
  UniformStructVar.Size = Size;
  UniformStructVar.Handle = UniformLocation;
  UniformArray.push_back(UniformStructVar);
  LogString += "Uniform matrix '" + UniformName + "' successfully created.\n";
}

void uniformmanager::setUniform(const std::string UniformName, const GLfloat* ValuePointer) {
  auto UniformIterator = findUniform(UniformName);
  if (UniformIterator != UniformArray.end() && UniformIterator->Type == UNIFORM) {
    switch (UniformIterator->Size) {
      case 1: glUniform1fv(UniformIterator->Handle, 1,  ValuePointer);
      break;
      case 2: glUniform2fv(UniformIterator->Handle, 1,  ValuePointer);
      break;
      case 3: glUniform3fv(UniformIterator->Handle, 1,  ValuePointer);
      break;
      case 4: glUniform4fv(UniformIterator->Handle, 1,  ValuePointer);
      break;
    }
    LogString += "Uniform '" + UniformName + "' set.\n";
  }
  else if (UniformIterator != UniformArray.end() && UniformIterator->Type == UNIFORM_MATRIX) {
    switch (UniformIterator->Size) {
      case 2: glUniformMatrix2fv(UniformIterator->Handle, 1, GL_FALSE, ValuePointer);
      break;
      case 3: glUniformMatrix3fv(UniformIterator->Handle, 1, GL_FALSE, ValuePointer);
      break;
      case 4: glUniformMatrix4fv(UniformIterator->Handle, 1, GL_FALSE, ValuePointer);
      break;
    }
    LogString += "Uniform matrix '" + UniformName + "' set.\n";
  }
}

void uniformmanager::setUniform(const std::string UniformName, const GLfloat ValueOne, const GLfloat ValueTwo, const GLfloat ValueThree, const GLfloat ValueFour) {
  auto UniformIterator = findUniform(UniformName);
  if (UniformIterator != UniformArray.end() && UniformIterator->Type == UNIFORM) {
    switch (UniformIterator->Size) {
      case 1: glUniform1f(UniformIterator->Handle, ValueOne);
      break;
      case 2: glUniform2f(UniformIterator->Handle, ValueOne, ValueTwo);
      break;
      case 3: glUniform3f(UniformIterator->Handle, ValueOne, ValueTwo, ValueThree);
      break;
      case 4: glUniform4f(UniformIterator->Handle, ValueOne, ValueTwo, ValueThree, ValueFour);
      break;
    }
    LogString += "Uniform '" + UniformName + "' set.\n";
  }
}

void uniformmanager::setUniform(const std::string UniformName, const GLint ValueOne, const GLint ValueTwo, const GLint ValueThree, const GLint ValueFour) {
  auto UniformIterator = findUniform(UniformName);
  if (UniformIterator != UniformArray.end() && UniformIterator->Type == UNIFORM) {
    switch (UniformIterator->Size) {
      case 1: glUniform1i(UniformIterator->Handle, ValueOne);
      break;
      case 2: glUniform2i(UniformIterator->Handle, ValueOne, ValueTwo);
      break;
      case 3: glUniform3i(UniformIterator->Handle, ValueOne, ValueTwo, ValueThree);
      break;
      case 4: glUniform4i(UniformIterator->Handle, ValueOne, ValueTwo, ValueThree, ValueFour);
      break;
    }
    LogString += "Uniform '" + UniformName + "' set.\n";
  }
}

std::string uniformmanager::getLog() {
  std::string Log = LogString;
  LogString = '\n';
  return Log;
}

void uniformmanager::cleanUp() {
  // No cleanup needed.
}

uniformmanager::~uniformmanager() {
}

auto uniformmanager::findUniform(const std::string UniformName) -> std::vector<uniformstruct>::iterator{
  auto UniformIterator = std::find_if(UniformArray.begin(), UniformArray.end(), 
      [&] (const uniformstruct& UniformStructVar) {return UniformStructVar.Name == UniformName;});
  if (UniformIterator == UniformArray.end())
    LogString = "Warning: uniform '" + UniformName + "' not found.\n";
  return UniformIterator;
}
