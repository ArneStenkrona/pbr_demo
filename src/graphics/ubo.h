#ifndef UBO_H
#define UBO_H

#include "src/config/config.h"
#include "src/graphics/geometry/light.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct ModelUBO {
    alignas(16) glm::mat4 model[NUMBER_SUPPORTED_MODEL_MATRICES];
    alignas(16) glm::mat4 invTransposeModel[NUMBER_SUPPORTED_MODEL_MATRICES];
    alignas(16) glm::mat4 viewProjection;
    alignas(16) glm::mat4 view;
    // alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 viewPosition;
    alignas(4)  float t = 0;
    // unsigned char pad12[12];
};

struct LightUBO {
    alignas(4)  float ambientLight;
    alignas(4)  uint32_t noPointLights;
    alignas(16) DirLight sun;
    alignas(16) glm::vec4 splitDepths[(NUMBER_SHADOWMAP_CASCADES + 4)/ 4];
    alignas(16) glm::mat4 cascadeSpace[NUMBER_SHADOWMAP_CASCADES];
    alignas(16) UBOPointLight pointLights[NUMBER_SUPPORTED_POINTLIGHTS];

};

struct BoneUBO {
    alignas(16) glm::mat4 bones[NUMBER_MAX_BONES];
};

struct StandardUBO {
    ModelUBO model;
    LightUBO lighting;
};

struct AnimatedStandardUBO {
    ModelUBO model;
    LightUBO lighting;
    BoneUBO bones;
};

struct StandardPushConstants {
    alignas(4)  int32_t   modelMatrixIdx;
    alignas(4)  int32_t   albedoIndex;
    alignas(4)  int32_t   metallicIndex;
    alignas(4)  int32_t   roughnessIndex;
    alignas(16) glm::vec4 albedo;
    alignas(4)  float     metallic;
    alignas(4)  float     roughness;
    alignas(4)  float     ao;
    alignas(4)  float     emissive;
    alignas(4)  int32_t   aoIndex;
    alignas(4)  int32_t   normalIndex;
    alignas(4)  uint32_t  boneOffset;
};

struct SkyboxUBO {
    alignas(16) glm::mat4 projection;
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 skyRotation;
    alignas(16) glm::vec4 sunDirection;
    alignas(16) glm::vec4 nightColor;
    alignas(16) glm::vec4 dayColor;
    alignas(16) glm::vec4 sunEdgeColor;
    alignas(16) glm::vec4 sunsetriseColor;
    alignas(16) glm::vec4 sunColor;
    alignas(4) float distToNoon;
};

struct ShadowMapUBO  {
    alignas(16) glm::mat4 model[NUMBER_SUPPORTED_MODEL_MATRICES];
    alignas(16) glm::mat4 depthVP[NUMBER_SHADOWMAP_CASCADES];
};

struct AnimatedShadowMapUBO {
    alignas(16) glm::mat4 model[NUMBER_SUPPORTED_MODEL_MATRICES];
    alignas(16) glm::mat4 depthVP[NUMBER_SHADOWMAP_CASCADES];
    alignas(16) BoneUBO bones;
};

#endif
