#version 450
#extension GL_ARB_separate_shader_objects : enable

struct DirLight {
    vec3 direction;
    vec3 color;
};

struct PointLight {
    vec3 pos;
    float a; // quadtratic term
    vec3 color;
    float b; // linear term
    float c; // constant term
};

layout(set = 0, binding = 0) uniform UniformBufferObject {
    /* Model */
    mat4 model[10];
    mat4 invTransposeModel[10];
    mat4 viewProjection;
    mat4 view;
    // mat4 proj;
    vec3 viewPos;
    float t;
    /* Lights */
    float ambientLight;
    uint noPointLights;
    DirLight sun;
    vec4 splitDepths[(5 + 4) / 4];
    mat4 cascadeSpace[5];
    PointLight pointLights[4];
} ubo;

layout(set = 0, binding = 1) uniform texture2D textures[64];
layout(set = 0, binding = 2) uniform sampler samp;

layout(set = 0, binding = 3) uniform sampler2DArray shadowMap;

layout(push_constant) uniform MATERIAL {
    layout(offset = 0) int   modelMatrixIdx;
    layout(offset = 4) int   albedoIndex;
    layout(offset = 8) int   metallicIndex;
    layout(offset = 12) int   roughnessIndex;
    layout(offset = 16) vec4  albedo;
    layout(offset = 32) float metallic;
    layout(offset = 36) float roughness;
    layout(offset = 40) float ao;
    layout(offset = 44) int   aoIndex;
    layout(offset = 48) int   normalIndex;
} material;

layout(location = 0) in VS_OUT {
    vec3 fragPos;
    vec2 fragTexCoord;
    vec3 fragNormal;
    vec3 shadowPos;
    vec3 tangentSunDir;
    vec3 tangentViewPos;
    vec3 tangentFragPos;
    mat3 invtbn;
} fs_in;

const mat4 biasMat = mat4(0.5, 0.0, 0.0, 0.0,
                          0.0, 0.5, 0.0, 0.0,
                          0.0, 0.0, 1.0, 0.0,
                          0.5, 0.5, 0.0, 1.0);

const float PI = 3.14159265359;

layout(location = 0) out vec4 accumColor;
layout(location = 1) out float revealColor;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a*a;
    float NdotH = max(dot(N,H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N,V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 CalcPointLight(int i, 
                    vec3 V, 
                    vec3 N, 
                    vec3 F0, 
                    vec3 albedo, 
                    float metallic,
                    float roughness) {
    vec3 L = normalize(transpose(fs_in.invtbn) * ubo.pointLights[i].pos - fs_in.tangentFragPos);
    vec3 H = normalize(V + L);

    float dist = length(ubo.pointLights[i].pos - fs_in.fragPos);
    float attenuation = 1.0 / (dist * dist);
    vec3 radiance = ubo.pointLights[i].color * attenuation;

    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N,L), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    vec3 contribution = (kD * albedo / PI + specular) * radiance * NdotL;

    return contribution;
}

void main() {
    vec3 albedo = material.albedoIndex < 0 ? material.albedo.rgb:
                    (texture(sampler2D(textures[material.albedoIndex], samp), fs_in.fragTexCoord).rgb) * material.albedo.rgb;

    float metallic = material.metallicIndex < 0 ? material.metallic :
                    (texture(sampler2D(textures[material.metallicIndex], samp), fs_in.fragTexCoord).r) * material.metallic;

    float roughness = material.roughnessIndex < 0 ? material.roughness :
                    (texture(sampler2D(textures[material.roughnessIndex], samp), fs_in.fragTexCoord).r) * material.roughness;
    float ao = material.aoIndex < 0 ? material.ao :
                    (texture(sampler2D(textures[material.aoIndex], samp), fs_in.fragTexCoord).r) * material.ao;
    vec3 N = material.normalIndex < 0 ? vec3(0,0,1) :
                    2.0 * texture(sampler2D(textures[material.normalIndex], samp), fs_in.fragTexCoord).rgb - 1.0;

    vec3 V = normalize(transpose(fs_in.invtbn) * ubo.viewPos - fs_in.fragPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < ubo.noPointLights; ++i) {
        Lo += CalcPointLight(i, V, N, F0, albedo, metallic, roughness);
    }
    vec3 ambient = vec3(ubo.ambientLight) * albedo * ao;
    vec3 color = ambient + Lo;
    // gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    vec4 outColor = vec4(color, material.albedo.a);

    float weight = 
        max(min(1.0, max( max(outColor.r, outColor.g), outColor.b ) * outColor.a ) , outColor.a) *
        clamp(0.03 / (1e-5 + pow(gl_FragCoord.z / 200, 4.0)), 1e-2, 3e3);

    // Blend Func: GL_ONE, GL_ONE
    // Switch to premultiplied alpha and weight
    accumColor = vec4(outColor.rgb * outColor.a, outColor.a) * weight;

    // Blend Func: GL_ZERO, GL_ONE_MINUS_SRC_ALPHA
    revealColor = outColor.a;
}