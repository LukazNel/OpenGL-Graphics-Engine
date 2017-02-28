#version 450 core

uniform mat4 CSMatrix;

struct block {
  ivec3 Coordinates;
  vec3 Colour;
  uint Level;
  ivec3 Rotation;
  ivec3 Offset;
};

layout(std140) buffer StorageBuffer {
  block BlockArray[1000000];
};

out vec3 Position;
out vec3 Colour;
out vec3 Normal;
flat out uint Level;

vec3 ExpandArray[36] = {
  {-5, 5, 5}, {5, 5, 5}, {-5, -5, 5}, {5, 5, 5}, {5, -5, 5}, {-5, -5, 5}, //Front
  {5, 5, 5}, {5, 5, -5}, {5, -5, 5}, {5, 5, -5}, {5, -5, -5}, {5, -5, 5}, //Right
  {5, 5, -5}, {-5, 5, -5}, {5, -5, -5}, {-5, 5, -5}, {-5, -5, -5}, {5, -5, -5}, //Back
  {-5, 5, -5}, {-5, 5, 5}, {-5, -5, -5}, {-5, 5, 5}, {-5, -5, 5}, {-5, -5, -5}, //Left
  {-5, 5, -5}, {5, 5, -5}, {-5, 5, 5}, {5, 5, -5}, {5, 5, 5}, {-5, 5, 5}, //Top
  {-5, -5, 5}, {5, -5, 5}, {-5, -5, -5}, {5, -5, 5}, {5, -5, -5}, {-5, -5, -5} //Bottom
};

vec3 NormalArray[6] = {
  {0, 0, 1}, //Front
  {1, 0, 0}, //Right
  {0, 0, -1}, //Back
  {-1, 0, 0}, //Left
  {0, 1, 0}, //Top
  {0, -1, 0} //Bottom
};

vec3 Positions = vec3(0, 0, 0);

void main() {
  int Index = gl_VertexID / 36;
  int Vertex = gl_VertexID % 36;
  Position = vec3(CSMatrix * vec4(vec3(BlockArray[Index].Coordinates) + ExpandArray[Vertex], 1));
//VertexPosition = vec3(Positions + ExpandArray[gl_VertexID]);
  gl_Position = vec4(Position, 1);
//gl_Position = vec4(ModelMatrix * vec4(VertexPosition, 1));
//gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  Colour = BlockArray[Index].Colour;
  Normal = normalize(transpose(inverse(mat3(CSMatrix))) * NormalArray[int(Vertex / 6)]);
  Level = BlockArray[Index].Level;
}
