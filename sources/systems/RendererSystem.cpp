#include "RendererSystem.hpp"
#include "BoundingBox.hpp"
#include "Cubemap.hpp"
#include "Frustum.hpp"
#include "LineBatch.hpp"
#include "OrbitalEngine.hpp"
#include "components/Camera.hpp"
#include "components/Celestial.hpp"
#include "components/OrbitalBody.hpp"
#include "components/Renderer.hpp"
#include "components/Transform.hpp"
#include "systems/Clock.hpp"
#include "utils.hpp"

#include <array>
#include <entt/entity/fwd.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <imgui.h>
#include <numeric>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <glm/geometric.hpp>
#include <vector>

RendererSystem::RendererSystem(entt::registry& registry)
    : m_registry { registry }
    , m_shader { "resources/shaders/main_v.glsl", "resources/shaders/main_f.glsl" }
    , m_skyboxShader { "resources/shaders/cubemap_v.glsl", "resources/shaders/cubemap_f.glsl" }
    , m_transparentShader { "resources/shaders/transparent_v.glsl", "resources/shaders/transparent_f.glsl" }
    , m_linesShader { "resources/shaders/line_v.glsl", "resources/shaders/line_f.glsl" }
{
    // Skybox
    m_skyboxTexture.reset(
        loadCubemap({ "resources/images/space_skybox/right.png",
            "resources/images/space_skybox/left.png",
            "resources/images/space_skybox/top.png",
            "resources/images/space_skybox/bottom.png",
            "resources/images/space_skybox/front.png",
            "resources/images/space_skybox/back.png" }));

    m_skyboxVAO.bind();
    m_skyboxVBO = VertexBuffer { skyboxVertices, sizeof(skyboxVertices) };
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
        (void*)0);
    glEnableVertexAttribArray(0);
}

std::array<Line, 12> toLines(const BoundingBox& aabb, const Transform& transform) noexcept;
std::array<Line, 12> toLinesAligned(const BoundingBox& aabb, const Transform& transform) noexcept;

void RendererSystem::render(const glm::mat4& proj) noexcept
{
    m_lastFrametimes[m_currentFrame] = m_registry.ctx().get<Clock>().getDelta();
    m_currentFrame = (m_currentFrame + 1) % FRAMES;

    if (m_currentFrame == 0) {
        float sum = std::accumulate(m_lastFrametimes.begin(), m_lastFrametimes.end(), 0.0f);
        float average = sum / FRAMES;
        FPS = 1.0f / average;
    }

    auto cameraEntity = m_registry.view<Camera, Transform>().front();
    auto [camera, cameraTransform] = m_registry.get<Camera, Transform>(cameraEntity);
    auto view = camera.getView(cameraTransform.position);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto view_ = glm::mat4(glm::mat3(view));
    glDepthMask(GL_FALSE);
    m_skyboxShader.use();
    m_skyboxShader.setUniform(view_, "view");
    m_skyboxShader.setUniform(proj, "proj");
    m_skyboxVAO.bind();
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture.get());
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);

    m_shader.use();

    m_shader.setUniform(view, "view");
    m_shader.setUniform(proj, "proj");

    Frustum frustum { proj * view };

    glm::vec3 lightPos { -15.0f, 15.0f, 15.0f };
    m_shader.setUniform(lightPos, "light.position");
    m_shader.setUniform(glm::vec3 { 0.2f, 0.2f, 0.2f }, "light.ambient");
    m_shader.setUniform(glm::vec3 { 0.5f, 0.5f, 0.5f }, "light.diffuse");
    m_shader.setUniform(glm::vec3 { 1.0f, 1.0f, 1.0f }, "light.specular");
    m_shader.setUniform(cameraTransform.position, "viewPos");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int drawed = 0;
    auto objects = m_registry.view<Transform, Renderer, BoundingBox>(entt::exclude<Transparent>);
    LineBatch orientationLines;
    for (auto [entity, transform, renderer, bb] : objects.each()) {
        auto aabb = toGlobalAABB(bb, transform);
        if (!frustum.isVisible(aabb))
            continue;
        drawed++;

        auto model = transform.getMatrix();
        m_shader.setUniform(model, "model");

        m_shader.setUniform(glm::vec3 { 0.8f, 0.8f, 0.8f }, "material.ambient");
        m_shader.setUniform(glm::vec3 { 0.5f, 0.5f, 0.5f }, "material.specular");
        m_shader.setUniform(0, "material.diffuse");
        m_shader.setUniform(1, "material.specular");
        m_shader.setUniform(32.0f, "material.shininess");

        renderer.texture->bind(0);
        renderer.specular->bind(1);

        renderer.model->draw();

        auto front = transform.position + (transform.rotation * glm::vec3(0, 0, -1));
        orientationLines.push({ transform.position, front });
    }

    m_transparentShader.use();
    m_transparentShader.setUniform(view, "view");
    m_transparentShader.setUniform(proj, "proj");

    auto tobjects = m_registry.view<Transform, Renderer, Transparent, BoundingBox>();
    std::vector<std::pair<float, entt::entity>> sorted;
    for (auto entity : tobjects) {
        auto& transform = tobjects.get<Transform>(entity);
        auto dist = glm::distance2(transform.position, cameraTransform.position);
        sorted.push_back({ dist, entity });
    }
    std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) {
        return a.first > b.first;
    });

    for (auto [_, entity] : sorted) {
        auto [transform, renderer, bb] = tobjects.get<Transform, Renderer, BoundingBox>(entity);

        auto aabb = toGlobalAABB(bb, transform);
        if (!frustum.isVisible(aabb))
            continue;
        drawed++;

        auto model = transform.getMatrix();
        m_transparentShader.setUniform(model, "model");
        renderer.texture->bind();
        renderer.model->draw();

        orientationLines.push({ transform.position, transform.position + (transform.rotation * glm::vec3(0, 0, -1)) });
    }

    ImGui::Begin("Render");
    ImGui::Text("FPS: %f", FPS);
    ImGui::Text("%d objects drawed", drawed);
    ImGui::End();

    glClear(GL_DEPTH_BUFFER_BIT);

    m_linesShader.use();
    m_linesShader.setUniform(view, "view");
    m_linesShader.setUniform(proj, "proj");

    orientationLines.draw();
    LineBatch lines { };
    for (auto [_, bb, transform, renderer] : m_registry.view<BoundingBox, Transform, Renderer, Picked>().each()) {
        auto model = glm::mat4 { 1.0f };
        model = glm::translate(model, transform.position);
        model = glm::scale(model, transform.scale);

        auto linesArray = toLines(bb, transform);
        for (const auto& line : linesArray) {
            lines.push(line);
        }

        if (renderer.drawAABB) {
            auto aabb = toGlobalAABB(bb, transform);
            auto alignedLinesArray = toLinesAligned(aabb, transform);
            for (const auto& line : alignedLinesArray) {
                lines.push(line);
            }
        }
    }

    lines.draw();

    auto& orbital = m_registry.ctx().emplace<OrbiralEngine>(m_registry);
    LineBatch orbitLines { };
    auto celView = m_registry.view<Celestial, Transform>();
    auto [celestial, celTransform] = m_registry.get<Celestial, Transform>(celView.front());
    for (auto [e, transform, body] : m_registry.view<Transform, OrbitalBody>().each()) {
        body.orbit.draw();
    }
}

