#ifndef VGE_SWAP_CHAIN_SUPPORT_DETAILS
#define VGE_SWAP_CHAIN_SUPPORT_DETAILS

#include <vulkan/vulkan.hpp>

namespace vge
{
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
}

#endif // ~VGE_SWAP_CHAIN_SUPPORT_DETAILS