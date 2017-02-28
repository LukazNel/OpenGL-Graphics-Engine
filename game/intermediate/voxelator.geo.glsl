#version 450 core

layout(points) in;
layout(triangle_strip, max_vertices = 12) out;

uniform mat4 ModelMatrix;

layout(std140) uniform ColourUniform {
  vec3 ColourArray[1022]; // Change to vec3 later!
};

in vec3 CenterPoint[];
flat in int VertexColour[];
flat in int VertexBlockLevel[];

out vec3 VertexPosition;
out vec3 FragmentColour;
out vec3 VertexNormal;
flat out int FragmentBlockLevel;

void main(void) {
  vec3 VoxelColour = ColourArray[VertexColour[0]];
  VertexPosition = vec3(gl_in[0].gl_Position.x - CenterPoint[0].x, gl_in[0].gl_Position.y + CenterPoint[0].y, gl_in[0].gl_Position.z - CenterPoint[0].z);
  gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  FragmentColour = VoxelColour;
  VertexNormal = vec3(0.0, 0.0, 1.0 * sign(CenterPoint[0].z));
  FragmentBlockLevel = VertexBlockLevel[0];
  EmitVertex();
  VertexPosition = vec3(gl_in[0].gl_Position.x + CenterPoint[0].x, gl_in[0].gl_Position.y + CenterPoint[0].y, gl_in[0].gl_Position.z - CenterPoint[0].z);
  gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  FragmentColour = VoxelColour;
  VertexNormal = vec3(0.0, 0.0, 1.0 * sign(CenterPoint[0].z));
  FragmentBlockLevel = VertexBlockLevel[0];
  EmitVertex();
  VertexPosition = vec3(gl_in[0].gl_Position.x - CenterPoint[0].x, gl_in[0].gl_Position.y - CenterPoint[0].y, gl_in[0].gl_Position.z - CenterPoint[0].z);
  gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  FragmentColour = VoxelColour;
  VertexNormal = vec3(0.0, 0.0, 1.0 * sign(CenterPoint[0].z));
  FragmentBlockLevel = VertexBlockLevel[0];
  EmitVertex();
  VertexPosition = vec3(gl_in[0].gl_Position.x + CenterPoint[0].x, gl_in[0].gl_Position.y - CenterPoint[0].y, gl_in[0].gl_Position.z - CenterPoint[0].z);
  gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  FragmentColour = VoxelColour;
  VertexNormal = vec3(0.0, 0.0, 1.0 * sign(CenterPoint[0].z));
  FragmentBlockLevel = VertexBlockLevel[0];
  EmitVertex();
  EndPrimitive();
  VertexPosition = vec3(gl_in[0].gl_Position.x + CenterPoint[0].x, gl_in[0].gl_Position.y + CenterPoint[0].y, gl_in[0].gl_Position.z - CenterPoint[0].z);
  gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  FragmentColour = VoxelColour;
  VertexNormal = vec3(1.0 * sign(CenterPoint[0].x), 0.0, 0.0);
  FragmentBlockLevel = VertexBlockLevel[0];
  EmitVertex();
  VertexPosition = vec3(gl_in[0].gl_Position.x + CenterPoint[0].x, gl_in[0].gl_Position.y + CenterPoint[0].y, gl_in[0].gl_Position.z + CenterPoint[0].z);
  gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  FragmentColour = VoxelColour;
  VertexNormal = vec3(1.0 * sign(CenterPoint[0].x), 0.0, 0.0);
  FragmentBlockLevel = VertexBlockLevel[0];
  EmitVertex();
  VertexPosition = vec3(gl_in[0].gl_Position.x + CenterPoint[0].x, gl_in[0].gl_Position.y - CenterPoint[0].y, gl_in[0].gl_Position.z - CenterPoint[0].z);
  gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  FragmentColour = VoxelColour;
  VertexNormal = vec3(1.0 * sign(CenterPoint[0].x), 0.0, 0.0);
  FragmentBlockLevel = VertexBlockLevel[0];
  EmitVertex();
  VertexPosition = vec3(gl_in[0].gl_Position.x + CenterPoint[0].x, gl_in[0].gl_Position.y - CenterPoint[0].y, gl_in[0].gl_Position.z + CenterPoint[0].z);
  gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  FragmentColour = VoxelColour;
  VertexNormal = vec3(1.0 * sign(CenterPoint[0].x), 0.0, 0.0);
  FragmentBlockLevel = VertexBlockLevel[0];
  EmitVertex();
  EndPrimitive();
  VertexPosition = vec3(gl_in[0].gl_Position.x + CenterPoint[0].x, gl_in[0].gl_Position.y + CenterPoint[0].y, gl_in[0].gl_Position.z - CenterPoint[0].z);
  gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  FragmentColour = VoxelColour;
  VertexNormal = vec3(0.0, 1.0 * sign(CenterPoint[0].y), 0.0);
  FragmentBlockLevel = VertexBlockLevel[0];
  EmitVertex();
  VertexPosition = vec3(gl_in[0].gl_Position.x + CenterPoint[0].x, gl_in[0].gl_Position.y + CenterPoint[0].y, gl_in[0].gl_Position.z + CenterPoint[0].z);
  gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  FragmentColour = VoxelColour;
  VertexNormal = vec3(0.0, 1.0 * sign(CenterPoint[0].y), 0.0);
  FragmentBlockLevel = VertexBlockLevel[0];
  EmitVertex();
  VertexPosition = vec3(gl_in[0].gl_Position.x - CenterPoint[0].x, gl_in[0].gl_Position.y + CenterPoint[0].y, gl_in[0].gl_Position.z - CenterPoint[0].z);
  gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  FragmentColour = VoxelColour;
  VertexNormal = vec3(0.0, 1.0 * sign(CenterPoint[0].y), 0.0);
  FragmentBlockLevel = VertexBlockLevel[0];
  EmitVertex();
  VertexPosition = vec3(gl_in[0].gl_Position.x - CenterPoint[0].x, gl_in[0].gl_Position.y + CenterPoint[0].y, gl_in[0].gl_Position.z + CenterPoint[0].z);
  gl_Position = ModelMatrix * vec4(VertexPosition, 1);
  FragmentColour = VoxelColour;
  VertexNormal = vec3(0.0, 1.0 * CenterPoint[0].y, 0.0);
  FragmentBlockLevel = VertexBlockLevel[0];
  EmitVertex();
  EndPrimitive;
}

