include_directories(${VulkanGameEngine_SOURCE_DIR}/include)
add_executable(VulkanGameEngine main.cpp VulkanGameEngine.cpp)

#Add GLM And GLFW to include directories
include_directories(${VulkanGameEngine_SOURCE_DIR}/thirdparty/glfw/include)
include_directories(${VulkanGameEngine_SOURCE_DIR}/thirdparty/GLM/include)

#Link with GLFW
target_link_libraries(VulkanGameEngine ${VulkanGameEngine_SOURCE_DIR}/thirdparty/glfw/build/src/Debug/glfw3.lib)

# Add the Vulkan SDK
find_package(VULKAN REQUIRED)
MESSAGE( "The vulkan: " ${Vulkan_INCLUDE_DIR})
MESSAGE( "The vulkan: " ${Vulkan_LIBRARY})
include_directories(${Vulkan_INCLUDE_DIR})
target_link_libraries(VulkanGameEngine ${Vulkan_LIBRARY})
