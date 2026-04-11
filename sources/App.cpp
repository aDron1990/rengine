#include "App.hpp"

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

#include <GLFW/glfw3.h>
#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "BoundingBox.hpp"
#include "Events.hpp"
#include "Input.hpp"
#include "RenderTexture.hpp"
#include "Texture.hpp"
#include "components/Body.hpp"
#include "components/Camera.hpp"
#include "components/Celestial.hpp"
#include "components/OrbitalBody.hpp"
#include "components/Renderer.hpp"
#include "components/Transform.hpp"
#include "objects/FlyingCamera.hpp"
#include "objects/ModelObject.hpp"
#include "systems/Clock.hpp"
#include "systems/OrbitalEngine.hpp"
#include "systems/PhysicsEngine.hpp"
#include "systems/RendererSystem.hpp"

#include <cstdlib>
#include <memory>
#include <stdexcept>

float random(float min, float max)
{
    return min + (float)rand() / RAND_MAX * (max - min);
}

App::App(int windowWidth, int windowHeight, const std::string& windowTitle)
    : m_registry { }
    , m_windowSize { windowWidth, windowHeight }
{
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
#if 0 // fullscreen
    m_windowSize = { mode->width, mode->height };
    m_window = GlfwWindowPtr(glfwCreateWindow(m_windowSize.x, m_windowSize.y, windowTitle.c_str(), monitor, nullptr));
#else
    m_window = GlfwWindowPtr(glfwCreateWindow(m_windowSize.x, m_windowSize.y, windowTitle.c_str(), nullptr, nullptr));
#endif
    if (m_window == nullptr) {
        throw std::runtime_error { "Failed to create GLFW window" };
    }

    glfwSetWindowUserPointer(m_window.get(), this);

    glfwMakeContextCurrent(m_window.get());
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error { "Failed to init GLEW" };
    }

    glEnable(GL_DEPTH_TEST);

    auto& input = m_registry.ctx().emplace<Input>();
    input.setGlfwWindow(m_window.get());
    m_registry.ctx().emplace<Clock>();

    glfwSetWindowCloseCallback(m_window.get(), windowCloseCallback);
    glfwSetFramebufferSizeCallback(m_window.get(), framebufferSizeCallback);
}

