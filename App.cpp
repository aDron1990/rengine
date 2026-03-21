#include "App.hpp"
#include "Shader.hpp"
#include "Buffer.hpp"
#include "VertexArray.hpp"
#include "Texture.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "tiny_obj_loader.h"

#include <iostream>
#include <stdexcept>
#include <vector>

App::App(int windowWidth, int windowHeight, const std::string& windowTitle) {
	m_window = GlfwWindowPtr(glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr));
	if (m_window == nullptr) {
		throw std::runtime_error{ "Failed to create GLFW window" };
	}
	m_input.setGlfwWindow(m_window.get());

	glfwSetWindowUserPointer(m_window.get(), this);
	glfwSetCursorPosCallback(m_window.get(), mouseCallback);
	glfwSetInputMode(m_window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(m_window.get());
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error{ "Failed to init GLEW" };
	}
	
	glViewport(0, 0, windowWidth, windowHeight);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}

void App::run() {
	std::vector<tinyobj::shape_t> shapes; 
	std::vector<tinyobj::material_t> materials; 
	tinyobj::attrib_t attrib; 

	std::string warn; 
	std::string err; 

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "viking_room.obj")) { 
		throw std::runtime_error(err); 
	} 
	std::vector<float> vertices; 
	std::vector<unsigned int> indices; 
	vertices.reserve(attrib.vertices.size() + attrib.texcoords.size()); 
	for (const auto& shape : shapes) { 
		for (const auto& idx : shape.mesh.indices) { 
			vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]); 
			vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]); 
			vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]); 
			if (idx.texcoord_index >= 0) { 
				vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]); 
				vertices.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]); 
			} else { 
				vertices.push_back(0.0f); 
				vertices.push_back(0.0f); 
			} 
		} 
	} 
	VertexArray vao; 
	vao.bind(); 
	
	VertexBuffer vbo{ vertices.data(), vertices.size() * sizeof(float)};
	//IndexBuffer ebo{ indices.data(), indices.size()}; 
	// position attribute 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); 
	glEnableVertexAttribArray(0); 
	// texture coord attribute 
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); 
	glEnableVertexAttribArray(1);

	vao.unbind();

	Texture texture{ "viking_room.png" };

	Shader shader{ "main_v.glsl", "main_f.glsl" };

	while (m_running) {
		updateWindow();
		processInput();
		render();
		
		auto proj = glm::perspective(glm::radians(60.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

		auto view = m_camera.getView();

		static int frame = 0;
		//frame++;
		//auto model = glm::rotate(glm::mat4{ 1.0f }, (float)frame / 100.0f, glm::vec3{ 0.0, 1.0f, 0.0f });

		auto model = glm::rotate(glm::mat4{ 1.0f }, glm::radians(-90.0f), glm::vec3{1.0, 0.0f, 0.0f});
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3{ 0.0, 0.0f, 1.0f });

		shader.use();
		shader.setUniform(model, "model");
		shader.setUniform(view, "view");
		shader.setUniform(proj, "proj");

		texture.bind();
		vao.bind();
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 5);
		//glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(m_window.get());
	}
}

Input& App::getInput() noexcept {
	return m_input;
}

void App::updateWindow() noexcept {
	glfwPollEvents();
}

void App::processInput() noexcept {
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

	m_camera.update();

	if (m_input.getKey(GLFW_KEY_ESCAPE))
		close();
}

void App::render() noexcept {
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void App::close() noexcept {
	m_running = false;
}

void App::mouseCallback(GLFWwindow* window, double xpos, double ypos) noexcept {
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

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	app->m_yaw += xoffset;
	app->m_pitch += yoffset;

	app->m_camera.rotate(app->m_yaw, app->m_pitch);
}
