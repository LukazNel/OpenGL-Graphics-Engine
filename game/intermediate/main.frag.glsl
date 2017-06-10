#version 450 core

uniform int NumLights;
uniform vec3 CameraPosition;
uniform vec3 SunPosition;

#define MAX_LIGHTS 10
struct light {
   vec4 Position;
   vec3 Intensity; //a.k.a the color of the light
   float Attenuation;
   float AmbientCoefficient;
};

layout(std140) uniform LightUniform {
  light LightArray[MAX_LIGHTS];
};

in vec3 WSPosition;
in vec3 Colour;
in vec3 WSNormal;
flat in uint Level;

out vec4 FinalColour;

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
        Attenuation = 1.0 / (1.0 + Light.Attenuation * pow(DistanceToLight, 2));
    }

    //ambient
    vec3 Ambient = Light.AmbientCoefficient * Colour.rgb * Light.Intensity;

    //diffuse
    float DiffuseCoefficient = max(0.0, dot(Normal, SurfaceToLight));
    vec3 Diffuse = DiffuseCoefficient * Colour.rgb * Light.Intensity;
    
    //specular
    float SpecularCoefficient = 0.0;
    if(DiffuseCoefficient > 0.0 && Level == 2)
        SpecularCoefficient = pow(max(0.0, dot(Normal, HalfwayDirection)), 8);
    vec3 Specular = SpecularCoefficient * vec3(0.5, 0.5, 0.5) * Light.Intensity;

    //linear color (color before gamma correction)
    return Ambient + Attenuation*(Diffuse + Specular);
}

void main() {
    vec3 SurfaceToCamera = normalize(CameraPosition - WSPosition);

    //combine color from all the lights
    light Sun = {vec4(-SunPosition.x, SunPosition.y, SunPosition.z, 0), vec3(1), 0.2, 0.005};
    if (SunPosition.y < 0) {
      Sun.Position.y *= -1;
      Sun.Position.x *= -1;
      Sun.Intensity = vec3(0.7529, 0.7529, 0.96);
      Sun.AmbientCoefficient = 0.000005;
    }
    vec3 LinearColour = vec3(0);
    if (Level != 3) {
      for(int i = 0; i < NumLights; ++i) {
        LinearColour += applyLight(LightArray[i], WSNormal, WSPosition, SurfaceToCamera);
      }
      LinearColour += applyLight(Sun, WSNormal, WSPosition, SurfaceToCamera);
    }  else LinearColour = Colour;

    //final color (after gamma correction)
    vec3 Gamma = vec3(1.0 / 2.2);
    FinalColour = vec4(pow(LinearColour, Gamma), 1.0);
    
    //FinalColour = vec4(Colour, 1);
}
