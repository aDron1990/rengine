#pragma once

#include "RenderBackend.hpp"
#include "RenderLayer.hpp"

#include <entt/entity/fwd.hpp>
#include <glm/ext/vector_int2.hpp>

#include <vector>

struct RenderContext {
    const entt::registry& registry;
    const std::vector<RenderLayer>& layers;
    entt::entity defaultLayerCamera;
    glm::ivec2 defaultLayerSize;
    glm::mat4 view { 1.0f };
    glm::mat4 proj { 1.0f };
};

inline void bindLayer(int nlayer, RenderBackend& backend, const RenderContext& ctx) noexcept
{
    if (nlayer == DEFAULT_RENDER_LAYER) {
        backend.bindDefaultFramebuffer();
    } else {
        auto& layer = ctx.layers[nlayer];
        backend.bindFramebuffer(layer.texture);
    }
}

inline entt::entity getLayerCamera(int nlayer, RenderBackend& backend, const RenderContext& ctx) noexcept
{
    if (nlayer == DEFAULT_RENDER_LAYER) {
        return ctx.defaultLayerCamera;
    } else {
        auto& layer = ctx.layers[nlayer];
        return layer.camera;
    }
}

inline glm::ivec2 getLayerSize(int nlayer, RenderBackend& backend, const RenderContext& ctx) noexcept
{
    if (nlayer == DEFAULT_RENDER_LAYER) {
        return ctx.defaultLayerSize;
    } else {
        auto& layer = ctx.layers[nlayer];
        return backend.getRenderTextureSize(layer.texture);
    }
}
