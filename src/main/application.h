#ifndef PBR_APPLICATION_H
#define PBR_APPLICATION_H

#include "src/graphics/geometry/model_manager.h"
#include "src/graphics/geometry/asset_manager.h"
#include "src/graphics/camera.h"
#include "src/graphics/renderer.h"
#include "src/graphics/renderer.h"

struct RenderData {
    Texture const * textures;
    size_t nTextures;

    Model const * models;
    size_t nModels;

    prt::vector<glm::mat4> staticTransforms;
    prt::vector<ModelID>   staticModelIDs;

    prt::vector<glm::mat4> animatedTransforms;
    prt::vector<ModelID>   animatedModelIDs;
    prt::vector<uint32_t>  boneOffsets;
};

class Application {
public:
    Application();
    ~Application();

    void run();

private:
    RenderGroupMask m_renderMask = RENDER_GROUP_FLAG_ALL;

    Input m_input;
    Renderer m_renderer;
    RenderData m_renderData;
    Camera m_camera;

    AssetManager m_assetManager;

    uint32_t m_frameRate;
    uint32_t m_microsecondsPerFrame;

    SkyLight m_sun;

    uint64_t m_currentFrame;
    float m_time;

    static constexpr int DEFAULT_WIDTH = 800;
    static constexpr int DEFAULT_HEIGHT = 600;

    void update(float deltaTime);
    void updateCamera(float deltaTime);
    void updateRenderData();
    void sampleAnimation(prt::vector<glm::mat4> & bones);
    void renderScene(Camera & camera, float deltaTime);

    void loadScene();
    void bindRenderData();
    void getSkybox(prt::array<Texture, 6>& cubeMap) const;
};

#endif