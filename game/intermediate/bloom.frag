#version 450 core
out vec4 ColourOut;
  
in vec2 TextureCoords;

uniform sampler2D Texture;
  
uniform bool Horizontal;
uniform float Weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
  vec2 TextureOffset = 1.0 / textureSize(Texture, 0); // gets size of single texel
  vec3 Result = texture(Texture, TextureCoords).rgb * Weight[0]; // current fragment's contribution
  if (Horizontal) {
    for (int i = 1; i < 5; ++i) {
        Result += texture(Texture, TextureCoords + vec2(TextureOffset.x * i, 0.0)).rgb * Weight[i];
        Result += texture(Texture, TextureCoords - vec2(TextureOffset.x * i, 0.0)).rgb * Weight[i];
    }
  } else {
    for(int i = 1; i < 5; ++i) {
        Result += texture(Texture, TextureCoords + vec2(0.0, TextureOffset.y * i)).rgb * Weight[i];
        Result += texture(Texture, TextureCoords - vec2(0.0, TextureOffset.y * i)).rgb * Weight[i];
    }
  }
  ColourOut = vec4(Result, 1.0);
}
