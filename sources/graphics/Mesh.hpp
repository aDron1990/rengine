#pragma once

#include "BoundingBox.hpp"
#include "types.hpp"
#include "utils/types.hpp"

#include <vector>

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    BoundingBox aabb;
    MeshID meshID;
};
