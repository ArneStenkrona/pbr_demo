#ifndef GAME_RENDERER_H
#define GAME_RENDERER_H

#include "vulkan_application.h"
#include "ubo.h"

#include "src/graphics/geometry/light.h"
#include "src/graphics/camera.h"
#include "src/graphics/imgui_renderer.h"
#include "src/graphics/geometry/model.h"
#include "src/container/hash_map.h"

class Renderer : public VulkanApplication {
public:
    /**
     * @param width frame width
     * @param heght frame height
     */
    Renderer(unsigned int width, unsigned int height);

    ~Renderer();
    
    /**
     * binds a scene to the graphics pipeline
     */
    void bindAssets(Model const * models, size_t nModels,
                    ModelID const * staticModelIDs,
                    size_t nStaticModelIDs,
                    ModelID const * animatedModelIDs,
                    uint32_t const * boneOffsets,
                    size_t nAnimatedModelIDs,
                    Texture const * textures,
                    size_t nTextures,
                    prt::array<Texture, 6> const & skybox);

    /**
     * updates the scene
     * @param modelMatrices : model matrices
     * @param camera : scene camera
     * @param sun : sun light
     */
    void update(prt::vector<glm::mat4> const & modelMatrices, 
                prt::vector<glm::mat4> const & animatedModelMatrices,
                prt::vector<glm::mat4> const & bones,
                Camera & camera,
                SkyLight const & sun,
                prt::vector<UBOPointLight> const & pointLights,
                float t);

    void render(float deltaTime, RenderGroupMask renderGroupMask);

    static constexpr unsigned int COMMON_RENDER_GROUP = 0;
    static constexpr unsigned int GAME_RENDER_GROUP = 1;
    static constexpr RenderGroupMask GAME_RENDER_MASK = RENDER_GROUP_FLAG_0 | RENDER_GROUP_FLAG_1;
    static constexpr unsigned int EDITOR_RENDER_GROUP = 2;
    static constexpr RenderGroupMask EDITOR_RENDER_MASK = RENDER_GROUP_FLAG_0 | RENDER_GROUP_FLAG_2;

private:
    ImGuiRenderer m_imguiRenderer;

    static constexpr float depthBiasConstant = 0.0f;//0.01f;//1.25f;
    static constexpr float depthBiasSlope = 0.0f;//0.01f;//1.75f;
    float nearPlane = 0.03f;
    float farPlane = 500.0f;
    float maxShadowDistance = 100.0f;
    float cascadeSplitLambda = 0.85f;

    struct RenderPassIndices {
        unsigned int scene;
        unsigned int shadow;
    } renderPassIndices;


    struct FBAIndices {
        unsigned int depth;
        unsigned int guiDepth;
        prt::vector<unsigned int> accumulation;
        prt::vector<unsigned int> revealage;
        prt::vector<unsigned int> shadow;
        prt::vector<unsigned int> object;

        unsigned int objectCopy;
        unsigned int depthCopy;
    } fbaIndices;

    unsigned int shadowMapIndex;
    
    struct PipelineIndices {
        int skybox = -1;
        int opaque = -1;
        int opaqueAnimated = -1;
        int shadow = -1;
        int shadowAnimated = -1;
        int transparent = -1;
        int transparentAnimated = -1;
        int water = -1;
        int composition = -1;
        int gui = -1;
    } pipelineIndices;

    VkDescriptorImageInfo samplerInfo;

    void init();
    void initFBAs();
    void initPipelines();
    void initBuffersAndTextures(size_t standardAssetIndex, 
                                size_t animatedStandardAssetIndex,
                                size_t skyboxAssetIndex);
    void initTextures(size_t standardAssetIndex,
                      size_t animatedStandardAssetIndex);

    void createStandardAndShadowPipelines(size_t standardAssetIndex, size_t standardUboIndex,
                                          size_t shadowmapUboIndex, 
                                          const char * relativeVert, const char * relativeFrag,
                                          const char * relativeTransparentFrag,
                                          const char * relativeShadowVert,
                                          VkVertexInputBindingDescription bindingDescription,
                                          prt::vector<VkVertexInputAttributeDescription> const & attributeDescription,
                                          int & standardPipeline, 
                                          int & transparentPipeline,
                                          int & shadowPipeline);

