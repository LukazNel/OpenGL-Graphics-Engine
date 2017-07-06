#version 450 core

uniform int NumLights;
uniform vec3 CameraPosition;
uniform vec3 SunPosition;

#define MAX_LIGHTS 10
struct light {
   vec4 Position;
   vec3 Colour; //a.k.a the color of the light
   float Intensity;
   float AmbientCoefficient;
};

layout(binding = 0) uniform atomic_uint LightCount;
layout(std430) buffer LightBuffer {
  light LightArray[MAX_LIGHTS];
};

uniform sampler2D ShadowDepth;

in vec3 SSPosition;
in vec3 WSPosition;
in vec3 Colour;
in vec3 WSNormal;
flat in uint Level;
flat in uint Intensity;

layout(location = 0) out vec4 FinalColour;
layout(location = 1) out vec4 BrightColour;

vec3 applyLight(light Light, vec3 Normal, vec3 Position, vec3 SurfaceToCamera) {
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
        SpecularCoefficient = pow(max(0.0, dot(Normal, HalfwayDirection)), uint(Intensity/10));
    vec3 Specular = SpecularCoefficient * vec3(0.5, 0.5, 0.5) * Light.Colour;

    //shadows
    float Shadow = 0;
    if (Light.Position.w == 0) {
      vec3 ShadowCoord = SSPosition * 0.5 + 0.5;
      float ClosestDepth = texture(ShadowDepth, ShadowCoord.xy).r;
      float CurrentDepth = ShadowCoord.z;
      if (CurrentDepth <= 1.0) {
        float Bias = max(0.05 - dot(Normal, LightDirection), 0.005);
        Shadow = CurrentDepth - Bias > ClosestDepth ? 1 : 0;
      }
    }

    //linear color (color before gamma correctioni)
    return Ambient + (1 - Shadow)*Attenuation*(Diffuse + Specular);
}

void main() {
    vec3 SurfaceToCamera = normalize(CameraPosition - WSPosition);

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
      LinearColour += applyLight(Sun, WSNormal, WSPosition, SurfaceToCamera);
      for(int i = 0; i < atomicCounter(LightCount); ++i) {
        LinearColour += applyLight(LightArray[i], WSNormal, WSPosition, SurfaceToCamera);
      }
      BrightColour = vec4(0, 0, 0, 1);
    }  else {
      LinearColour = Colour;
      BrightColour = vec4(Colour, 1);
    }

    //final color (after gamma correction)
    vec3 Gamma = vec3(1.0 / 2.2);
    FinalColour = vec4(pow(LinearColour, Gamma), 1.0);
    
    //FinalColour = vec4(Colour, 1);
}
