#include "App.hpp"

#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <optional>
#include <stdexcept>

#include "AABB.hpp"
#include "Mesh.hpp"
#include "Object.hpp"
#include "Texture.hpp"
#include "components/Renderer.hpp"
#include "components/Transform.hpp"
#include "systems/RendererSystem.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

App::App(int windowWidth, int windowHeight, const std::string& windowTitle)
    : m_windowSize { windowWidth, windowHeight }
{
    m_window = GlfwWindowPtr(glfwCreateWindow(
        windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr));
    if (m_window == nullptr) {
        throw std::runtime_error { "Failed to create GLFW window" };
    }
    m_input.setGlfwWindow(m_window.get());

    glfwSetWindowUserPointer(m_window.get(), this);
    glfwSetCursorPosCallback(m_window.get(), mouseCallback);
    glfwSetInputMode(m_window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
    RendererSystem renderer { m_registry, m_camera };

    auto& shader = renderer.getShader();

    auto planeMesh = std::make_shared<Mesh>("resources/models/plane.obj");
    auto obMesh = std::make_shared<Mesh>("resources/models/ob.obj");

    auto whiteTexture = std::make_shared<Texture>("resources/images/white.png");
    auto floorTexture = std::make_shared<Texture>("resources/images/floor.jpg");
    auto containerTexture = std::make_shared<Texture>("resources/images/container2.png");
    auto containerSpecularTexture = std::make_shared<Texture>("resources/images/container2_specular.png");
    auto windowTexture = std::make_shared<Texture>("resources/images/window.png");

    Object floor { m_registry, planeMesh, whiteTexture, whiteTexture };
    Object ob { m_registry, obMesh, containerTexture, whiteTexture };
    Object window { m_registry, planeMesh, windowTexture, whiteTexture };
    Object window1 { m_registry, planeMesh, windowTexture, whiteTexture };
    Object window2 { m_registry, planeMesh, windowTexture, whiteTexture };

    floor.scale() *= 2.5f;
    floor.position() += glm::vec3 { 0.0f, -0.2f, 0.0f };

    ob.position() = { 2.0f, 0.3f, 0.0f };
    ob.rotation() = { 0.0f, -90.0f, 0.0f };

    window.addComponent(Transparent { });
    window1.addComponent(Transparent { });
    window2.addComponent(Transparent { });

    window.scale() *= 0.3;
    window1.scale() *= 0.3;
    window2.scale() *= 0.3;

    window.rotation() = glm::vec3 { 0.0f, 0.0f, 90.0f };
    window1.rotation() = glm::vec3 { 0.0f, 0.0f, 90.0f };
    window2.rotation() = glm::vec3 { 0.0f, 0.0f, 90.0f };

    window.position() = { 0.5f, 0.5f, 0.0f };
    window1.position() = { 0.0f, 0.5f, 0.0f };
    window2.position() = { -0.5f, 0.5f, 0.0f };

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui_ImplGlfw_InitForOpenGL(m_window.get(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    std::cout << m_registry.view<AABB>()->size() << std::endl;

    glm::vec3 lightPos { -15.0f, 15.0f, 15.0f };

    while (m_running) {
        updateWindow();

        processInput();

        if (m_input.getKeyDown(GLFW_KEY_Q)) {
            showCursor = !showCursor;
            glfwSetInputMode(m_window.get(), GLFW_CURSOR, showCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Settings");
        ImGui::DragFloat3("lightPos", glm::value_ptr(lightPos));
        ImGui::End();

        ImGui::Render();

        auto proj = glm::perspective(glm::radians(60.0f),
            (float)m_windowSize.x / m_windowSize.y,
            0.1f, 100.0f);
        auto& view = m_camera.getView();

        static int frame = 0;

        frame++;
        ob.rotation() = glm::vec3 { (float)frame, (float)frame * 1.5f, (float)frame * 1.7f } * 0.2f;

        shader.setUniform(lightPos, "light.position");

        renderer.render(view, proj);

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

    m_camera.update();

    static int last_state = 0;
    bool click = false;
    int state = glfwGetMouseButton(m_window.get(), GLFW_MOUSE_BUTTON_LEFT);
    if (last_state != state) {
        if (state == GLFW_PRESS)
            click = true;

        last_state = state;
    }

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    if (!click || io.WantCaptureMouse || glfwGetInputMode(m_window.get(), GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
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

    auto rayIntersectsAABB = [](const Ray& ray, const AABB& aabb, float& tNear, float& tFar) {
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

    auto view = m_registry.view<AABB, Transform>();
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

void App::mouseCallback(GLFWwindow* window, double xpos, double ypos) noexcept
{
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (app->m_firstMouse) {
        app->m_lastX = xpos;
        app->m_lastY = ypos;
        app->m_firstMouse = false;
    }

    float xoffset = xpos - app->m_lastX;
    float yoffset = app->m_lastY - ypos;
    app->m_lastX = xpos;
    app->m_lastY = ypos;

    if (showCursor)
        return;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    app->m_yaw += xoffset;
    app->m_pitch += yoffset;

    app->m_camera.rotate(app->m_yaw, app->m_pitch);
}