    void createCompositionPipeline();

    void createSkyboxPipeline(size_t assetIndex, size_t uboIndex);

    int createStandardPipeline(size_t assetIndex, size_t uboIndex, 
                               char const * vertexShader, char const * fragmentShader,
                               VkVertexInputBindingDescription bindingDescription,
                               prt::vector<VkVertexInputAttributeDescription> const & attributeDescription,
                               bool transparent);
                                          
    int createShadowmapPipeline(size_t assetIndex, size_t uboIndex,
                                    char const * vertexShader,
                                    VkVertexInputBindingDescription bindingDescription,
                                    prt::vector<VkVertexInputAttributeDescription> const & attributeDescription);

    void createCommandBuffers();
    void createCommandBuffer(size_t imageIndex);

    void createVertexBuffers(Model const * models, size_t nModels, 
                             size_t staticAssetIndex, size_t animatedAssetIndex);
    
    void createIndexBuffers(Model const * models, size_t nModels,
                            size_t staticAssetIndex, size_t animatedAssetIndex);

    void createCubeMapBuffers(size_t assetIndex);
    
    void loadModels(Model const * models, size_t nModels, 
                    Texture const * textures, size_t nTextures,
                    size_t staticAssetIndex,
                    size_t animatedAssetIndex,
                    prt::hash_map<int, int> & staticTextureIndices,
                    prt::hash_map<int, int> & animatedTextureIndices);

    void loadTextures(size_t staticAssetIndex,
                      size_t animatedAssetIndex,
                      Model const * models, size_t nModels,
                      Texture const * textures,
                      prt::hash_map<int, int> & staticTextureIndices,
                      prt::hash_map<int, int> & animatedTextureIndices);

    void loadCubeMap(prt::array<Texture, 6> const & skybox, size_t assetIndex);

    void createSkyboxDrawCalls();
    void createModelDrawCalls(Model const * models,   size_t nModels,
                              ModelID const * staticModelIDs,
                              size_t nStaticModelIDs,
                              ModelID const * animatedModelIDs,
                              size_t nAnimatedModelIDs,
                              uint32_t const * boneOffsets,
                              prt::hash_map<int, int> const & staticTextureIndices,
                              prt::hash_map<int, int> const & animatedTextureIndices,
                              prt::vector<DrawCall> & standard,
                              prt::vector<DrawCall> & transparent,
                              prt::vector<DrawCall> & animated,
                              prt::vector<DrawCall> & transparentAnimated,
                              prt::vector<DrawCall> & water,
                              prt::vector<DrawCall> & shadow,
                              prt::vector<DrawCall> & shadowAnimated);

    void createShadowDrawCalls(size_t shadowPipelineIndex, size_t pipelineIndex);

    void createCompositionDrawCalls(size_t pipelineIndex);

    void updateUBOs(prt::vector<glm::mat4> const & nonAnimatedModelMatrices, 
                    prt::vector<glm::mat4> const & animatedModelMatrices,
                    prt::vector<glm::mat4> const & bones,
                    Camera & camera,
                    SkyLight const & sun,
                    prt::vector<UBOPointLight> const & pointLights,
                    float t);
                    
    void updateSkyboxUBO(Camera const & camera, SkyLight const & sky);

    void updateCascades(glm::mat4 const & projectionMatrix,
                        glm::mat4 const & viewMatrix,
                        glm::vec3 const & lightDir,
                        prt::array<glm::mat4, NUMBER_SHADOWMAP_CASCADES> & cascadeSpace,
                        prt::array<float, NUMBER_SHADOWMAP_CASCADES> & splitDepths);

    size_t pushBackDepthFBA();
    void pushBackAccumulationFBA();
    void pushBackRevealageFBA();
    void pushBackShadowFBA();

    void pushBackSceneRenderPass();
    void pushBackShadowRenderPass();

    void pushBackSunShadowMap();

    prt::vector<VkPipelineColorBlendAttachmentState> getOpaqueBlendAttachmentState();
    prt::vector<VkPipelineColorBlendAttachmentState> getTransparentBlendAttachmentState();
    prt::vector<VkPipelineColorBlendAttachmentState> getCompositionBlendAttachmentState();
    prt::vector<VkPipelineColorBlendAttachmentState> getShadowBlendAttachmentState();
};

#endif
