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
#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>
#include <functional>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "BoundingBox.hpp"
#include "Events.hpp"
#include "Input.hpp"
#include "components/Body.hpp"
#include "components/Camera.hpp"
#include "components/Celestial.hpp"
#include "components/LineRenderer.hpp"
#include "components/MeshRenderer.hpp"
#include "components/OrbitalBody.hpp"
#include "components/Transform.hpp"
#include "components/WorldPosition.hpp"
#include "graphics/Image.hpp"
#include "graphics/RenderBackend.hpp"
#include "graphics/RenderLayer.hpp"
#include "objects/ModelObject.hpp"
#include "objects/Navball.hpp"
#include "objects/OrbitCamera.hpp"
#include "objects/TestSatelite.hpp"
#include "systems/Clock.hpp"
#include "systems/OrbitalEngine.hpp"
#include "systems/OriginRebaseSystem.hpp"
#include "systems/PhysicsEngine.hpp"
#include "systems/RenderEngine.hpp"

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
    // glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
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

#if 1
    glfwMakeContextCurrent(m_window.get());
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error { "Failed to init GLEW" };
    }
#endif

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
    auto& originRebase = m_registry.ctx().emplace<OriginRebaseSystem>(m_registry);
    auto& orbital = m_registry.ctx().emplace<OrbiralEngine>(m_registry);

    RenderEngine renderer { m_registry, (uint32_t)m_windowSize.x, (uint32_t)m_windowSize.y, m_window.get() };
    m_registry.ctx().emplace<std::reference_wrapper<RenderEngine>>(renderer);

    auto xzModel = std::make_shared<Model>("resources/models/cursor.fbx");
    auto cubeModel = std::make_shared<Model>("resources/models/cube.obj");
    auto sphereModel = std::make_shared<Model>("resources/models/sphere.fbx");

    auto renderBack = m_registry.ctx().get<std::shared_ptr<RenderBackend>>();
    auto& device = renderBack->getDevice();
    for (auto& mesh : xzModel->getMeshes()) {
        mesh.meshID = device.createMesh(mesh.vertices, mesh.indices);
    }
    for (auto& mesh : cubeModel->getMeshes()) {
        mesh.meshID = device.createMesh(mesh.vertices, mesh.indices);
    }
    for (auto& mesh : sphereModel->getMeshes()) {
        mesh.meshID = device.createMesh(mesh.vertices, mesh.indices);
    }

    auto whiteImage = loadImage("resources/images/white.png");
    auto navballImage = loadImage("resources/images/navball_brownblue.png");
    auto whiteTexture = device.createTexture(whiteImage);
    auto navballTexture = device.createTexture(navballImage);

    ModelObject cube { m_registry, cubeModel, whiteTexture, whiteTexture };
    cube.addComponent(Celestial { 0.000001 });
    cube.addComponent(WorldPosition { { 0.0, 0.0, 0.0 } });
    TestSatelite xz { m_registry, xzModel, whiteTexture, whiteTexture };
    Navball navball { m_registry, sphereModel, navballTexture };

    xz.position() = { -20.0f, 0.0f, 0.0f };
    xz.getComponent<WorldPosition>().positionKm = { -0.02, 0.0, 0.0 };
    originRebase.update();
    xz.getComponent<LineRenderer>().lines = orbital.calcOrbit(xz.getEntity(), m_registry.view<Celestial>().front());
    xz.addComponent(Picked { });

    OrbitCamera cam { m_registry, xz.getEntity() };
    renderer.setRenderLayerCamera(DEFAULT_RENDER_LAYER, cam.getEntity());

    physics.createCollider(cube.getEntity(), false);
    physics.createCollider(xz.getEntity(), true);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(m_window.get(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glm::vec2 scale;
    glfwGetWindowContentScale(m_window.get(), &scale.x, &scale.y);
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(scale.x);

    ImFontConfig cfg;
    auto fontSize = 13.0f * scale.x;

    auto* font = io.Fonts->AddFontFromFileTTF("C:/Users/andre/dev/rengine/resources/fonts/Ubuntu-Bold.ttf",
        fontSize,
        &cfg);
    io.Fonts->Build();
    io.FontDefault = font;

    bool simulateOrbital = false;
    while (m_running) {
        updateWindow();
        m_registry.ctx().get<Clock>().update();
        // physics.update();
        if (simulateOrbital)
            orbital.update();
        originRebase.update();
        for (auto entity : m_registry.view<Body, WorldPosition>()) {
            physics.applyTransform(entity);
        }
        cam.update();
        xz.update();
        navball.update();

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
        ImGui::End();

        {
            ImGui::Begin("Navball");

            ImVec2 size(400, 400);

            // рисуем изображение
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::Image(renderer.getGuiTextureFromLayer(0), size, { 0, 1 }, { 1, 0 });

            // получаем draw list окна
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            // центр изображения
            ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);

            // параметры крестовины
            float xlen = 20.0f;
            float ylen = 10.0f;
            ImU32 color = IM_COL32(255, 255, 255, 255);

            // горизонтальная линия
            draw_list->AddLine(
                ImVec2(center.x - xlen, center.y),
                ImVec2(center.x + xlen, center.y),
                color, 1.5f);

            // вертикальная линия
            draw_list->AddLine(
                ImVec2(center.x, center.y - ylen),
                ImVec2(center.x, center.y + ylen),
                color, 1.5f);

            auto piView = m_registry.view<Picked, Transform, MeshRenderer>();
            if (piView.size_hint() > 0) {
                {
                    auto entity = piView.front();
                    auto [transform, renderer] = m_registry.get<Transform, MeshRenderer>(entity);
                    if (m_registry.all_of<OrbitalBody>(entity)) {
                        auto& body = m_registry.get<OrbitalBody>(entity);
                        glm::vec3 dir = glm::normalize(glm::vec3(body.velocityKmPerSec));
                        dir.x = -dir.x;
                        glm::vec3 indicator = navball.getIndicatorsQuat() * dir;
                        if (indicator.z < 0.0f) {
                            indicator.z = 0;
                            indicator = glm::normalize(indicator);
                        }
                        {
                            float screen_x = (indicator.x * 0.5f + 0.5f);
                            float screen_y = (1.0f - (indicator.y * 0.5f + 0.5f));

                            ImVec2 marker_center = ImVec2(pos.x + size.x * screen_x, pos.y + size.y * screen_y);

                            float marker_size = 12.0f;
                            ImU32 prograde_color = IM_COL32(0, 255, 0, 255);

                            draw_list->AddCircle(marker_center, marker_size, prograde_color, 16, 3.0f);

                            draw_list->AddLine(
                                ImVec2(marker_center.x, marker_center.y - marker_size * 0.5),
                                ImVec2(marker_center.x, marker_center.y - marker_size * 1.2f),
                                prograde_color, 3.0f);

                            draw_list->AddLine(
                                ImVec2(marker_center.x - marker_size * 1.2f, marker_center.y),
                                ImVec2(marker_center.x - marker_size * 0.5f, marker_center.y),
                                prograde_color, 3.0f);

                            draw_list->AddLine(
                                ImVec2(marker_center.x + marker_size * 1.2f, marker_center.y),
                                ImVec2(marker_center.x + marker_size * 0.5f, marker_center.y),
                                prograde_color, 3.0f);
                        }
                    }
                }
                {
                    auto entity = piView.front();
                    auto [transform, renderer] = m_registry.get<Transform, MeshRenderer>(entity);
                    if (m_registry.all_of<OrbitalBody>(entity)) {
                        auto& body = m_registry.get<OrbitalBody>(entity);
                        glm::vec3 dir = glm::normalize(glm::vec3(-body.velocityKmPerSec));
                        dir.x = -dir.x;
                        glm::vec3 indicator = navball.getIndicatorsQuat() * dir;
                        if (indicator.z < 0.0f) {
                            indicator.z = 0;
                            indicator = glm::normalize(indicator);
                        }
                        {
                            float screen_x = (indicator.x * 0.5f + 0.5f);
                            float screen_y = (1.0f - (indicator.y * 0.5f + 0.5f));

                            ImVec2 marker_center = ImVec2(pos.x + size.x * screen_x, pos.y + size.y * screen_y);

                            float marker_size = 12.0f;
                            ImU32 prograde_color = IM_COL32(255, 0, 0, 255);

                            draw_list->AddCircle(marker_center, marker_size, prograde_color, 16, 3.0f);

                            draw_list->AddLine(
                                ImVec2(marker_center.x, marker_center.y - marker_size * 0.5),
                                ImVec2(marker_center.x, marker_center.y - marker_size * 1.2f),
                                prograde_color, 3.0f);

                            draw_list->AddLine(
                                ImVec2(marker_center.x - marker_size * 1.2f, marker_center.y),
                                ImVec2(marker_center.x - marker_size * 0.5f, marker_center.y),
                                prograde_color, 3.0f);

                            draw_list->AddLine(
                                ImVec2(marker_center.x + marker_size * 1.2f, marker_center.y),
                                ImVec2(marker_center.x + marker_size * 0.5f, marker_center.y),
                                prograde_color, 3.0f);
                        }
                    }
                }
            }
            ImGui::End();
        }

        ImGui::Begin("OrbitEngine");
        ImGui::DragScalar("GM", ImGuiDataType_Double, &cube.getComponent<Celestial>().GM, 0.0000001);
        ImGui::Checkbox("Simulate", &simulateOrbital);
        ImGui::End();

        auto proj = camera.getProj((float)m_windowSize.x / m_windowSize.y);

        auto pickedView = m_registry.view<Picked, Transform, MeshRenderer>();
        if (pickedView.size_hint() > 0) {
            auto entity = pickedView.front();
            auto [transform, renderer] = m_registry.get<Transform, MeshRenderer>(entity);

            ImGui::Begin("Inspector");

            ImGui::SeparatorText("Transform");
            bool localPositionChanged = ImGui::DragFloat3("position", glm::value_ptr(transform.position), 0.025f);
            ImGui::DragFloat4("rotation", glm::value_ptr(transform.rotation), 0.025f);
            transform.rotation = glm::normalize(transform.rotation);
            ImGui::DragFloat3("scale", glm::value_ptr(transform.scale), 0.025f);
            if (m_registry.all_of<WorldPosition>(entity)) {
                auto& worldPosition = m_registry.get<WorldPosition>(entity);
                if (localPositionChanged) {
                    worldPosition.positionKm = originRebase.toWorldKm(transform.position);
                }
                ImGui::SeparatorText("World Position");
                if (ImGui::DragScalarN("position km", ImGuiDataType_Double, glm::value_ptr(worldPosition.positionKm), 3, 0.001)) {
                    originRebase.syncTransform(entity);
                }
            }

            ImGui::SeparatorText("Renderer");
            auto l = renderer.layer;
            ImGui::DragInt("render layer", &l);
            if (renderer.layer != l) {
                renderer.layer = l;
            }
            if (m_registry.all_of<LineRenderer>(entity)) {
                auto& renderer = m_registry.get<LineRenderer>(entity);
                ImGui::Checkbox("Draw lines", &renderer.draw);
            }

            if (m_registry.all_of<OrbitalBody>(entity)) {
                auto& body = m_registry.get<OrbitalBody>(entity);
                ImGui::SeparatorText("OrbitalBody");
                ImGui::BeginDisabled(simulateOrbital);
                ImGui::DragScalarN("orbital velocity km/s", ImGuiDataType_Double, glm::value_ptr(body.velocityKmPerSec), 3, 0.00005);
                ImGui::EndDisabled();
                ImGui::BeginDisabled(true);
                auto vel = glm::length(body.velocityKmPerSec);
                ImGui::DragScalar("orbital velocity", ImGuiDataType_Double, &vel, 0.00005);
                ImGui::EndDisabled();
            }

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

        renderer.render();

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

    auto& camera = m_registry.get<Camera>(m_registry.view<Camera>().front());
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
    app.m_registry.ctx().get<std::reference_wrapper<RenderEngine>>().get().resize(width, height);
}
