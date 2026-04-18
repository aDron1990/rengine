#pragma once

#include "graphics/RenderLayer.hpp"
#include "utils/types.hpp"

#include <vector>

struct LineRenderer {
    std::vector<Line> lines;
    int layer = DEFAULT_RENDER_LAYER;
};
