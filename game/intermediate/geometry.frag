#version 450 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gColour;

in vec3 WSPosition;
in vec3 WSNormal;
in vec3 Colour;
flat in uint Level;

void main() {
  gPosition = WSPosition;
  gNormal = WSNormal;
  gColour = vec4(Colour, Level);
}
