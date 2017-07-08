#version 450 core

uniform sampler2D Texture;
uniform sampler2D Bloom;

in vec2 TextureCoords;
out vec4 ColourOut;

void main() {
    vec3 hdrColor = texture(Texture, TextureCoords).rgb;
    vec3 bloomColor = texture(Bloom, TextureCoords).rgb;
    hdrColor += bloomColor;
    // reinhard tone mapping
    //vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    float exposure = 2.5;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
  
    vec3 Gamma = vec3(1.0 / 2.2);
    //ColourOut = vec4(pow(mapped, Gamma), 1.0);
    ColourOut = vec4(mapped, 1.0);
//    ColourOut = texture(Bloom, TextureCoords);
}
