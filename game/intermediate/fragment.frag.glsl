#version 450 core

uniform int NumLights;

#define MAX_LIGHTS 10
uniform struct light {
   vec4 Position;
   vec3 Intensity; //a.k.a the color of the light
   float Attenuation;
   float AmbientCoefficient;
} Light;

layout(std140) uniform LightUniform {
  light LightArray[MAX_LIGHTS];
};

in vec3 Position;
in vec3 Colour;
in vec3 Normal;
flat in uint Level;

out vec4 FinalColour;

vec3 applyLight(light Light, vec3 Normal, vec3 Position, vec3 SurfaceToCamera) {
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
        SpecularCoefficient = pow(max(0.0, dot(SurfaceToCamera, reflect(-SurfaceToLight, Normal))), 2);
    vec3 Specular = SpecularCoefficient * vec3(1.0, 1.0, 1.0) * Light.Intensity;

    //linear color (color before gamma correction)
    return Ambient + Attenuation*(Diffuse + Specular);
}

void main() {
    vec3 SurfaceToCamera = normalize(Position);
    //vec3 FragmentNormal = normalize(transpose(inverse(mat3(WSMatrix))) * VertexNormal);
    //vec3 FragmentPosition = vec3(WSMatrix * vec4(VertexPosition, 0));
    //combine color from all the lights
    //light Current = LightArray[0];
    //Current.Position = Light.Position;
    vec3 LinearColour = vec3(0);
    if (Level != 3)
      for(int i = 0; i < NumLights; ++i) {
        LinearColour += applyLight(LightArray[i], Normal, Position, SurfaceToCamera);
      }
        //LinearColour += applyLight(LightArray[i], VertexNormal, FragmentPosition, SurfaceToCamera);

    //final color (after gamma correction)
    vec3 Gamma = vec3(1.0/2.2);
    FinalColour = vec4(pow(LinearColour, Gamma), 1.0);
    //FinalColour = vec4(LinearColour, 1);
    
    //FinalColour = vec4(Colour, 1);
}
