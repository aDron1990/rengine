#pragma once

#include "LineBatch.hpp"
#include <glm/glm.hpp>

struct OrbitalBody {
    glm::vec3 velocity { };
    LineBatch orbit;
};
