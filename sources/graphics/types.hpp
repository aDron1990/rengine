#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>

#include <string>
#include <variant>

using ID = size_t;
using MeshID = ID;
using TextureID = ID;
using PipelineID = ID;
using CubemapID = ID;
using RenderTextureID = ID;

struct Cull {
    enum class Front {
        CW,
        CCW
    } front { Cull::Front::CCW };
    enum class Mode {
        None,
        Front,
        Back
    } mode { Cull::Mode::None };
};

enum class Primitive {
    Triangle,
    Line,
};

struct RenderState {
    bool depth = true;
    bool blend = true;
    Cull cull;
    Primitive primitive { Primitive::Triangle };
};

struct PipelineParams {
    std::string vertexPath;
    std::string fragmentPath;
};

using Value = std::variant<glm::mat4, glm::vec3, float, int>;
