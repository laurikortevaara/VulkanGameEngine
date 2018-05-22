include_directories(${VulkanGameEngine_SOURCE_DIR}/include)
add_executable(VulkanGameEngine main.cpp VulkanGameEngine.cpp)

# Add the Vulkan SDK
find_package(VULKAN REQUIRED)
MESSAGE( "The vulkan: " ${Vulkan_INCLUDE_DIR})
MESSAGE( "The vulkan: " ${Vulkan_LIBRARY})
include_directories(${Vulkan_INCLUDE_DIR})
target_link_libraries(VulkanGameEngine ${Vulkan_LIBRARY})
