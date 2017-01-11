#include "classes/uniform_manager.h"

uniformmanager::uniformmanager() {
  LogString += '\n';
}

void uniformmanager::halt(bool Halt) {
  HaltBool = Halt;
  if (Halt)
    LogString += "UniformManager set to halt.\n";
}

void uniformmanager::createUniform(const std::string UniformName, const uniformtype UniformType, int Size, GLuint UniformLocation) {
  uniformstruct UniformStructVar;
  UniformStructVar.UniformName = UniformName;
  UniformStructVar.UniformType = UniformType;
  UniformStructVar.UniformSize = Size;
  UniformStructVar.UniformHandle = UniformLocation;
  UniformArray.push_back(UniformStructVar);
  LogString += "Uniform '" + UniformName + "' successfully created.\n";
}

void uniformmanager::setUniform(const std::string UniformName, const GLfloat* ValuePointer) {
  if (!HaltBool) {
    bool UniformFound;
    auto UniformIterator = findUniform(UniformName, UniformFound);
    if (UniformFound && UniformIterator->UniformType == UNIFORM) {
      switch (UniformIterator->UniformSize) {
        case 1: glUniform1fv(UniformIterator->UniformHandle, 1,  ValuePointer);
        break;
        case 2: glUniform2fv(UniformIterator->UniformHandle, 1,  ValuePointer);
        break;
        case 3: glUniform3fv(UniformIterator->UniformHandle, 1,  ValuePointer);
        break;
        case 4: glUniform4fv(UniformIterator->UniformHandle, 1,  ValuePointer);
        break;
      }
      LogString += "Uniform '" + UniformName + "' set.\n";
    }
    else if (UniformFound && UniformIterator->UniformType == UNIFORM_MATRIX) {
      switch (UniformIterator->UniformSize) {
        case 2: glUniformMatrix2fv(UniformIterator->UniformHandle, 1, GL_FALSE, ValuePointer);
        break;
        case 3: glUniformMatrix3fv(UniformIterator->UniformHandle, 1, GL_FALSE, ValuePointer);
        break;
        case 4: glUniformMatrix4fv(UniformIterator->UniformHandle, 1, GL_FALSE, ValuePointer);
        break;
      }
      LogString += "Uniform matrix '" + UniformName + "' set.\n";
    }
  }
}

void uniformmanager::setUniform(const std::string UniformName, const GLfloat ValueOne, const GLfloat ValueTwo, const GLfloat ValueThree, const GLfloat ValueFour) {
  if (!HaltBool) {
    bool UniformFound;
    auto UniformIterator = findUniform(UniformName, UniformFound);
    if (UniformFound && UniformIterator->UniformType == UNIFORM) {
      switch (UniformIterator->UniformSize) {
        case 1: glUniform1f(UniformIterator->UniformHandle, ValueOne);
        break;
        case 2: glUniform2f(UniformIterator->UniformHandle, ValueOne, ValueTwo);
        break;
        case 3: glUniform3f(UniformIterator->UniformHandle, ValueOne, ValueTwo, ValueThree);
        break;
        case 4: glUniform4f(UniformIterator->UniformHandle, ValueOne, ValueTwo, ValueThree, ValueFour);
        break;
      }
      LogString += "Uniform '" + UniformName + "' set.\n";
  }
  }
}

void uniformmanager::setUniform(const std::string UniformName, const GLint ValueOne, const GLint ValueTwo, const GLint ValueThree, const GLint ValueFour) {
  if (!HaltBool) {
    bool UniformFound;
    auto UniformIterator = findUniform(UniformName, UniformFound);
    if (UniformFound && UniformIterator->UniformType == UNIFORM) {
      switch (UniformIterator->UniformSize) {
        case 1: glUniform1i(UniformIterator->UniformHandle, ValueOne);
        break;
        case 2: glUniform2i(UniformIterator->UniformHandle, ValueOne, ValueTwo);
        break;
        case 3: glUniform3i(UniformIterator->UniformHandle, ValueOne, ValueTwo, ValueThree);
        break;
        case 4: glUniform4i(UniformIterator->UniformHandle, ValueOne, ValueTwo, ValueThree, ValueFour);
        break;
      }
      LogString += "Uniform '" + UniformName + "' set.\n";
  }
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

auto uniformmanager::findUniform(const std::string UniformName, bool& UniformFound) -> std::vector<uniformstruct>::iterator{
  auto UniformIterator = std::find_if(UniformArray.begin(), UniformArray.end(), 
      [&] (const uniformstruct& UniformStructVar) {return UniformStructVar.UniformName == UniformName;});
  if (UniformIterator != UniformArray.end()) {
    UniformFound = true;
    return UniformIterator;
  } else {
    UniformFound = false;
    LogString = "Warning: uniform '" + UniformName + "' not found.\n";
    return UniformIterator;
  }
}
