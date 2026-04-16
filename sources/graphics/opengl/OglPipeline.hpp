#pragma once

#include "OglShader.hpp"
#include "graphics/types.hpp"

struct OglPipeline {
    PipelineParams params;
    RenderState state;
    OglShader shader;
};