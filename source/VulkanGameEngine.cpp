#include "VulkanGameEngine.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>

#include <iostream>
#include <vector>
#include <set>
#include <Windows.h>

/// VulkanGameEngine Constructor
VulkanGameEngine::VulkanGameEngine()
{
	std::cout << "Vulkan Game Engine instance created" << std::endl;
	int res = glfwInit();
	if (res != GLFW_TRUE)
		throw(std::runtime_error("Could not initialize GLFW!"));
}

/// VulkanGameEngine::findQueueFamilies
/// \brief It'll return you the queue families
/// Detailed documentation for the function goes here...
QueueFamilyIndices VulkanGameEngine::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}

/// Checks whether required device extensions are supported
bool VulkanGameEngine::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	// Query extension count
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	// Query extensions
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

/// Validates a Vulkan physical Devices
bool VulkanGameEngine::isSuitableDevice(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProps;
	vkGetPhysicalDeviceProperties(device, &deviceProps);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	if (indices.isComplete() && extensionsSupported)
		return true;

	// Check if device has a GPU And has Geometry Shaders
	return (deviceProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		deviceFeatures.geometryShader);
}

/// VulkanGameEngine initWindow
/// Creates a GLFW window
void VulkanGameEngine::initWindow()
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan window", nullptr, nullptr);

}

/// VulkanGameEngine pick physical device will 
/// enumerate availabe Vulkan devices (graphics cards)
/// and pick one of them.
void VulkanGameEngine::pickPhysicalDevice()
{
	VkPhysicalDevice device = VK_NULL_HANDLE;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("No Vulkan compatible graphics cards found! Aborting!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (isSuitableDevice(device))
		{
			physicalDevice = device;
			break;
		}
	}
	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

/// VulkanGameEngine::createLogicalDevice()
/// Will create the logical device for the Engine
///
void VulkanGameEngine::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
	queueCreateInfo.queueCount = 1;

	// Priority between 0.0 - 1.0 [REQUIRED]
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	// Create Queue Create Infos
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

	//float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// No need to define any features at this point
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	

	createInfo.pEnabledFeatures = &deviceFeatures;

	// No extensions enabled at this point
	createInfo.enabledExtensionCount = 0;

	// No validation layers enabled at this point
	createInfo.enabledLayerCount = 0;

	// Now we're ready to create the logical device
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Vulkan logical device!");
	}

	vkGetDeviceQueue(logicalDevice, indices.presentFamily, 0, &presentQueue);
}

/// VulkanGameEngine initVulkan
/// Initializes the Vulkan API
void VulkanGameEngine::initVulkan()
{
	createVulkanInstance();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	
}

/// VulkanGameEngine createSurface
/// Creates the VKSurfaceKHR (KHR=Khronos)
void VulkanGameEngine::createSurface()
{
	/* Example surface creation on Windows
	*
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = glfwGetWin32Window(window);
	createInfo.hinstance = GetModuleHandle(nullptr);

	auto CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");

	if (!CreateWin32SurfaceKHR || CreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create win32 window surface!");
	}
	*/

	// Instead of using a custom surface creation function, let's use the
	// glfw's crossplatform approach
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

/// Check for Validation layer support
/// [NOT IMPLEMENTED]
bool VulkanGameEngine::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	return false;
}

/// Creates the Vulkan API Instance
void VulkanGameEngine::createVulkanInstance()
{
	// Initialize VkApplicationInfo
	VkApplicationInfo Info = {};
	Info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	Info.pApplicationName = "VulkanGameEngine";
	Info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	Info.pEngineName = "Omen";
	Info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	Info.apiVersion = VK_API_VERSION_1_0;

	// Get GLFW extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// Initialize VkInstanceCreateInfo
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &Info;
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Vulkan Instance");
	}

	// Get information regarding the Vulkan Extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "available extensions:" << std::endl;

	for (const auto& extension : extensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	checkValidationLayerSupport();
}

/// VulkanGameEngine initialization
void VulkanGameEngine::init()
{
	initWindow();
	initVulkan();
}

/// VulkanGameEngine main loop runner
void VulkanGameEngine::run()
{
	init();
	mainLoop();
	cleanup();
}

/// VulkanGameEngine main loop
void VulkanGameEngine::mainLoop()
{
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

/// VulkanGameEngine cleaning up code
void VulkanGameEngine::cleanup()
{
	vkDestroyDevice(logicalDevice, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}


/// Simple google unit test method 
bool VulkanGameEngine::lessThanTen(int lessThanTen)
{
	return lessThanTen < 10;
}