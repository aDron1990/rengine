#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>
#include <vulkan/vulkan.hpp>

class VulkanRenderBackend {
public:
    VulkanRenderBackend(GLFWwindow* window);
    ~VulkanRenderBackend();

private:
    vkb::Instance m_instance;
    vkb::PhysicalDevice m_gpu;
    vkb::Device m_device;
    vkb::Swapchain m_swapchain;
    VkQueue m_queue;
    VkSurfaceKHR m_surface;
};
