#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
	vec3 fragPos;
	vec3 TexCoords;
} fs_in;

layout(location = 0) out vec4 outColor;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;
	mat4 skyRotation;
	vec4 sunDirection;
    vec4 nightColor;
    vec4 dayColor;
    vec4 sunEdgeColor;
    vec4 sunsetriseColor;
    vec4 sunColor;
    float distToNoon;
} ubo;

layout(set = 0, binding = 1) uniform samplerCube samplerCubeMap;

#define PI 3.14159265359

void main()
{    
    vec4 texColor = texture(samplerCubeMap, fs_in.fragPos);
    outColor = vec4(texColor.rgb, 1.0);
}