void App::run()
{
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory { };
    JPH::RegisterTypes();
    JPH::TempAllocatorImpl tempAllocator { 10 * 4096 * 4096 };
    JPH::JobSystemThreadPool jobSystem {
        JPH::cMaxPhysicsJobs,
        JPH::cMaxPhysicsBarriers,
        (int)std::thread::hardware_concurrency()
    };

    auto& input = m_registry.ctx().get<Input>();
    auto& physics = m_registry.ctx().emplace<PhysicsEngine>(m_registry, tempAllocator, jobSystem);
    auto& orbital = m_registry.ctx().emplace<OrbiralEngine>(m_registry);

    FlyingCamera _cam = { m_registry, glm::vec3 { 0.0f, 0.0f, 30.0f } };
    auto& flyCamera = m_registry.ctx().emplace<std::reference_wrapper<FlyingCamera>>(_cam).get();

    auto renderTex = m_registry.ctx().emplace<std::shared_ptr<RenderTexture>>(std::make_shared<RenderTexture>(glm::ivec2 { 500, 300 }));

    RendererSystem renderer { m_registry };

    auto xzModel = std::make_shared<Model>("resources/models/xz.fbx");
    auto cubeModel = std::make_shared<Model>("resources/models/cube.obj");

    auto xzTexture = std::make_shared<Texture>("resources/images/xz.png");
    auto whiteTexture = std::make_shared<Texture>("resources/images/white.png");

    ModelObject xz { m_registry, xzModel, xzTexture, whiteTexture };
    ModelObject cube { m_registry, cubeModel, whiteTexture, whiteTexture };

    xz.position() = { -20.0f, 0.0f, 0.0f };
    xz.addComponent(OrbitalBody { .velicity = {0.0f, 0.0f, 4.0f}});
    xz.addComponent(Picked { });

    cube.addComponent(Celestial { 1000.0f });

    physics.createCollider(xz.getEntity(), true);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(m_window.get(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (m_running) {
        updateWindow();
        m_registry.ctx().get<Clock>().update();
        // physics.update();
        orbital.update();
        flyCamera.update();

        auto cameraEntity = m_registry.view<Camera, Transform>().front();
        auto [camera, cameraTransform] = m_registry.get<Camera, Transform>(cameraEntity);
        auto view = camera.getView(cameraTransform.position);

        processInput(view, cameraTransform.position);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::Begin("Camera");
        ImGui::DragFloat3("position", glm::value_ptr(cameraTransform.position), 0.025f);
        ImGui::DragFloat3("front", glm::value_ptr(camera.front), 0.025f);
        ImGui::DragFloat2("near/far", &camera.near, 0.025f);
        ImGui::DragFloat("fov", &camera.fov, 0.1f);
        ImGui::Image(renderTex->getId(), { 300, 180 }, { 0, 1 }, { 1, 0 });
        ImGui::End();

        auto proj = camera.getProj((float)m_windowSize.x / m_windowSize.y);

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

            if (m_registry.all_of<Body>(entity)) {
                ImGui::SeparatorText("Body");
                auto velocity = physics.getVelocity(entity);
                ImGui::BeginDisabled(true);
                ImGui::InputFloat3("velocity", glm::value_ptr(velocity));
                ImGui::EndDisabled();
                static glm::vec3 impulse { };
                ImGui::DragFloat3("impulse", glm::value_ptr(impulse), 1.0f);
                if (ImGui::Button("Apply impulse")) {
                    physics.addImpulse(entity, impulse);
                }
            }

            ImGui::End();

            physics.applyTransform(entity);
        }

#if 1
        {
            auto size = renderTex->getSize();
            auto aspect = renderTex->getAspect();
            auto proj = camera.getProj(aspect);
            renderTex->bindFBO();
            glViewport(0, 0, size.x, size.y);
            renderer.render(proj);
            renderTex->unbindFBO();
        }
#endif

        glViewport(0, 0, m_windowSize.x, m_windowSize.y);
        renderer.render(proj);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        input.update();
        glfwSwapBuffers(m_window.get());
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void App::updateWindow() noexcept { glfwPollEvents(); }

void App::close() noexcept { m_running = false; }

void App::processInput(const glm::mat4& viewMatrix, const glm::vec3& cameraPos) noexcept
{
    auto& input = m_registry.ctx().get<Input>();
    if (input.getKey(GLFW_KEY_ESCAPE))
        close();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    if (io.WantCaptureMouse || glfwGetInputMode(m_window.get(), GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        return;

    if (input.getButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
        m_registry.clear<Picked>();

    if (!input.getButtonDown(GLFW_MOUSE_BUTTON_LEFT))
        return;

    m_registry.clear<Picked>();

    auto pos = input.getCursorPosition();
    float mouseX = pos.x;
    float mouseY = pos.y;

    int windowWidth, windowHeight;
    glfwGetWindowSize(m_window.get(), &windowWidth, &windowHeight);
    glm::vec2 mouseNDC = {
        (2.0f * mouseX / windowWidth) - 1.0f, 1.0f - (2.0f * mouseY / windowHeight)
    };
    auto& camera = m_registry.ctx().get<std::reference_wrapper<FlyingCamera>>().get().getComponent<Camera>();
    auto proj = camera.getProj((float)m_windowSize.x / m_windowSize.y);

    glm::vec4 clip = { mouseNDC.x, mouseNDC.y, -1.0f, 1.0f };
    glm::vec4 eye = glm::inverse(proj) * clip;
    eye.z = -1.0f;
    eye.w = 0.0f;
    glm::vec3 worldDir = glm::normalize(glm::vec3(glm::inverse(viewMatrix) * eye));

    Ray ray { cameraPos, worldDir };
    auto& physics = m_registry.ctx().get<PhysicsEngine>();
    auto picked = physics.pick(ray);
    if (picked)
        m_registry.emplace_or_replace<Picked>(picked.value());
}

void App::windowCloseCallback(GLFWwindow* window) noexcept
{
    auto& app = *static_cast<App*>(glfwGetWindowUserPointer(window));
    app.close();
    app.m_dispatcher.trigger<Event::WindowClose>();
}

void App::framebufferSizeCallback(GLFWwindow* window, int width, int height) noexcept
{
    if (width == 0 || height == 0)
        return;
    auto& app = *static_cast<App*>(glfwGetWindowUserPointer(window));
    app.m_windowSize = { width, height };
    app.m_dispatcher.trigger<Event::WindowResize>({ { app.m_windowSize } });
}