#include "QueueFamilyIndices.hpp"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class VulkanGameEngine
{
	public:
		VulkanGameEngine();

		void init();
		void run();
		void cleanup();
protected:

	void initWindow();
	void initVulkan();

	void mainLoop();

	bool checkValidationLayerSupport();
	void createVulkanInstance();
	void pickPhysicalDevice();
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