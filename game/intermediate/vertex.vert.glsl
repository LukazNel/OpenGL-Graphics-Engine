#version 450 core

uniform mat4 CSMatrix;
uniform mat4 WSMatrix;

struct block {
  ivec3 Coordinates;
  vec3 Colour;
  uint Level;
  ivec3 Rotation;
  ivec3 Offset;
};

layout(std430) buffer StorageBuffer {
  block BlockArray[]; // 1000000
} BufferIn;

out vec4 Position;
out vec3 Colour;
out vec3 Normal;
flat out uint Level;

vec3 ExpandArray[36] = {
  {-0.5, 0.5, 0.5}, {0.5, 0.5, 0.5}, {-0.5, -0.5, 0.5}, {0.5, 0.5, 0.5}, {0.5, -0.5, 0.5}, {-0.5, -0.5, 0.5}, //Front
  {0.5, 0.5, 0.5}, {0.5, 0.5, -0.5}, {0.5, -0.5, 0.5}, {0.5, 0.5, -0.5}, {0.5, -0.5, -0.5}, {0.5, -0.5, 0.5}, //Right
  {0.5, 0.5, -0.5}, {-0.5, 0.5, -0.5}, {0.5, -0.5, -0.5}, {-0.5, 0.5, -0.5}, {-0.5, -0.5, -0.5}, {0.5, -0.5, -0.5}, //Back
  {-0.5, 0.5, -0.5}, {-0.5, 0.5, 0.5}, {-0.5, -0.5, -0.5}, {-0.5, 0.5, 0.5}, {-0.5, -0.5, 0.5}, {-0.5, -0.5, -0.5}, //Left
  {-0.5, 0.5, -0.5}, {0.5, 0.5, -0.5}, {-0.5, 0.5, 0.5}, {0.5, 0.5, -0.5}, {0.5, 0.5, 0.5}, {-0.5, 0.5, 0.5}, //Top
  {-0.5, -0.5, 0.5}, {0.5, -0.5, 0.5}, {-0.5, -0.5, -0.5}, {0.5, -0.5, 0.5}, {0.5, -0.5, -0.5}, {-0.5, -0.5, -0.5} //Bottom
};

vec3 NormalArray[6] = {
  {0, 0, 1}, //Front
  {1, 0, 0}, //Right
  {0, 0, -1}, //Back
  {-1, 0, 0}, //Left
  {0, 1, 0}, //Top
  {0, -1, 0} //Bottom
};

vec3 Positions = vec3(0, -2, 2);

void main() {
  int Index = gl_VertexID / 36;
  int Vertex = gl_VertexID % 36;
  Position = WSMatrix * vec4(vec3(BufferIn.BlockArray[Index].Coordinates) + ExpandArray[Vertex], 1);
//Position = vec3(WSMatrix * vec4(Positions + ExpandArray[Vertex], 1));
  gl_Position = CSMatrix * Position;
//gl_Position = vec4(ModelMatrix * vec4(VertexPosition, 1));
//gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  Colour = BufferIn.BlockArray[Index].Colour;
  Normal = normalize(transpose(inverse(mat3(WSMatrix))) * NormalArray[int(Vertex / 6)]);
  Level = BufferIn.BlockArray[Index].Level;
}
