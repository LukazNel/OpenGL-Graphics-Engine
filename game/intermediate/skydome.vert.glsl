#version 450 core

out vec3 vPosition;

uniform mat4 SkydomeMatrix;

vec3 Vertices[] = {
    // Positions          
    {0.0f,  1.0f, 0.0f},
    {1.0f, 0.0f, -1.0f},
    {-1.0f, 0.0f, -1.0f},

    {0.0f, 1.0f, 0.0f},
    {-1.0f,  0.0f, -1.0f},
    {-1.0f,  0.0f, 1.0f},

    {0.0f, 1.0f,  0.0f},
    {-1.0f, 0.0f, 1.0f},
    {1.0f,  0.0f, 1.0f},

    {0.0f,  1.0f, 0.0f},
    {1.0f,  0.0f, 1.0f},
    {1.0f, 0.0f,  -1.0f}
};

void main() {
  gl_Position = vec4(Vertices[gl_VertexID], 1);
  vPosition = Vertices[gl_VertexID];
}
