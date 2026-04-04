#include "App.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

#include "Mesh.hpp"
#include "Object.hpp"
#include "Texture.hpp"
#include "components/Renderer.hpp"
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
            std::cout << showCursor << std::endl;
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
