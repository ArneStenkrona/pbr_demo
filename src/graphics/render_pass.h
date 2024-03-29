#ifndef RENDER_PASS_H
#define RENDER_PASS_H

#include <vulkan/vulkan.h>

#include "src/graphics/graphics_pipeline.h"
#include "src/container/vector.h"

struct SubPass {
    VkPipelineBindPoint                        bindPoint;
    prt::vector<VkAttachmentReference>         colorReferences;
    VkAttachmentReference                      depthReference = {0,VK_IMAGE_LAYOUT_END_RANGE}; // use VK_IMAGE_LAYOUT_END_RANGE to signify no depth reference
    prt::vector<VkAttachmentReference>         inputReferences;
    prt::vector<prt::vector<VkCommandBuffer> > commandBuffers;
    bool                                       dynamic = false;
};

struct RenderPass {
    VkExtent2D                           extent;
    prt::vector<VkAttachmentDescription> attachments;
    prt::vector<SubPass>                 subpasses;
    prt::vector<VkSubpassDependency>     dependencies;

    VkRenderPass renderPass;

    prt::vector<VkClearValue> clearValues;

    /*
     * TODO: find better solution to
     * swapchain recreation issues
     **/
    enum RenderOutputType {
        RENDER_OUTPUT_TYPE_SWAPCHAIN,
        RENDER_OUTPUT_TYPE_SHADOWMAP
    };
    RenderOutputType outputType;

    size_t shadowMapIndex; // only used for RENDER_OUTPUT_TYPE_SHADOWMAP

    /*
     * TODO: Refactor this monstrosity
     * It is only used to set the current
     * cascade index through push constant
     * when rendering to shadow map
     **/
    int pushConstantFBIndexByteOffset = -1;
};

#endif
