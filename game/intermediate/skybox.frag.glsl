#version 450 core

in vec3 TexCoord;
out vec4 Colour;

uniform samplerCube Skybox;

void main() {
  Colour = texture(Skybox, TexCoord);
}
