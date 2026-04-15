#pragma once

#include "graphics/LineBatch.hpp"
#include <glm/glm.hpp>

struct OrbitalBody {
    glm::vec3 velocity { };
    LineBatch orbit;
};
