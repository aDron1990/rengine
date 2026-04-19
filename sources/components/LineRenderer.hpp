#pragma once

#include "graphics/RenderLayer.hpp"
#include "utils/types.hpp"

#include <vector>

struct LineRenderer {
    std::vector<Line> lines;
    glm::vec4 color = { 0.0f, 1.0f, 0.0f, 1.0f };
    int layer = DEFAULT_RENDER_LAYER;
    bool draw = true;
};
