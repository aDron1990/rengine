#pragma once

#include "BoundingBox.hpp"

#include <glm/glm.hpp>

#include <array>

struct Plane {
    glm::vec3 normal { };
    float d;
};

struct Frustum {
    inline Frustum(const glm::mat4& viewProj)
    {
        auto& m = viewProj;

        // left
        planes[0].normal.x = m[0][3] + m[0][0];
        planes[0].normal.y = m[1][3] + m[1][0];
        planes[0].normal.z = m[2][3] + m[2][0];
        planes[0].d = m[3][3] + m[3][0];

        // right
        planes[1].normal.x = m[0][3] - m[0][0];
        planes[1].normal.y = m[1][3] - m[1][0];
        planes[1].normal.z = m[2][3] - m[2][0];
        planes[1].d = m[3][3] - m[3][0];

        // bottom
        planes[2].normal.x = m[0][3] + m[0][1];
        planes[2].normal.y = m[1][3] + m[1][1];
        planes[2].normal.z = m[2][3] + m[2][1];
        planes[2].d = m[3][3] + m[3][1];

        // top
        planes[3].normal.x = m[0][3] - m[0][1];
        planes[3].normal.y = m[1][3] - m[1][1];
        planes[3].normal.z = m[2][3] - m[2][1];
        planes[3].d = m[3][3] - m[3][1];

        // near
        planes[4].normal.x = m[0][3] + m[0][2];
        planes[4].normal.y = m[1][3] + m[1][2];
        planes[4].normal.z = m[2][3] + m[2][2];
        planes[4].d = m[3][3] + m[3][2];

        // far
        planes[5].normal.x = m[0][3] - m[0][2];
        planes[5].normal.y = m[1][3] - m[1][2];
        planes[5].normal.z = m[2][3] - m[2][2];
        planes[5].d = m[3][3] - m[3][2];

        for (auto& p : planes) {
            float len = glm::length(p.normal);
            p.normal /= len;
            p.d /= len;
        }
    }

    bool isVisible(const BoundingBox& aabb)
    {
        for (const auto& plane : planes) {
            glm::vec3 positive;

            positive.x = (plane.normal.x >= 0) ? aabb.max.x : aabb.min.x;
            positive.y = (plane.normal.y >= 0) ? aabb.max.y : aabb.min.y;
            positive.z = (plane.normal.z >= 0) ? aabb.max.z : aabb.min.z;

            if (glm::dot(plane.normal, positive) + plane.d < 0) {
                return false;
            }
        }

        return true;
    }

    std::array<Plane, 6> planes { };
};