std::array<Line, 12> toLines(const BoundingBox& aabb, const Transform& transform) noexcept
{
    auto model = transform.getMatrix();

    glm::vec3 corners[] = {
        { aabb.min.x, aabb.min.y, aabb.min.z },
        { aabb.max.x, aabb.min.y, aabb.min.z },
        { aabb.min.x, aabb.max.y, aabb.min.z },
        { aabb.max.x, aabb.max.y, aabb.min.z },
        { aabb.min.x, aabb.min.y, aabb.max.z },
        { aabb.max.x, aabb.min.y, aabb.max.z },
        { aabb.min.x, aabb.max.y, aabb.max.z },
        { aabb.max.x, aabb.max.y, aabb.max.z }
    };

    std::array<Line, 12> result;
    result[0] = { corners[0], corners[1] };
    result[1] = { corners[1], corners[3] };
    result[2] = { corners[3], corners[2] };
    result[3] = { corners[2], corners[0] };
    result[4] = { corners[4], corners[5] };
    result[5] = { corners[5], corners[7] };
    result[6] = { corners[7], corners[6] };
    result[7] = { corners[6], corners[4] };
    result[8] = { corners[0], corners[4] };
    result[9] = { corners[1], corners[5] };
    result[10] = { corners[2], corners[6] };
    result[11] = { corners[3], corners[7] };

    for (auto& line : result) {
        line.p1 = glm::vec3 { model * glm::vec4 { line.p1, 1.0f } };
        line.p2 = glm::vec3 { model * glm::vec4 { line.p2, 1.0f } };
    }

    return result;
}

std::array<Line, 12> toLinesAligned(const BoundingBox& aabb, const Transform& transform) noexcept
{
    glm::vec3 corners[] = {
        { aabb.min.x, aabb.min.y, aabb.min.z },
        { aabb.max.x, aabb.min.y, aabb.min.z },
        { aabb.min.x, aabb.max.y, aabb.min.z },
        { aabb.max.x, aabb.max.y, aabb.min.z },
        { aabb.min.x, aabb.min.y, aabb.max.z },
        { aabb.max.x, aabb.min.y, aabb.max.z },
        { aabb.min.x, aabb.max.y, aabb.max.z },
        { aabb.max.x, aabb.max.y, aabb.max.z }
    };

    std::array<Line, 12> result;
    result[0] = { corners[0], corners[1] };
    result[1] = { corners[1], corners[3] };
    result[2] = { corners[3], corners[2] };
    result[3] = { corners[2], corners[0] };
    result[4] = { corners[4], corners[5] };
    result[5] = { corners[5], corners[7] };
    result[6] = { corners[7], corners[6] };
    result[7] = { corners[6], corners[4] };
    result[8] = { corners[0], corners[4] };
    result[9] = { corners[1], corners[5] };
    result[10] = { corners[2], corners[6] };
    result[11] = { corners[3], corners[7] };

    return result;
}