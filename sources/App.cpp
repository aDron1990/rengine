#include "App.hpp"

#include <GLFW/glfw3.h>
#include <cmath>
#include <entt/entity/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include <stdexcept>

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Math/Real.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include "BoundingBox.hpp"
#include "Input.hpp"
#include "Mesh.hpp"
#include "Object.hpp"
#include "Texture.hpp"
#include "components/Renderer.hpp"
#include "components/Transform.hpp"
#include "systems/PhysicsEngine.hpp"
#include "systems/RendererSystem.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

App::App(int windowWidth, int windowHeight, const std::string& windowTitle)
    : m_registry { }
    , m_windowSize { windowWidth, windowHeight }
{
    m_window = GlfwWindowPtr(glfwCreateWindow(
        windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr));
    if (m_window == nullptr) {
        throw std::runtime_error { "Failed to create GLFW window" };
    }
    m_input.setGlfwWindow(m_window.get());

    glfwSetWindowUserPointer(m_window.get(), this);

    glfwMakeContextCurrent(m_window.get());
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error { "Failed to init GLEW" };
    }

    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);
}

static bool showCursor = false;
void App::run()
{
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory { };
    JPH::RegisterTypes();
    JPH::TempAllocatorImpl tempAllocator { 10 * 1024 * 1024 };
    JPH::JobSystemThreadPool jobSystem {
        JPH::cMaxPhysicsJobs,
        JPH::cMaxPhysicsBarriers,
        (int)std::thread::hardware_concurrency()
    };
    PhysicsEngine m_physics { m_registry, tempAllocator, jobSystem };

    RendererSystem renderer { m_registry, m_camera };

    auto& shader = renderer.getShader();

    auto planeMesh = std::make_shared<Mesh>("resources/models/plane.obj");
    auto obMesh = std::make_shared<Mesh>("resources/models/ob.obj");
    auto cubeMesh = std::make_shared<Mesh>("resources/models/cube.obj");

    auto whiteTexture = std::make_shared<Texture>("resources/images/white.png");
    auto floorTexture = std::make_shared<Texture>("resources/images/floor.jpg");
    auto containerTexture = std::make_shared<Texture>("resources/images/container2.png");
    auto containerSpecularTexture = std::make_shared<Texture>("resources/images/container2_specular.png");
    auto windowTexture = std::make_shared<Texture>("resources/images/window.png");

    Object floor { m_registry, cubeMesh, floorTexture, whiteTexture };
    Object cube { m_registry, cubeMesh, containerTexture, whiteTexture };
    Object ob { m_registry, obMesh, containerTexture, whiteTexture };
    
    floor.scale() = { 20.0f, .2f, 20.0f };
    cube.position() = { .0f, 5.f, 0.0f };
    ob.position() = { 0.0f, 2.5f, 0.0f };

    m_physics.createCollider(floor.getEntity(), false);
    m_physics.createCollider(cube.getEntity());
    m_physics.createCollider(ob.getEntity());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui_ImplGlfw_InitForOpenGL(m_window.get(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glm::vec3 lightPos { -15.0f, 15.0f, 15.0f };

    while (m_running) {
        updateWindow();
        m_physics.update();

        processInput();

        if (m_input.getKeyDown(GLFW_KEY_Q)) {
            showCursor = !showCursor;
            glfwSetInputMode(m_window.get(), GLFW_CURSOR, showCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        auto pickedView = m_registry.view<Picked, Transform, Renderer>();
        if (pickedView.size_hint() > 0) {
            auto entity = pickedView.front();
            auto [transform, renderer] = m_registry.get<Transform, Renderer>(entity);

            ImGui::Begin("Inspector");

            ImGui::SeparatorText("Transform");
            ImGui::DragFloat3("position", glm::value_ptr(transform.position), 0.025f);
            ImGui::DragFloat3("rotation", glm::value_ptr(transform.rotation), 0.5f);
            ImGui::DragFloat3("scale", glm::value_ptr(transform.scale), 0.025f);

            ImGui::SeparatorText("Renderer");
            ImGui::Checkbox("Draw AABB", &renderer.drawAABB);

            ImGui::End();

            m_physics.applyTransform(entity);
        }

        auto proj = glm::perspective(glm::radians(60.0f),
            (float)m_windowSize.x / m_windowSize.y,
            0.1f, 100.0f);
        auto& view = m_camera.getView();

        shader.setUniform(lightPos, "light.position");

        renderer.render(view, proj);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_input.update();
        glfwSwapBuffers(m_window.get());
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

Input& App::getInput() noexcept { return m_input; }

void App::updateWindow() noexcept { glfwPollEvents(); }

void App::close() noexcept { m_running = false; }

void App::processInput() noexcept
{
    if (m_input.getKey(GLFW_KEY_W))
        m_camera.move(Camera::Direction::Front);
    if (m_input.getKey(GLFW_KEY_S))
        m_camera.move(Camera::Direction::Back);
    if (m_input.getKey(GLFW_KEY_A))
        m_camera.move(Camera::Direction::Left);
    if (m_input.getKey(GLFW_KEY_D))
        m_camera.move(Camera::Direction::Right);
    if (m_input.getKey(GLFW_KEY_SPACE))
        m_camera.move(Camera::Direction::Up);
    if (m_input.getKey(GLFW_KEY_LEFT_SHIFT))
        m_camera.move(Camera::Direction::Down);
    if (m_input.getKey(GLFW_KEY_ESCAPE))
        close();

    if (m_input.getButton(GLFW_MOUSE_BUTTON_MIDDLE))
        if (auto delta = m_input.getCursorDelta(); delta) {
            float sensitivity = 0.1f;
            auto xoffset = -delta->x * sensitivity;
            auto yoffset = delta->y * sensitivity;

            m_yaw += xoffset;
            m_pitch += yoffset;

            m_camera.rotate(m_yaw, m_pitch);
        }

    m_camera.update();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    if (io.WantCaptureMouse || glfwGetInputMode(m_window.get(), GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        return;

    if (m_input.getButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
        m_registry.clear<Picked>();

    if (!m_input.getButtonDown(GLFW_MOUSE_BUTTON_LEFT))
        return;

    m_registry.clear<Picked>();

    double xpos, ypos;
    glfwGetCursorPos(m_window.get(), &xpos, &ypos);
    float mouseX = xpos;
    float mouseY = ypos;

    int windowWidth, windowHeight;
    glfwGetWindowSize(m_window.get(), &windowWidth, &windowHeight);

    struct Ray {
        glm::vec3 origin; // точка начала
        glm::vec3 direction; // нормализованное направление
    };

    auto rayIntersectsAABB = [](const Ray& ray, const BoundingBox& aabb, float& tNear, float& tFar) {
        tNear = -INFINITY;
        tFar = +INFINITY;

        for (int i = 0; i < 3; i++) {
            if (ray.direction[i] != 0.0f) {
                float t1 = (aabb.min[i] - ray.origin[i]) / ray.direction[i];
                float t2 = (aabb.max[i] - ray.origin[i]) / ray.direction[i];

                if (t1 > t2)
                    std::swap(t1, t2);

                tNear = std::max(tNear, t1);
                tFar = std::min(tFar, t2);

                if (tNear > tFar)
                    return false;
                if (tFar < 0)
                    return false;
            } else {
                // Луч параллелен плоскости
                if (ray.origin[i] < aabb.min[i] || ray.origin[i] > aabb.max[i])
                    return false;
            }
        }

        return true;
    };

    glm::vec2 mouseNDC = { // NDC = [-1,1]
        (2.0f * mouseX / windowWidth) - 1.0f, 1.0f - (2.0f * mouseY / windowHeight)
    };

    auto projMatrix = glm::perspective(glm::radians(60.0f),
        (float)m_windowSize.x / m_windowSize.y,
        0.1f, 100.0f);
    auto& viewMatrix = m_camera.getView();

    glm::vec4 clip = { mouseNDC.x, mouseNDC.y, -1.0f, 1.0f };
    glm::vec4 eye = glm::inverse(projMatrix) * clip;
    eye.z = -1.0f;
    eye.w = 0.0f;

    glm::vec3 worldDir = glm::normalize(glm::vec3(glm::inverse(viewMatrix) * eye));

    Ray ray { m_camera.getPos(), worldDir };

    auto view = m_registry.view<BoundingBox, Transform>();
    auto closest = +INFINITY;
    std::optional<entt::entity> picked;
    for (auto [entity, localAABB, transform] : view.each()) {
        auto aabb = toGlobalAABB(localAABB, transform);
        float tNear, tFar;
        if (rayIntersectsAABB(ray, aabb, tNear, tFar)) {
            if (tNear < closest) {
                closest = tNear;
                picked = entity;
            }
        }
    }
    if (picked)
        m_registry.emplace_or_replace<Picked>(picked.value());
}
