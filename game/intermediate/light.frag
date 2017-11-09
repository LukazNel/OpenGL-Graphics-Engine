#version 450 core

uniform vec3 CameraPosition;
uniform vec3 SunPosition;

#define MAX_LIGHTS 10
struct light {
   vec4 Position;
   vec3 Colour;
   float Intensity;
   float AmbientCoefficient;
};

layout(binding = 0) uniform atomic_uint LightCount;
layout(std430) buffer LightBuffer {
  light LightArray[MAX_LIGHTS];
};

layout(std140) uniform ColourUniform {
  vec3 ColourArray[1022];
};

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColour;

layout(location = 0) out vec3 lColour;
layout(location = 1) out vec3 lBloom;

in vec2 TextureCoords;

vec3 applyLight(light Light, vec3 Normal, vec3 Position, vec3 SurfaceToCamera, vec3 Colour, uint Level);

void main() {
    vec3 Position = texture(gPosition, TextureCoords).rgb;
    vec3 Normal = texture(gNormal, TextureCoords).rgb;
    vec3 SurfaceToCamera = normalize(CameraPosition - Position);
    vec3 Colour = texture(gColour, TextureCoords).rgb;
    uint Level = uint(texture(gColour, TextureCoords).a);

    //combine color from all the lights
    light Sun = {vec4(SunPosition.x, SunPosition.y, -SunPosition.z, 0), vec3(1), 0.2, 0.005};
    if (SunPosition.y < 0) {
      Sun.Position.y *= -1;
      Sun.Position.x *= -1;
      Sun.Colour = vec3(0.7529, 0.7529, 0.96);
      Sun.AmbientCoefficient = 0.005;
    }
    vec3 LinearColour = vec3(0);
    if (Level != 1) {
      LinearColour += applyLight(Sun, Normal, Position, SurfaceToCamera, Colour, Level);
      for(int i = 0; i < atomicCounter(LightCount); ++i) {
        LinearColour += applyLight(LightArray[i], Normal, Position, SurfaceToCamera, Colour, Level);
      }
      lBloom = vec3(0);
    }  else {
      LinearColour = Colour;
      lBloom = Colour;
    }

    lColour = LinearColour;
}

vec3 applyLight(light Light, vec3 Normal, vec3 Position, vec3 SurfaceToCamera, vec3 Colour, uint Level) {
    vec3 LightDirection = normalize(Light.Position.xyz - Position);
    vec3 HalfwayDirection = normalize(LightDirection + SurfaceToCamera);

    vec3 SurfaceToLight;
    float Attenuation = 1.0;
    if(Light.Position.w == 0.0) {
        //directional light
        SurfaceToLight = normalize(Light.Position.xyz);
        Attenuation = 1.0; //no attenuation for directional lights
    } else {
        //point light
        SurfaceToLight = normalize(Light.Position.xyz - Position);
        float DistanceToLight = length(Light.Position.xyz - Position);
        Attenuation = 1.0 / (1.0 + Light.Intensity * pow(DistanceToLight, 2));
    }

    //ambient
    vec3 Ambient = Light.AmbientCoefficient * Colour.rgb * Light.Colour;

    //diffuse
    float DiffuseCoefficient = max(0.0, dot(Normal, SurfaceToLight));
    vec3 Diffuse = DiffuseCoefficient * Colour.rgb * Light.Colour;
    
    //specular
    float SpecularCoefficient = 0.0;
    if(DiffuseCoefficient > 0.0 && Level == 2)
        SpecularCoefficient = pow(max(0.0, dot(Normal, HalfwayDirection)), 2);//uint(Intensity/10));
    vec3 Specular = SpecularCoefficient * vec3(0.5, 0.5, 0.5) * Light.Colour;

    //shadows
    float Shadow = 0;
    /*if (Light.Position.w == 0) {
      vec3 ShadowCoord = SSPosition * 0.5 + 0.5;
      float ClosestDepth = texture(Depth, vec3(TextureCoords, 1));
      float CurrentDepth = ShadowCoord.z;
      if (CurrentDepth <= 1.0) {
        float Bias = max(0.05 - dot(Normal, LightDirection), 0.005);
        Shadow = CurrentDepth - Bias > ClosestDepth ? 1 : 0;
      }
    }*/

    return Ambient + (1 - Shadow)*Attenuation*(Diffuse + Specular);
}
