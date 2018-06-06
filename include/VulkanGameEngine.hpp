/*! \mainpage Vulkan Game Engine
*
* \section intro_sec Introduction
*
* This is an effort to create a game engine based on Vulkan API
*
* \section install_sec Installation
*
* \subsection step1 Setting up the development environment
* Windows:
* - Downlad and install Visual Studio 2017 from https://www.visualstudio.com/downloads/
* - Download and install CMake from https://cmake.org/download/
* - Download and install Vulkan SDK from https://vulkan.lunarg.com/
* - Clone GLFW from https://github.com/glfw/glfw and build 64-bit libraries. Place the repo under VulkanGameEngine/thirdparty/glfw
* - Clone GLM from https://github.com/g-truc/glm. Place the repo under VulkanGameEngine/thirdparty/glm
* - Clone Google Test framework from https://github.com/google/googletest. Place the repo under VulkanGameEngine/thirdparty/googletest
* 
* Optional:
* - Install Doxygen if you wish to read documentation
*
*/

#include "QueueFamilyIndices.hpp"
#include "SwapChainSupportDetails.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace vge
{
	/// \brief The Vulkan Game Engine
	///
	/// This is the main API for Vulkan Game Engine
	/// The implementation is in c++ language
	class VulkanGameEngine
	{
	public:
		// Example unit test method
		bool lessThanTen(int lessThanTen);

	public:
		VulkanGameEngine();

		void init();
		void run();
		void cleanup();
	protected:

		/// initWindow will create a new GLFW window for the engine
		void initWindow();

		/// initVulkan will initialize the Vulkan API
		void initVulkan();

		/// mainLoop will be called for each rendering frame by GLFW
		void mainLoop();

		/// Setup Vulkan debug callback
		void setupDebugCallback();

		/// checkValidationLayerSupport will return the shiit
		bool checkValidationLayerSupport();

		/// Creates the Vulkan API instance
		void createVulkanInstance();

		/// Creates physical Vulkan Device referring to Grpahics card
		void pickPhysicalDevice();

		/// Creates logical Vulkan Device 
		void createLogicalDevice();

		/// Create Surface
		void createSurface();

		/// Checks for supported extensions
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);

		/// Checks whether device is suitable for rendering
		bool isSuitableDevice(VkPhysicalDevice device);

		/// Query the swap chain support
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

		/// Chooses the swap surface format
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		/// Chooses the swap present mode
		VkPresentModeKHR chooseSwapSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		/// Chooses the swap extent
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

		/// Creates the swap chain
		void createSwapChain();

		/// Creates the swap chain image views
		void createSwapChainImageViews();

		/// Creates the render pass
		void createRenderPass();

		/// Creates the graphics pipeline
		void createGraphicsPipeline();

		/// Creates a shader module
		VkShaderModule createShaderModule(const std::vector<char>& code);

		/// Finds the queue families
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

		/// Creates the Frame buffers
		void createFramebuffers();

		/// Creates command pool
		void createCommandPool();

		/// Creates command buffers
		void createCommandBuffers();

		/// Draws a frame
		void drawFrame();

		/// Creates synchronization semaphores
		void createSemaphores();

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
	private:
		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		const uint32_t WIDTH = 800;
		const uint32_t HEIGHT = 600;

		GLFWwindow*			window;
		VkInstance			instance;
		VkDebugReportCallbackEXT callback;

		/// The devices (physical & logical)
		VkPhysicalDevice	physicalDevice;
		VkDevice			logicalDevice;

		/// The queues (graphics & present)
		VkQueue				graphicsQueue;
		VkQueue				presentQueue;

		/// The surface
		VkSurfaceKHR		surface;

		/// The swap chain
		VkSwapchainKHR		 swapChain;
		std::vector<VkImage> swapChainImages;
		std::vector<VkFramebuffer> swapChainFramebuffers;
		VkFormat			 swapChainImageFormat;
		VkExtent2D			 swapChainExtent;

		/// The Image views
		std::vector<VkImageView> swapChainImageViews;

		/// Render Pass
		VkRenderPass		renderPass;
		VkPipelineLayout	pipelineLayout;

		///	Graphics pipeline
		VkPipeline			graphicsPipeline;

		/// Command pool
		VkCommandPool		commandPool;

		/// Command buffers
		std::vector<VkCommandBuffer>	commandBuffers;

		/// Synchronization semaphores
		VkSemaphore			imageAvailableSemaphore;
		VkSemaphore			renderFinishedSemaphore;
	};

	

} /// ~ namespace vge