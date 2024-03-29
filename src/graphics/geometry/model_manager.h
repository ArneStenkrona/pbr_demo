#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include "src/container/hash_map.h"
#include "src/container/vector.h"

#include "src/graphics/geometry/texture_manager.h"

#include "src/graphics/geometry/model.h"

/* Animation blending */
struct BlendedAnimation {
    uint32_t clipA;
    uint32_t clipB;
    float blendFactor = 0.0f;
    float time = 0.0f;
    bool paused = false;
};

class ModelManager {
public:
    ModelManager(const char * directory, TextureManager & textureManager);

    inline void getModels(Model const * & models, size_t & n) const { models = m_loadedModels.data();
                                                                      n = m_loadedModels.size(); }

    inline Model const & getModel(ModelID id) const { return m_loadedModels[id]; }

    void getBoneOffsets(ModelID const * modelIDs,
                        uint32_t * boneOffsets,
                        size_t n);

    void getSampledAnimation(float t, 
                             prt::vector<ModelID> const & modelIDs,
                             prt::vector<uint32_t> const & animationIndices, 
                             prt::vector<glm::mat4> & transforms);

    void getSampledBlendedAnimation(ModelID const * modelIDs,
                                    BlendedAnimation const * animationBlends, 
                                    prt::vector<glm::mat4> & transforms,
                                    size_t n);

    static bool defAlreadyLoaded;
    ModelID loadModel(char const * path, 
                      bool animated, bool & alreadyLoaded = defAlreadyLoaded);

    uint32_t getAnimationIndex(ModelID modelID, char const * name);

private:
    TextureManager & m_textureManager;  

    prt::hash_map<std::string, ModelID> m_pathToModelID;
    char m_modelDirectory[256];

    prt::vector<Model> m_loadedModels;
};

#endif