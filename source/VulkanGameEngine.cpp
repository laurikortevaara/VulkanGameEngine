#include "VulkanGameEngine.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_win32.h>

#include <iostream>
#include <set>
#include <algorithm>

#include <fstream>

#include <Windows.h>

static std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file: " + filename);
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

namespace vge
{

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func != nullptr) {
			func(instance, callback, pAllocator);
		}
	}


	/// VulkanGameEngine Constructor
	VulkanGameEngine::VulkanGameEngine() :
		window(nullptr),
		instance(VK_NULL_HANDLE),
		physicalDevice(VK_NULL_HANDLE),
		logicalDevice(VK_NULL_HANDLE),
		graphicsQueue(VK_NULL_HANDLE),
		presentQueue(VK_NULL_HANDLE),
		surface(VK_NULL_HANDLE),
		swapChain(VK_NULL_HANDLE),
		swapChainImages(0),
		swapChainExtent({0,0}),
		swapChainImageViews(0),
		renderPass(VK_NULL_HANDLE),
		pipelineLayout(VK_NULL_HANDLE),
		graphicsPipeline(VK_NULL_HANDLE)
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
		setupDebugCallback();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createSwapChainImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createCommandPool();
		createCommandBuffers();
	}

	/// Creates the swap chain image views
	void VulkanGameEngine::createSwapChainImageViews()
	{
		// As many views as there are images
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); ++i)
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];

			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create image view!");
			}
		}
	}

	void VulkanGameEngine::setupDebugCallback() 
	{
		if (!enableValidationLayers) return;

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		createInfo.pfnCallback = debugCallback;

		if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug callback!");
		}
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

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}

		return extensions;
	}

	/// Creates the Vulkan API Instance
	void VulkanGameEngine::createVulkanInstance()
	{
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
	}

	///
	VkShaderModule VulkanGameEngine::createShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module!");
		}
		return shaderModule;
	}

	/// Creates the render pass
	void VulkanGameEngine::createRenderPass()
	{
		/// Render Pass definition
		VkAttachmentDescription colorAttachment = {};

		colorAttachment.format = swapChainImageFormat;
		// Only one sample 
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		// Clear on start of the render pass
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		// Keep framebuffer when done
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		// No stencil
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		// Layout of the image is not known when the render pass begins
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// When render pass is done, present the image on display
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// Sub-renderpass definition
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1; // refers to layout(location = 0) out vec4 outColor in fragment shader!
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create render pass!");
		}
	}

	/// Creates the graphics pipeline
	void VulkanGameEngine::createGraphicsPipeline()
	{
		// Creating shader modules
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");

		/// Shader modules
		VkShaderModule		vertShaderModule;
		VkShaderModule		fragShaderModule;

		vertShaderModule = createShaderModule(vertShaderCode);
		fragShaderModule = createShaderModule(fragShaderCode);

		// Shader stage creation
		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// Vertex input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		// Input assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Viewport and scissors
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChainExtent.width;
		viewport.height = (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		// Multisampling
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		// Color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		bool blendEnabled = false;
		if (blendEnabled)
		{
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		}
		else
		{
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		// Finally, create the graphics pipeline, based on the information given above
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		// We're not deriving from an existing pipeline, so the next values will be set invalid
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		
		if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE /*No graphics pipeline cache*/, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline!");
		}
		vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
	}


	/// VulkanGameEngine create frame buffers
	void VulkanGameEngine::createFramebuffers()
	{
		swapChainFramebuffers.resize(swapChainImageViews.size());

		for (size_t i=0; i < swapChainImageViews.size(); ++i)
		{
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create a framebuffer!");
			}
		}
	}

	/// VulkanGameEngine create command pool
	void VulkanGameEngine::createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		poolInfo.flags = 0; // Optional

		if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool!");
		}
	} 

	/// VulkanGameEngine create command buffers
	void VulkanGameEngine::createCommandBuffers()
	{
		commandBuffers.resize(swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

		if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers!");
		}
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
		// Destroy command pool
		vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

		// Destroy framebuffers
		for (auto framebuffer : swapChainFramebuffers)
		{
			vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
		}
		
		// Destroy the pipeline
		vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
		
		// Destroy the pipeline layout
		vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
		
		// Destroy the render pass
		vkDestroyRenderPass(logicalDevice, renderPass, nullptr);

		// Destroy the swap chain image views
		for (auto imageView : swapChainImageViews)
		{
			vkDestroyImageView(logicalDevice, imageView, nullptr);
		}

		// Destroy the swap chain
		vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);

		// Destroy the debug report callback
		if (enableValidationLayers)
		{
			DestroyDebugReportCallbackEXT(instance, callback, nullptr);
		}

		// Destroy the logical device
		vkDestroyDevice(logicalDevice, nullptr);

		// Destroy the surface
		vkDestroySurfaceKHR(instance, surface, nullptr);

		// Destroy the Vulkan instance
		vkDestroyInstance(instance, nullptr);

		// Destroy the GLFW window
		glfwDestroyWindow(window);

		// Terminate the App
		glfwTerminate();
	}


	/// Simple google unit test method 
	bool VulkanGameEngine::lessThanTen(int lessThanTen)
	{
		return lessThanTen < 10;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VulkanGameEngine::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData) {
		std::cerr << "validation layer: " << msg << std::endl;

		return VK_FALSE;
	}

}