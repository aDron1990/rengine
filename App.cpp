#include "App.hpp"
#include "Shader.hpp"
#include "Buffer.hpp"
#include "VertexArray.hpp"

#include <iostream>
#include <stdexcept>

App::App(int windowWidth, int windowHeight, const std::string& windowTitle) {
	m_window = GlfwWindowPtr(glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr));
	if (m_window == nullptr) {
		throw std::runtime_error{ "Failed to create GLFW window" };
	}

	glfwMakeContextCurrent(m_window.get());
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error{ "Failed to init GLEW" };
	}
	
	glViewport(0, 0, windowWidth, windowHeight);
}

float vertices[] = {
	 0.5f,  0.5f, 0.0f,  // top right
	 0.5f, -0.5f, 0.0f,  // bottom right
	-0.5f, -0.5f, 0.0f,  // bottom left
	-0.5f,  0.5f, 0.0f   // top left 
};
unsigned int indices[] = {  // note that we start from 0!
	0, 1, 3,   // first triangle
	1, 2, 3    // second triangle
};

void App::run() {
	VertexArray vao;
	vao.bind();

	VertexBuffer vbo{ vertices, sizeof(vertices) };
	IndexBuffer ebo{ indices, sizeof(indices) };

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	vao.unbind();

	Shader shader{ "main_v.glsl", "main_f.glsl" };

	while (!glfwWindowShouldClose(m_window.get())) {
		updateWindow();
		processInput();
		render();
		
		shader.use();
		vao.bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(m_window.get());
	}
}

void App::updateWindow() noexcept {
	glfwPollEvents();
}

void App::processInput() noexcept {

}

void App::render() noexcept {
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}
