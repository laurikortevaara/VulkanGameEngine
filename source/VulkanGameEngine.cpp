#include "VulkanGameEngine.hpp"

#include <vulkan/vulkan.hpp>

#include <iostream>
#include <vector>

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
			indices.graphicsFamily = i;

		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}

/// Validates a Vulkan physical Devices
bool VulkanGameEngine::isSuitableDevice(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProps;
	vkGetPhysicalDeviceProperties(device, &deviceProps);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = findQueueFamilies(device);

	if (indices.isComplete())
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

	// No need to define any features at this point
	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;

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

	vkGetDeviceQueue(logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
}

/// VulkanGameEngine initVulkan
/// Initializes the Vulkan API
void VulkanGameEngine::initVulkan()
{
	createVulkanInstance();
	pickPhysicalDevice();
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
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}


/// Simple google unit test method 
bool VulkanGameEngine::lessThanTen(int lessThanTen)
{
	return lessThanTen < 10;
}