#version 450 core

in vec3 Vertex;
out vec4 Colour;

uniform sampler2D Sky;
uniform sampler2D Glow;

void main() {
  vec3 SunSpherical = vec3(100, radians(90), radians(35)); //distance, degrees from +Y, degrees from -Z
  vec3 SunPosition = normalize(vec3(SunSpherical.x * sin(SunSpherical.z) * cos(SunSpherical.y), SunSpherical.x * sin(SunSpherical.z) * sin(SunSpherical.y), SunSpherical.x * cos(SunSpherical.z)));
  float Radius = 0.1;
  float Proximity = 0;
  if (SunPosition.y - Vertex.y < Radius)
    Proximity = dot(SunPosition, Vertex);
  float SkyVar = 0;
  if (SunPosition.y < 0)
    SkyVar = smoothstep(0.99, 0.01, -Vertex.y);
  else SkyVar = smoothstep(0.99, 0.01, -Vertex.y);
  float GlowVar = 1 - SkyVar;
  vec4 SkyColour = texture(Sky, vec2((SunPosition.y + 1) / 2, SkyVar));
  vec4 GlowColour = texture(Glow, vec2((SunPosition.y + 1) / 2, GlowVar));

  Colour = vec4((SkyColour.rgb + GlowColour.rgb * GlowColour.a) / 2, SkyColour.a);
}
