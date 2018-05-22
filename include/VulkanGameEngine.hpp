/*! \mainpage My Personal Index Page
*
* \section intro_sec Introduction
*
* This is the introduction.
*
* \section install_sec Installation
*
* \subsection step1 Step 1: Opening the box
*
* etc...
*/

#include "QueueFamilyIndices.hpp"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

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
	
	///
	void createVulkanInstance();
	
	///
	void pickPhysicalDevice();
	
	///
	void createLogicalDevice();

	bool isSuitableDevice(VkPhysicalDevice device);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
private:
	const int WIDTH = 800;
	const int HEIGHT = 600;

	GLFWwindow*			window;
	VkInstance			instance;
	VkPhysicalDevice	physicalDevice;
	VkDevice			logicalDevice;
};