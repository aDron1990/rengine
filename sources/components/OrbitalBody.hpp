#pragma once

#include "utils/types.hpp"
#include <glm/ext/vector_double3.hpp>
#include <vector>

struct OrbitalBody {
    glm::dvec3 velocityKmPerSec { };
    std::vector<Line> orbit;
};
