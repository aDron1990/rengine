#pragma once

#include "utils.hpp"

#include <string>

class App {
public:
	App(int windowWidth, int windowHeight, const std::string& windowTitle);
	void run();

private:
	void updateWindow() noexcept;
	void processInput() noexcept;
	void render() noexcept;

private:
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	App(App&&) = delete;
	App& operator=(App&&) = delete;

private:
	GlfwContext m_glfwContext;
	GlfwWindowPtr m_window;

};

