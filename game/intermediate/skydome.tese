#version 450 core

layout(triangles, equal_spacing, cw) in;
in vec3 tcPosition[];
out vec3 pos;
out vec3 sun_norm;
out vec3 star_pos;
uniform mat4 SkydomeMatrix;
uniform mat4 StarMatrix;
uniform vec3 SunPosition;

void main() {
  vec3 p0 = gl_TessCoord.x * tcPosition[0];
  vec3 p1 = gl_TessCoord.y * tcPosition[1];
  vec3 p2 = gl_TessCoord.z * tcPosition[2];
  pos = normalize(p0 + p1 + p2);
  gl_Position = vec4(mat3(SkydomeMatrix) * pos, 1).xyww;
  star_pos = vec3(StarMatrix * vec4(pos, 1));
  sun_norm = normalize(SunPosition);
}
