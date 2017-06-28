#ifndef _MODULE_RENDERER_UNIFORM_MANAGER_H_
#define _MODULE_RENDERER_UNIFORM_MANAGER_H_

#include <string>
#include <vector>
#include <algorithm>

#include "external/include/glad.h"
#include "external/include/stb_image.h"

class uniformmanager {
 public:
  uniformmanager();

  void createUniform(const std::string UniformName, int Size, GLuint UniformLocation);
  void createUniformMatrix(const std::string UniformName, int Size, GLuint UniformLocation);
  void setUniform(const std::string UniformName, const GLfloat* ValuePointer);
  void setUniform(const std::string UniformName, const GLfloat ValueOne, const GLfloat ValueTwo = 0.0, const GLfloat ValueThree = 0.0, const GLfloat ValueFour = 0.0);
  void setUniform(const std::string UniformName, const GLint ValueOne, const GLint ValueTwo = 0.0, const GLint ValueThree = 0.0, const GLint ValueFour = 0.0);

  template<typename ...args>
    void createTextures(GLenum Type, int Amount, const std::string TextureName, args... Arguments) {
      GLuint Textures[Amount];
      glCreateTextures(Type, Amount, Textures);
      createTextureHelper(Textures, Amount, TextureName, Arguments...);
    }

  void setTexture(std::string TextureName, std::string ImageName, GLuint UniformLocation);
  void setBlankTexture(std::string TextureName, GLenum InternalFormat, int Width, int Height, GLint UniformLocation);
  void setImageTexture(std::string TextureName, GLuint UniformLocation, GLenum InternalFormat, int Width, int Height);
  void setMultisampleTexture(std::string TextureName, int Samples, GLenum InternalFormat, int Width, int Height, bool FixedSamples);
  void setDefaultParameters(const std::string TextureName, int MinFilter, int MagFilter, int WrapS, int WrapT);
  void setTextureParameter(const std::string TextureName, GLenum Parameter, int Value);
  void setTextureParameter(const std::string TextureName, GLenum Parameter, float Value);
  void setTextureParameter(const std::string TextureName, GLenum Parameter, int* Values);
  void setTextureParameter(const std::string TextureName, GLenum Parameter, float* Values);
  GLuint getTexture(std::string TextureName);

  std::string getLog();
  void cleanUp();
  ~uniformmanager();

 private:
  enum uniformtype {UNIFORM, UNIFORM_MATRIX};

  struct uniform {
    std::string Name;
    uniformtype Type;
    int Size;
    GLuint Handle;
  };

  struct texture {
    std::string Name;
    GLuint Handle;
  };

  template<typename ...args>
    void createTextureHelper(GLuint* Textures, int& Index, const std::string TextureName, args... Arguments) {
      //Index--;
      createTextureHelper(Textures, Index, TextureName);
      createTextureHelper(Textures, Index, Arguments...);
    }
  void createTextureHelper(GLuint* Textures, int& Index, const std::string TextureName);

  std::vector<uniform>::iterator findUniform(const std::string UniformName);
  std::vector<texture>::iterator findTexture(const std::string TextureName);
  
  std::vector<uniform> UniformArray;
  std::vector<texture> TextureArray;
  int TextureCount;
  int ImageCount;
  std::string LogString;
};

#endif
