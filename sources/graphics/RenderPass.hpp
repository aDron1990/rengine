#pragma once

#include "RenderBackend.hpp"
#include "RenderContext.hpp"

class RenderPass {
public:
    ~RenderPass() = default;
    virtual void collect(const RenderContext& ctx) noexcept = 0;
    virtual void render(RenderBackend& backend, const RenderContext& ctx) noexcept = 0;
};