#version 450

out vec2 TextureCoords;

void main() 
{
	TextureCoords = vec2(gl_VertexID & 2, (gl_VertexID << 1) & 2);
	gl_Position = vec4(TextureCoords * 2.0f + -1.0f, 0.0f, 1.0f);
}
