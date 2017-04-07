#version 450 core

layout(triangles, equal_spacing, cw) in;
in vec3 tcPosition[];
out vec3 Vertex;
uniform mat4 SkydomeMatrix; 

void main() {
  vec3 p0 = gl_TessCoord.x * tcPosition[0];
  vec3 p1 = gl_TessCoord.y * tcPosition[1];
  vec3 p2 = gl_TessCoord.z * tcPosition[2];
  Vertex = normalize(p0 + p1 + p2);
  gl_Position = vec4(mat3(SkydomeMatrix) * Vertex, 1).xyww;
}
