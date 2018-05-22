#include "VulkanGameEngine.hpp"

#include <GLFW/glfw3.h>
#include <iostream>

VulkanGameEngine::VulkanGameEngine()
{
	std::cout << "Vulkan Game Engine instance created" << std::endl;
	int res = glfwInit();
	if (res != GLFW_TRUE)
		throw(std::runtime_error("Could not initialize GLFW!"));
}
