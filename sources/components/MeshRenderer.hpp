#pragma once

#include "Model.hpp"
#include "graphics/RenderLayer.hpp"
#include "graphics/types.hpp"

#include <memory>

struct MeshRenderer {
    std::shared_ptr<Model> model;
    TextureID texture;
    TextureID specular;
    bool drawAABB = false;
    int layer = DEFAULT_RENDER_LAYER;
};
