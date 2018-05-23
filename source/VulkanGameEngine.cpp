#include "VulkanGameEngine.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>

#include <iostream>
#include <set>
#include <algorithm>

#include <Windows.h>

namespace vge 
{

/// VulkanGameEngine Constructor
VulkanGameEngine::VulkanGameEngine() :
	window(nullptr),
	instance(VK_NULL_HANDLE),
	physicalDevice(VK_NULL_HANDLE),
	logicalDevice(VK_NULL_HANDLE),
	graphicsQueue(VK_NULL_HANDLE),
	surface(VK_NULL_HANDLE),
	presentQueue(VK_NULL_HANDLE),
	swapChain(VK_NULL_HANDLE)
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

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

/// Query the swap chain support
SwapChainSupportDetails VulkanGameEngine::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	// Query the surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	// Query the surface formats
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	// Query present modes
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}



	return details;
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
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return (indices.isComplete() && extensionsSupported && swapChainAdequate);
}

/// Chooses the optimal surface format
VkSurfaceFormatKHR VulkanGameEngine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	// If there's only one format available, let's use BGRA-32-bit color, with SRGB color space definition
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	// If there's no preferred format available, let's choose an optimal one
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;

	}

	// If optimal one wasn't found, just return the first available
	return availableFormats[0];
}

/// Chooses the optimal present mode
VkPresentModeKHR VulkanGameEngine::chooseSwapSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	// Look for Mailbox presentmode as it allows triple buffering
	// if not available, return the immediate mode
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
			bestMode = availablePresentMode;

	}
	// Only the FIFO KHR mode is quaranteed to be available
	return bestMode;
}

/// Chooses the swap extent
VkExtent2D VulkanGameEngine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	// Some OS windowing systems will define the extent size to be max value of uint32_t
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return surfaceCapabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { VulkanGameEngine::WIDTH, VulkanGameEngine::HEIGHT };

		actualExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}

}

/// Creates the swap chain
void VulkanGameEngine::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapSurfacePresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1; // Always one unless we're creating stereoscopic view
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	// We dont' want any specific transform (e.g. rotate 90-degrees, or flip horizontally, etc)
	// So we define that we'll use the current transform
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;


	// We don't want to blend with another windows in the windowing system
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	
	// We don't care about clipped pixels, behind other windows in the windowing system
	createInfo.clipped = VK_TRUE;

	// Let's not worry about the old swap chain yet, must be taken care of when resizing window though
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) 
	{
		throw std::runtime_error("Failed to create swap chain!");
	}

	// Get swap chain images
	/*uint32_t*/ imageCount = 0;
	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);

	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
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

	// Initialize the device extensions, which are verified earlier to be supported
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

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
	createSwapChain();
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
	vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
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
 
}