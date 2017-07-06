#version 450 core

out vec2 TextureCoords;

const vec3 QuadVerts[6] = {
  {-1, 1, 0}, {1, 1, 0}, {-1, -1, 0}, {1, 1, 0}, {1, -1, 0}, {-1, -1, 0}
};

const vec2 Coords[6] = {
  {0, 1}, {1, 1}, {0, 0}, {1, 1}, {1, 0}, {0, 0}
};

void main() {
 gl_Position = vec4(QuadVerts[gl_VertexID], 1);
 TextureCoords = Coords[gl_VertexID];
}
