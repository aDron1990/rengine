#pragma once

#include <cmath>
#include <glm/glm.hpp>

struct AABB {
    glm::vec3 min{+INFINITY};
    glm::vec3 max{-INFINITY};
};

