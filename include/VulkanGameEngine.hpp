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

		/// Creates the Swap Chain
		void createSwapChain();

		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	private:
		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		const uint32_t WIDTH = 800;
		const uint32_t HEIGHT = 600;

		GLFWwindow*			window;
		VkInstance			instance;
		VkPhysicalDevice	physicalDevice;
		VkDevice			logicalDevice;
		VkQueue				graphicsQueue;
		VkSurfaceKHR		surface;
		VkQueue				presentQueue;

		VkSwapchainKHR		 swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat			 swapChainImageFormat;
		VkExtent2D			 swapChainExtent;
	};

} /// ~ namespace vge