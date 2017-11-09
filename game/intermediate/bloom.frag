#version 450 core

uniform sampler2D Bloom;
uniform bool Horizontal;
uniform float Weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

out vec3 bBloom;
in vec2 TextureCoords;

void main() {
  vec2 TextureOffset = 1.0 / textureSize(Bloom, 0); // gets size of single texel
  vec3 Result = texture(Bloom, TextureCoords).rgb * Weight[0]; // current fragment's contribution
  if (Horizontal) {
    for (int i = 1; i < 5; ++i) {
        Result += texture(Bloom, TextureCoords + vec2(TextureOffset.x * i, 0.0)).rgb * Weight[i];
        Result += texture(Bloom, TextureCoords - vec2(TextureOffset.x * i, 0.0)).rgb * Weight[i];
    }
  } else {
    for(int i = 1; i < 5; ++i) {
        Result += texture(Bloom, TextureCoords + vec2(0.0, TextureOffset.y * i)).rgb * Weight[i];
        Result += texture(Bloom, TextureCoords - vec2(0.0, TextureOffset.y * i)).rgb * Weight[i];
    }
  }
  bBloom = Result;
}
