#pragma once

#include "types.hpp"

#include <entt/entity/fwd.hpp>

constexpr int DEFAULT_RENDER_LAYER = -1;

struct RenderLayer {
    RenderTextureID texture;
    entt::entity camera;
};