#include "application.h"

#include "src/container/vector.h"
#include "src/config/config.h"

#include <GLFW/glfw3.h>

#include <chrono>
#include <thread>

#include <iostream>

Application::Application()
: m_input(),
  m_renderer(DEFAULT_WIDTH, DEFAULT_HEIGHT),
  m_camera(m_input),
  m_assetManager(RESOURCE_PATH),
  m_frameRate(FRAME_RATE),
  m_microsecondsPerFrame(1000000 / m_frameRate),
  m_currentFrame(0),
  m_time(0.0f) {
    m_input.init(m_renderer.getWindow());
    loadScene();
}

Application::~Application() {
}

void Application::run() {
    using clock = std::chrono::high_resolution_clock;
    //static auto startTime = std::chrono::high_resolution_clock::now();
    auto lastTime = clock::now();
    clock::time_point deadLine = clock::now();

    uint32_t framesMeasured = 0;
    clock::time_point nextSecond = lastTime + std::chrono::seconds(1);
        
    while (m_renderer.isWindowOpen()) {
        deadLine = deadLine + std::chrono::microseconds(m_microsecondsPerFrame);
        auto currentTime = clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
        lastTime = currentTime;

        update(deltaTime);

        std::this_thread::sleep_until(deadLine);

        framesMeasured++;
        if (nextSecond <= clock::now()) {
            nextSecond += std::chrono::seconds(1);
            std::cout << "Frame rate: " << framesMeasured << "FPS" << std::endl;
            framesMeasured = 0;
        }

        m_currentFrame++;

        glfwPollEvents();
    }
}

void Application::update(float deltaTime) {
    m_time += deltaTime;
    m_input.update(false);
    updateSun();
    updateCamera(deltaTime);
    renderScene(m_camera, deltaTime);
}

void Application::updateCamera(float deltaTime) {
    m_camera.update(deltaTime, true);
}

void Application::updateSun() {
    float ph = 0.2f*m_time;
    m_sun.phase = ph;
    m_sun.direction = glm::normalize(glm::vec3(0.2f, glm::cos(ph), glm::sin(ph)));
    float distToNoon = glm::acos(glm::dot(-m_sun.direction, glm::vec3(0,1,0))) / glm::pi<float>();
    m_sun.color = glm::mix(glm::vec3(255,255,255), glm::vec3(255,153,51), distToNoon)/255.0f;

    m_sun.distToNoon = glm::acos(glm::dot(-m_sun.direction, glm::vec3(0.0f,1.0f,0.0f))) / glm::pi<float>();
    m_sun.nightColor = glm::mix(glm::vec3(80.0f,80.0f,250.0f), glm::vec3(0.0f), m_sun.distToNoon)/255.0f;
    m_sun.dayColor = glm::mix(glm::vec3(204.0f,204.0f,255.0f), glm::vec3(5.0f,5.0f,25.0f), m_sun.distToNoon)/255.0f;
    m_sun.sunEdgeColor = glm::vec3(255.0f,119.0f,51.0f)/255.0f;
    m_sun.sunsetriseColor = glm::vec3(255.0f,119.0f,51.0f)/255.0f;
    m_sun.sunColor = glm::mix(glm::vec3(255.0f,255.0f,230.0f), glm::vec3(255.0f,153.0f,51.0f), m_sun.distToNoon)/255.0f;
}

void Application::updateRenderData() {
    m_renderData.staticTransforms[0] = glm::mat4{1.0f};
}

void Application::sampleAnimation(prt::vector<glm::mat4> & bones) {
    prt::vector<BlendedAnimation> blends;
    blends.resize(m_renderData.animatedModelIDs.size());

    m_assetManager.getModelManager().getSampledBlendedAnimation(m_renderData.animatedModelIDs.data(),
                                                                blends.data(),
                                                                bones,
                                                                m_renderData.animatedModelIDs.size());
}

void Application::renderScene(Camera & camera, float deltaTime) {
    updateRenderData();

    prt::vector<glm::mat4> bones; 
    sampleAnimation(bones);

    prt::vector<UBOPointLight> pointLights;

    double x,y;
    m_input.getCursorPos(x,y);
    m_renderer.update(m_renderData.staticTransforms, 
                      m_renderData.animatedTransforms,
                      bones,
                      camera, 
                      m_sun,
                      pointLights,
                      m_time);

    m_renderer.render(deltaTime, m_renderMask);
}

void Application::loadScene() {
    bindRenderData();
    
    prt::array<Texture, 6> skybox;
    getSkybox(skybox);

    m_renderer.bindAssets(m_renderData.models,
                          m_renderData.nModels,
                          m_renderData.staticModelIDs.data(),
                          m_renderData.staticModelIDs.size(),
                          m_renderData.animatedModelIDs.data(),
                          m_renderData.boneOffsets.data(),
                          m_renderData.animatedModelIDs.size(),
                          m_renderData.textures, m_renderData.nTextures,
                          skybox);
}

void Application::bindRenderData() {
    // clear previous render data
    m_renderData.staticTransforms.resize(0);
    m_renderData.staticModelIDs.resize(0);
    m_renderData.animatedTransforms.resize(0);
    m_renderData.animatedModelIDs.resize(0);
    m_renderData.boneOffsets.resize(0);

    ModelID modelID = m_assetManager.getModelManager().loadModel("docks/docks.dae", false);
    m_renderData.staticModelIDs.push_back(modelID);

    m_assetManager.getModelManager().getModels(m_renderData.models, m_renderData.nModels);

    m_renderData.staticTransforms.resize(m_renderData.staticModelIDs.size());
    m_renderData.animatedTransforms.resize(m_renderData.animatedModelIDs.size());

    m_renderData.boneOffsets.resize(m_renderData.animatedModelIDs.size());
    m_assetManager.getModelManager().getBoneOffsets(m_renderData.animatedModelIDs.data(),
                                                    m_renderData.boneOffsets.data(),
                                                    m_renderData.animatedModelIDs.size());

    m_assetManager.getTextureManager().getTextures(m_renderData.textures, m_renderData.nTextures);
}

void Application::getSkybox(prt::array<Texture, 6> & cubeMap) const {
    m_assetManager.loadCubeMap("stars", cubeMap);
}
