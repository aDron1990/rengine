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
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "BoundingBox.hpp"
#include "Input.hpp"
#include "Mesh.hpp"
#include "RenderTexture.hpp"
#include "Texture.hpp"
#include "components/Body.hpp"
#include "components/Camera.hpp"
#include "components/Renderer.hpp"
#include "components/Transform.hpp"
#include "objects/FlyingCamera.hpp"
#include "objects/Model.hpp"
#include "systems/Clock.hpp"
#include "systems/PhysicsEngine.hpp"
#include "systems/RendererSystem.hpp"

#include <cstdlib>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

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
    m_windowSize = { mode->width, mode->height };
    m_window = GlfwWindowPtr(glfwCreateWindow(
        mode->width, mode->height, windowTitle.c_str(), monitor, nullptr));
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
}

static bool showCursor = false;
static float fov = 60.0f;
static float n_f[2] = { 0.1f, 100.0f };
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

    FlyingCamera _cam = { m_registry, glm::vec3 { 10.0f, 10.0f, 10.0f } };
    auto& flyCamera = m_registry.ctx().emplace<std::reference_wrapper<FlyingCamera>>(_cam).get();

    auto renderTex = m_registry.ctx().emplace<std::shared_ptr<RenderTexture>>(std::make_shared<RenderTexture>(glm::ivec2 { 500, 300 }));

    RendererSystem renderer { m_registry };

    auto& shader = renderer.getShader();

    auto planeMesh = std::make_shared<Mesh>("resources/models/plane.obj");
    auto obMesh = std::make_shared<Mesh>("resources/models/ob.obj");
    auto cubeMesh = std::make_shared<Mesh>("resources/models/cube.obj");

    auto whiteTexture = std::make_shared<Texture>("resources/images/white.png");
    auto floorTexture = std::make_shared<Texture>("resources/images/floor.jpg");
    auto containerTexture = std::make_shared<Texture>("resources/images/container2.png");
    auto containerSpecularTexture = std::make_shared<Texture>("resources/images/container2_specular.png");
    auto windowTexture = std::make_shared<Texture>("resources/images/window.png");

    Model floor0 { m_registry, cubeMesh, floorTexture, whiteTexture };
    Model floor1 { m_registry, cubeMesh, floorTexture, whiteTexture };
    Model floor2 { m_registry, cubeMesh, floorTexture, whiteTexture };
    Model floor3 { m_registry, cubeMesh, floorTexture, whiteTexture };
    Model floor { m_registry, cubeMesh, floorTexture, whiteTexture };

    floor0.position() = { 0.0f, 0.0f, -5.0f };
    floor1.position() = { 0.0f, 0.0f, 5.0f };
    floor2.position() = { 5.0f, 0.0f, 0.0f };
    floor3.position() = { -5.0f, 0.0f, 0.0f };

    floor0.scale() = { 20.0f, .2f, 20.0f };
    floor1.scale() = { 20.0f, .2f, 20.0f };
    floor2.scale() = { 20.0f, .2f, 20.0f };
    floor3.scale() = { 20.0f, .2f, 20.0f };

    floor0.rotation() = { 45.0f, 0.f, 0.f };
    floor1.rotation() = { -45.0f, 0.f, 0.f };
    floor2.rotation() = { 0.f, 0.f, 45.f };
    floor3.rotation() = { 0.f, 0.f, -45.f };

    floor.position() = { 0.0f, -4.0f, 0.0f };

    floor.scale() = { 5.0f, .2f, 5.0f };

    physics.createCollider(floor.getEntity(), false);
    physics.createCollider(floor0.getEntity(), false);
    physics.createCollider(floor1.getEntity(), false);
    physics.createCollider(floor2.getEntity(), false);
    physics.createCollider(floor3.getEntity(), false);

    constexpr auto COUNT = 100;
    std::vector<Model> cubes;
    cubes.reserve(COUNT);
    for (int i = 0; i < COUNT; i++) {
        auto& cube = cubes.emplace_back(m_registry, cubeMesh, containerTexture, containerSpecularTexture);
        cube.position() = { random(-5.0, 5.0), random(25.0, 200.0), random(-5.0, 5.0) };
        physics.createCollider(cube.getEntity());
    }

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
        physics.update();
        flyCamera.update();

        auto cameraEntity = m_registry.view<Camera, Transform>().front();
        auto [camera, cameraTransform] = m_registry.get<Camera, Transform>(cameraEntity);
        auto view = camera.getView(cameraTransform.position);

        processInput(view, cameraTransform.position);

        if (input.getKeyDown(GLFW_KEY_Q)) {
            showCursor = !showCursor;
            glfwSetInputMode(m_window.get(), GLFW_CURSOR, showCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();

        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::Begin("Camera");
        ImGui::DragFloat3("position", glm::value_ptr(cameraTransform.position), 0.025f);
        ImGui::DragFloat3("front", glm::value_ptr(camera.front), 0.025f);
        ImGui::DragFloat2("near/far", n_f, 0.025f);
        ImGui::DragFloat("fov", &camera.fov, 0.1f);
        ImGui::Image(renderTex->getId(), { 500, 300 }, { 0, 1 }, { 1, 0 });
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
