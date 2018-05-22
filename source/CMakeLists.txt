include_directories(${VulkanGameEngine_SOURCE_DIR}/include)
add_library(VulkanGameEngine VulkanGameEngine.cpp)

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


add_executable(VulkanGameEngineTestApp main.cpp)
target_link_libraries(VulkanGameEngineTestApp VulkanGameEngine)

# Documentation by doxygen
# check if Doxygen is installed
find_package(Doxygen REQUIRED)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/../docs/Doxyfile.in)
    set(DOXYGEN_OUT ${VulkanGameEngine_SOURCE_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
