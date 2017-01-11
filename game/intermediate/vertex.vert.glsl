#version 420 core

uniform mat4 WorldMatrix;
uniform vec3 CameraPosition;

in vec3 Position;
layout (location = 1) in float Colour;
in float BlockLevel;

out vec3 CenterPoint;
flat out int VertexColour;
flat out int VertexBlockLevel;

void main() {
  gl_Position = vec4(Position, 1.0);

  vec4 PointPosition = WorldMatrix * vec4(Position, 1);
  CenterPoint = vec3(0);
  if (CameraPosition.x <= PointPosition.x)
    CenterPoint.x = -0.5;
  else CenterPoint.x = 0.5;
  if (CameraPosition.y <= PointPosition.y)
    CenterPoint.y = -0.5;
  else CenterPoint.y = 0.5;
  if (CameraPosition.z <= PointPosition.z)
    CenterPoint.z = 0.5;
  else CenterPoint.z = -0.5;
  
  VertexColour = int(Colour);
  VertexBlockLevel = int(BlockLevel);

}
