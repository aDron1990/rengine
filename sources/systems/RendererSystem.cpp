#include "RendererSystem.hpp"
#include "AABB.hpp"
#include "Cubemap.hpp"
#include "LineBatch.hpp"
#include "components/Renderer.hpp"
#include "components/Transform.hpp"
#include "utils.hpp"

#include <array>
#include <entt/entity/fwd.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <glm/geometric.hpp>
#include <vector>

RendererSystem::RendererSystem(entt::registry& registry, Camera& camera)
    : m_registry { registry }
    , m_shader { "resources/shaders/main_v.glsl", "resources/shaders/main_f.glsl" }
    , m_skyboxShader { "resources/shaders/cubemap_v.glsl", "resources/shaders/cubemap_f.glsl" }
    , m_transparentShader { "resources/shaders/transparent_v.glsl", "resources/shaders/transparent_f.glsl" }
    , m_linesShader { "resources/shaders/line_v.glsl", "resources/shaders/line_f.glsl" }
    , m_camera { camera }
{
    // Skybox
    m_skyboxTexture.reset(
        loadCubemap({ "resources/images/skybox/right.jpg",
            "resources/images/skybox/left.jpg",
            "resources/images/skybox/top.jpg",
            "resources/images/skybox/bottom.jpg",
            "resources/images/skybox/front.jpg",
            "resources/images/skybox/back.jpg" }));

    m_skyboxVAO.bind();
    m_skyboxVBO = VertexBuffer { skyboxVertices, sizeof(skyboxVertices) };
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
        (void*)0);
    glEnableVertexAttribArray(0);
}

std::array<Line, 12> aabbToLines(const AABB& aabb, const glm::mat4& model) noexcept;
std::array<Line, 12> aabbToLines(const AABB& aabb) noexcept;
void RendererSystem::render(const glm::mat4& view, const glm::mat4& proj) noexcept
{
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

    glm::vec3 lightPos { -15.0f, 15.0f, 15.0f };
    m_shader.setUniform(lightPos, "light.position");
    m_shader.setUniform(glm::vec3 { 0.2f, 0.2f, 0.2f }, "light.ambient");
    m_shader.setUniform(glm::vec3 { 0.5f, 0.5f, 0.5f }, "light.diffuse");
    m_shader.setUniform(glm::vec3 { 1.0f, 1.0f, 1.0f }, "light.specular");
    m_shader.setUniform(m_camera.getPos(), "viewPos");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto objects = m_registry.view<Transform, Renderer>(entt::exclude<Transparent>);
    for (auto [entity, transform, renderer] : objects.each()) {
        auto model = transform.getMatrix();

        m_shader.setUniform(model, "model");

        m_shader.setUniform(glm::vec3 { 0.8f, 0.8f, 0.8f }, "material.ambient");
        m_shader.setUniform(glm::vec3 { 0.5f, 0.5f, 0.5f }, "material.specular");
        m_shader.setUniform(0, "material.diffuse");
        m_shader.setUniform(1, "material.specular");
        m_shader.setUniform(32.0f, "material.shininess");

        renderer.texture->bind(0);
        renderer.specular->bind(1);

        renderer.mesh->draw();
    }

    m_transparentShader.use();
    m_transparentShader.setUniform(view, "view");
    m_transparentShader.setUniform(proj, "proj");

    auto tobjects = m_registry.view<Transform, Renderer, Transparent>();
    std::vector<std::pair<float, entt::entity>> sorted;
    for (auto entity : tobjects) {
        auto& transform = tobjects.get<Transform>(entity);
        auto dist = glm::distance2(transform.position, m_camera.getPos());
        sorted.push_back({ dist, entity });
    }
    std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) {
        return a.first > b.first;
    });

    for (auto [_, entity] : sorted) {
        auto [transform, renderer] = tobjects.get<Transform, Renderer>(entity);
        auto model = transform.getMatrix();
        m_transparentShader.setUniform(model, "model");
        renderer.texture->bind();
        renderer.mesh->draw();
    }

    glClear(GL_DEPTH_BUFFER_BIT);

    m_linesShader.use();
    m_linesShader.setUniform(view, "view");
    m_linesShader.setUniform(proj, "proj");

    LineBatch lines { };
    for (auto [_, aabb, transform] : m_registry.view<AABB, Transform>().each()) {
        auto model = transform.getMatrix();

        auto linesArray = aabbToLines(aabb, model);
        AABB alignedAABB { };
        for (const auto& line : linesArray) {
            alignedAABB.min.x = std::min(alignedAABB.min.x, line.p1.x);
            alignedAABB.min.x = std::min(alignedAABB.min.x, line.p2.x);
            alignedAABB.max.x = std::max(alignedAABB.max.x, line.p1.x);
            alignedAABB.max.x = std::max(alignedAABB.max.x, line.p2.x);

            alignedAABB.min.y = std::min(alignedAABB.min.y, line.p1.y);
            alignedAABB.min.y = std::min(alignedAABB.min.y, line.p2.y);
            alignedAABB.max.y = std::max(alignedAABB.max.y, line.p1.y);
            alignedAABB.max.y = std::max(alignedAABB.max.y, line.p2.y);

            alignedAABB.min.z = std::min(alignedAABB.min.z, line.p1.z);
            alignedAABB.min.z = std::min(alignedAABB.min.z, line.p2.z);
            alignedAABB.max.z = std::max(alignedAABB.max.z, line.p1.z);
            alignedAABB.max.z = std::max(alignedAABB.max.z, line.p2.z);
        }

        auto alignedLinesArray = aabbToLines(alignedAABB);
        for (const auto& line : alignedLinesArray) {
            lines.pushLine(line);
        }
    }

    lines.draw();
}

std::array<Line, 12> aabbToLines(const AABB& aabb, const glm::mat4& model) noexcept
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

    for (int i = 0; i < 8; i++) {
        corners[i] = glm::vec3(model * glm::vec4(corners[i], 1.0f));
    }

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

std::array<Line, 12> aabbToLines(const AABB& aabb) noexcept
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