#include "classes/uniform_manager.h"

uniformmanager::uniformmanager() {
  LogString += '\n';
}

void uniformmanager::createUniform(const std::string UniformName, int Size, GLuint UniformLocation) {
  uniform UniformVar;
  UniformVar.Name = UniformName;
  UniformVar.Type = UNIFORM;
  UniformVar.Size = Size;
  UniformVar.Handle = UniformLocation;
  UniformArray.push_back(UniformVar);
  LogString += "Uniform '" + UniformName + "' successfully created.\n";
}

void uniformmanager::createUniformMatrix(const std::string UniformName, int Size, GLuint UniformLocation) {
  uniform UniformVar;
  UniformVar.Name = UniformName;
  UniformVar.Type = UNIFORM_MATRIX;
  UniformVar.Size = Size;
  UniformVar.Handle = UniformLocation;
  UniformArray.push_back(UniformVar);
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

void uniformmanager::setTexture(std::string TextureName, std::string ImageName, GLuint UniformLocation) {
  auto TextureIterator = findTexture(TextureName);
  if (TextureIterator != TextureArray.end()) {
    int Width, Height, Channels;
    TextureIterator->Unit = ++TextureCount;
    unsigned char* Image = stbi_load(ImageName.c_str(), &Width, &Height, &Channels, 0);
    if (Image != nullptr) {
      LogString += "Loading Image '" + ImageName + "': Width " + std::to_string(Width) + ", Height " + std::to_string(Height) + " with " + std::to_string(Channels) + " channels.\n";
      glBindTextureUnit(TextureIterator->Unit, TextureIterator->Handle);
      if (Channels == 4) {
        glTextureStorage2D(TextureIterator->Handle, 1, GL_RGBA8, Width, Height);
        glTextureSubImage2D(TextureIterator->Handle, 0, 0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, Image);
      } else if (Channels == 3) {
        glTextureStorage2D(TextureIterator->Handle, 1, GL_RGB8, Width, Height);
        glTextureSubImage2D(TextureIterator->Handle, 0, 0, 0, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, Image);
      } else {
        LogString += "Warning: Cannot load image '" + ImageName + "' into texture '" + TextureName + "': Invalid channels.\n";
        return;
      }
      LogString += "Image '" + ImageName + "' loaded into texture '" + TextureName + "'.\n";
      glUniform1i(UniformLocation, TextureIterator->Unit);

    } else LogString += "Warning: Cannot load image '" + ImageName + "' into texture '" + TextureName + "': File not found.\n";
    stbi_image_free(Image);
  }
}

void uniformmanager::setBlankTexture(std::string TextureName, GLenum InternalFormat, int Width, int Height, GLint UniformLocation) {
  auto TextureIterator = findTexture(TextureName);
  if (TextureIterator != TextureArray.end()) {
    TextureIterator->Unit = ++TextureCount;
    glBindTextureUnit(TextureIterator->Unit, TextureIterator->Handle);
    glTextureStorage2D(TextureIterator->Handle, 1, InternalFormat, Width, Height);
    if (UniformLocation != -1)
      glUniform1i(UniformLocation, TextureIterator->Unit);
    LogString += "Blank texture '" + TextureName + "' set.\n";
  }
}

void uniformmanager::setImageTexture(std::string TextureName, GLuint UniformLocation, GLenum InternalFormat, int Width, int Height) {
  auto TextureIterator = findTexture(TextureName);
  if (TextureIterator != TextureArray.end()) {
    TextureIterator->Unit = ++ImageCount;
    glBindImageTexture(TextureIterator->Unit, TextureIterator->Handle, 0, GL_FALSE, 0, GL_READ_WRITE, InternalFormat);
    glTextureStorage2D(TextureIterator->Handle, 1, InternalFormat, Width, Height);
    glUniform1i(UniformLocation, TextureIterator->Unit);
    LogString += "Image texture '" + TextureName + "' set.\n";
  }
}

void uniformmanager::setMultisampleTexture(std::string TextureName, int Samples, GLenum InternalFormat, int Width, int Height, bool FixedSamples) {
  auto TextureIterator = findTexture(TextureName);
  if (TextureIterator != TextureArray.end()) {
    TextureIterator->Unit = ++TextureCount;
    glBindTextureUnit(TextureIterator->Unit, TextureIterator->Handle);
    glTextureStorage2DMultisample(TextureIterator->Handle, Samples, InternalFormat, Width, Height, FixedSamples);
    LogString += "Multisample texture '" + TextureName + "' set.\n";
  }
}

void uniformmanager::setDefaultParameters(const std::string TextureName, int MinFilter, int MagFilter, int WrapS, int WrapT) {
  auto TextureIterator = findTexture(TextureName);
  if (TextureIterator != TextureArray.end()) {
    glTextureParameteri(TextureIterator->Handle, GL_TEXTURE_MIN_FILTER, MinFilter);
    glTextureParameteri(TextureIterator->Handle, GL_TEXTURE_MAG_FILTER, MagFilter);
    glTextureParameteri(TextureIterator->Handle, GL_TEXTURE_WRAP_S, WrapS); 
    glTextureParameteri(TextureIterator->Handle, GL_TEXTURE_WRAP_T, WrapT); 
  }
}

void uniformmanager::setTextureParameter(const std::string TextureName, GLenum Parameter, int Value) {
  auto TextureIterator = findTexture(TextureName);
  if (TextureIterator != TextureArray.end()) {
    glTextureParameteri(TextureIterator->Handle, Parameter, Value);
  }
}

void uniformmanager::setTextureParameter(const std::string TextureName, GLenum Parameter, float Value) {
  auto TextureIterator = findTexture(TextureName);
  if (TextureIterator != TextureArray.end()) {
    glTextureParameterf(TextureIterator->Handle, Parameter, Value);
  }
}

void uniformmanager::setTextureParameter(const std::string TextureName, GLenum Parameter, int* Values) {
  auto TextureIterator = findTexture(TextureName);
  if (TextureIterator != TextureArray.end()) {
    glTextureParameteriv(TextureIterator->Handle, Parameter, Values);
  }
}

void uniformmanager::setTextureParameter(const std::string TextureName, GLenum Parameter, float* Values) {
  auto TextureIterator = findTexture(TextureName);
  if (TextureIterator != TextureArray.end()) {
    glTextureParameterfv(TextureIterator->Handle, Parameter, Values);
  }
}

void uniformmanager::setTextureUnit(std::string TextureName, GLuint TextureUnit) {
  auto TextureIterator = findTexture(TextureName);
  if (TextureIterator != TextureArray.end()) {
    TextureIterator->Unit = TextureUnit;
    glBindTextureUnit(TextureIterator->Unit, TextureIterator->Handle);
  }
}

GLuint uniformmanager::getTexture(std::string TextureName) {
  auto TextureIterator = findTexture(TextureName);
  if (TextureIterator != TextureArray.end()) {
    return TextureIterator->Handle;
  }
}

GLuint uniformmanager::getTextureUnit(std::string TextureName) {
  auto TextureIterator = findTexture(TextureName);
  if (TextureIterator != TextureArray.end()) {
    return TextureIterator->Unit;
  }
}

std::string uniformmanager::getLog() {
  std::string Log = LogString;
  LogString = '\n';
  return Log;
}

void uniformmanager::cleanUp() {
  for (auto TextureIterator : TextureArray)
    glDeleteTextures(1, &TextureIterator.Handle);
}

uniformmanager::~uniformmanager() {
}

void uniformmanager::createTextureHelper(GLuint* Textures, int& Index, const std::string TextureName) {
  Index--;
  texture TextureVar;
  TextureVar.Name = TextureName;
  TextureVar.Handle = Textures[Index];
  TextureArray.push_back(TextureVar);
  LogString += "Texture '" + TextureName + "' created.\n";
}

auto uniformmanager::findUniform(const std::string UniformName) -> std::vector<uniform>::iterator{
  auto UniformIterator = std::find_if(UniformArray.begin(), UniformArray.end(), 
      [&] (const uniform& UniformVar) {return UniformVar.Name == UniformName;});
  if (UniformIterator == UniformArray.end())
    LogString = "Warning: uniform '" + UniformName + "' not found.\n";
  return UniformIterator;
}

auto uniformmanager::findTexture(const std::string TextureName) -> std::vector<texture>::iterator {
  auto TextureIterator = std::find_if(TextureArray.begin(), TextureArray.end(), 
      [&] (const texture& TextureVar) {return TextureVar.Name == TextureName;});
  if (TextureIterator == TextureArray.end())
    LogString = "Warning: Texture '" + TextureName + "' not found.\n";
  return TextureIterator;
}
