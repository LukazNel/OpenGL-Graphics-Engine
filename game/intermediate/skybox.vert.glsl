#version 450 core

out vec3 TexCoord;

uniform mat4 SkyboxMatrix;

vec3 Vertices[] = {
    // Positions          
    {-1.0f,  1.0f, -1.0f},
    {-1.0f, -1.0f, -1.0f},
     {1.0f, -1.0f, -1.0f},
     {1.0f, -1.0f, -1.0f},
     {1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},

    {-1.0f, -1.0f,  1.0f},
    {-1.0f, -1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f,  1.0f},
    {-1.0f, -1.0f,  1.0f},

     {1.0f, -1.0f, -1.0f},
     {1.0f, -1.0f,  1.0f},
     {1.0f,  1.0f,  1.0f},
     {1.0f,  1.0f,  1.0f},
     {1.0f,  1.0f, -1.0f},
     {1.0f, -1.0f, -1.0f},

    {-1.0f, -1.0f,  1.0f},
    {-1.0f,  1.0f,  1.0f},
     {1.0f,  1.0f,  1.0f},
     {1.0f,  1.0f,  1.0f},
     {1.0f, -1.0f,  1.0f},
    {-1.0f, -1.0f,  1.0f},

    {-1.0f,  1.0f, -1.0f},
     {1.0f,  1.0f, -1.0f},
     {1.0f,  1.0f,  1.0f},
     {1.0f,  1.0f,  1.0f},
    {-1.0f,  1.0f,  1.0f},
    {-1.0f,  1.0f, -1.0f},

    {-1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f,  1.0f},
     {1.0f, -1.0f, -1.0f},
     {1.0f, -1.0f, -1.0f},
    {-1.0f, -1.0f,  1.0f},
     {1.0f, -1.0f,  1.0f}
};

void main() {
  gl_Position = vec4(mat3(SkyboxMatrix) * Vertices[gl_VertexID], 1).xyww;
  TexCoord = Vertices[gl_VertexID];
}
