#version 450 core

uniform mat4 CSMatrix;
uniform mat4 WSMatrix;

struct block {
  ivec3 Coordinates;
  vec3 Colour;
  uint Level;
  vec4 Quaternion;
  vec3 Offset;
};

layout(std430) buffer StorageBuffer {
  block BlockArray[]; // 1000000
} BufferIn;

out vec3 WSPosition;
out vec3 Colour;
out vec3 WSNormal;
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

void main() {
  int Index = gl_VertexID / 36;
  int VertexID = gl_VertexID  % 36;
  block Block = BufferIn.BlockArray[Index];
  vec3 Vertex = ExpandArray[VertexID];
  vec3 RotatedPosition = Vertex + (2 * cross(Block.Quaternion.xyz, (cross(Block.Quaternion.xyz, Vertex) + Block.Quaternion.w * Vertex)));
  vec3 TranslatedPosition = vec3(Block.Coordinates) + RotatedPosition;
  WSPosition = vec3(WSMatrix * vec4(TranslatedPosition, 1));
//Position = vec3(WSMatrix * vec4(Positions + ExpandArray[Vertex], 1));
  gl_Position = CSMatrix * vec4(WSPosition, 1);
//gl_Position = vec4(ModelMatrix * vec4(VertexPosition, 1));
//gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  Colour = Block.Colour;
  vec3 Normal = NormalArray[int(VertexID / 6)];
  WSNormal = normalize(transpose(inverse(mat3(WSMatrix))) * (Normal + (2 * cross(Block.Quaternion.xyz, (cross(Block.Quaternion.xyz, Normal) + Block.Quaternion.w * Normal)))));
  Level = Block.Level;
}
