#include "VulkanRenderBackend.hpp"

VulkanRenderBackend::VulkanRenderBackend(GLFWwindow* window)
{
    vkb::InstanceBuilder instanceBuilder { };
    m_instance = vkb::InstanceBuilder { }
                     .require_api_version(1, 3)
                     .build()
                     .value();

    glfwCreateWindowSurface(m_instance.instance, window, nullptr, &m_surface);
    m_gpu = vkb::PhysicalDeviceSelector { m_instance }
                .set_surface(m_surface)
                .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
                .set_minimum_version(1, 3)
                .select()
                .value();

    m_device = vkb::DeviceBuilder { m_gpu }
                   .build()
                   .value();

    m_queue = m_device
                  .get_queue(vkb::QueueType::graphics)
                  .value();

    m_swapchain = vkb::SwapchainBuilder { m_device }
                      .build()
                      .value();
}

VulkanRenderBackend::~VulkanRenderBackend()
{
    vkb::destroy_swapchain(m_swapchain);
    vkb::destroy_device(m_device);
    vkb::destroy_surface(m_instance, m_surface);
    vkb::destroy_instance(m_instance);
}

