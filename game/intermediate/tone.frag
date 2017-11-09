#version 450 core

uniform sampler2D lColour;
uniform sampler2D bBloom;
uniform float Exposure = 2.5;

out vec4 ColourOut;
in vec2 TextureCoords;

void main() {
    vec3 HDRColour = texture(lColour, TextureCoords).rgb;
    vec3 BloomColour = texture(bBloom, TextureCoords).rgb;
    HDRColour += BloomColour;
    vec3 LDRColour = vec3(1.0) - exp(-HDRColour * Exposure);
    ColourOut = vec4(sqrt(LDRColour), 1.0);
}
