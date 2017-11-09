#version 450 core

layout (triangles) in;
layout (triangle_strip, max_vertices=27) out;

struct block_data {
  vec3 WSPosition;
  vec3 WSNormal;
  vec3 CSPosition;
  vec3 SSPosition;
  uint Colour;
  uint Level;
};

flat in block_data VertexData[3];
flat out block_data FragmentData;

void main()
{
    for(int face = 0; face < 9; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            FragmentData = VertexData[i];
            switch (gl_Layer) {
              case 0 : gl_Position = vec4(FragmentData.CSPosition, 1);
                       break;
              case 1 : gl_Position = vec4(FragmentData.SSPosition, 1);
                       break;
              default : gl_Position = vec4(FragmentData.CSPosition, 1);
            }
            EmitVertex();
        }    
        EndPrimitive();
    }
}  
