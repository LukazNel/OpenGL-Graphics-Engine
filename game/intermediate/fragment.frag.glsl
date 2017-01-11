#version 420 core

uniform mat4 WorldMatrix;
uniform vec3 CameraPosition;
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

in vec4 VertexPosition;
in vec3 FragmentColour;
in vec3 VertexNormal;
flat in int FragmentBlockLevel;

out vec4 FinalColour;

vec3 applyLight(light Light, vec3 FragmentNormal, vec3 FragmentPosition, vec3 SurfaceToCamera) {
    vec3 SurfaceToLight;
    float Attenuation = 1.0;
    if(Light.Position.w == 0.0) {
        //directional light
        SurfaceToLight = normalize(Light.Position.xyz);
        Attenuation = 1.0; //no attenuation for directional lights
    } else {
        //point light
        SurfaceToLight = normalize(Light.Position.xyz - FragmentPosition);
        float DistanceToLight = length(Light.Position.xyz - FragmentPosition);
        Attenuation = 1.0 / (1.0 + Light.Attenuation * pow(DistanceToLight, 2));
    }

    //ambient
    vec3 Ambient = Light.AmbientCoefficient * FragmentColour.rgb * Light.Intensity;

    //diffuse
    float DiffuseCoefficient = max(0.0, dot(FragmentNormal, SurfaceToLight));
    vec3 Diffuse = DiffuseCoefficient * FragmentColour.rgb * Light.Intensity;
    
    //specular
    float SpecularCoefficient = 0.0;
    if(DiffuseCoefficient > 0.0 && FragmentBlockLevel == 2)
        SpecularCoefficient = pow(max(0.0, dot(SurfaceToCamera, reflect(-SurfaceToLight, FragmentNormal))), 2);
    vec3 Specular = SpecularCoefficient * vec3(1.0, 1.0, 1.0) * Light.Intensity;

    //linear color (color before gamma correction)
    return Ambient + Attenuation*(Diffuse + Specular);
}

void main() {
    vec3 FragmentNormal = normalize(transpose(inverse(mat3(WorldMatrix))) * VertexNormal);
    vec3 FragmentPosition = vec3(WorldMatrix * VertexPosition);
    vec3 SurfaceToCamera = normalize(CameraPosition - FragmentPosition);

    //combine color from all the lights
    //light Current = LightArray[0];
    //Current.Position = Light.Position;
    vec3 LinearColour = vec3(0);
    if (FragmentBlockLevel != 3)
      for(int i = 0; i < NumLights; ++i) {
        LinearColour += applyLight(LightArray[0], FragmentNormal, FragmentPosition, SurfaceToCamera);
      }
    else LinearColour = FragmentColour.xyz * 2;

    //final color (after gamma correction)
    vec3 Gamma = vec3(1.0/2.2);
    FinalColour = vec4(pow(LinearColour, Gamma), 0.5);
    //FinalColour = vec4(LinearColour, 1);
    
    //FinalColour = vec4(1); //VertexColour;
}
