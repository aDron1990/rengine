#pragma once

#include "graphics/Texture.hpp"

#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>

#include <memory>

struct Material {
    std::shared_ptr<Texture> diffuse;
    glm::vec3 specular;
    float shininess;
};
