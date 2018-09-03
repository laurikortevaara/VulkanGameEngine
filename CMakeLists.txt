cmake_minimum_required (VERSION 3.11)
project (VulkanGameEngine)

# Options. Turn on with 'cmake -Dmyvarname=ON'.

# Option for creating unit tests
option(VULKAN_GAME_ENGINE_UNIT_TESTS "Build all tests." ON) # Makes boolean 'test' available.
# Option for creating doxygen documentation
option(BUILD_DOC "Build documentation" ON)

#Set the c++ lastest (c++17) as the language preference
message(STATUS "Setting MSVC flags")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHc /std:c++latest")

#If using MSVC undef the min, max functions which collide with the STL functions
add_definitions(-DNOMINMAX)

#Set the binary directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

#Set the project version
set(VulkanGameEngine_VERSION_MAJOR 1)
set(VulkanGameEngine_VESRION_MINOR 0)


##
# Download and unpack glm at configure time
configure_file(${CMAKE_SOURCE_DIR}/thirdparty/glm/CMakeLists.txt.in glm-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/glm-download )
if(result)
  message(FATAL_ERROR "CMake step for GLM failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/glm-download )
if(result)
  message(FATAL_ERROR "Build step for GLM failed: ${result}")
endif()

# Add glm directly to our build. 
add_subdirectory(${CMAKE_BINARY_DIR}/glm-src
                 ${CMAKE_BINARY_DIR}/glm-build
                 EXCLUDE_FROM_ALL)
# GLM Included
##

##
# Download and unpack GLFW at configure time
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")

configure_file(${CMAKE_SOURCE_DIR}/thirdparty/glfw/CMakeLists.txt.in glfw-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/glfw-download )
if(result)
  message(FATAL_ERROR "CMake step for glfw failed: ${result}")
endif()

# Download the glfw
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/glfw-download )
if(result)
  message(FATAL_ERROR "Download step for glfw failed: ${result}")
endif()

# Build the glfw
#execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
#  RESULT_VARIABLE result
#  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/glfw-src )
#if(result)
#  message(FATAL_ERROR "CMake step for glfw failed: ${result}")
#endif()

#message("The GLFW build directory is: " ${CMAKE_BINARY_DIR}/glfw-src)
#execute_process(COMMAND ${CMAKE_COMMAND} --build . RESULT_VARIABLE result WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/glfw-src )
#if(result)
#  message(FATAL_ERROR "Build step for glfw failed: ${result}")
#endif()

# Add glfw directly to our build. 
#add_subdirectory(${CMAKE_BINARY_DIR}/glfw-src
#                 ${CMAKE_BINARY_DIR}/glfw-build
#                 EXCLUDE_FROM_ALL)
#add_subdirectory(${CMAKE_BINARY_DIR}/glfw-build
#				 EXCLUDE_FROM_ALL)
# glfw Included
##
subdirs(${CMAKE_BINARY_DIR}/glfw-src)

#Configure the source and header directories
subdirs(test)
subdirs(source)

# Add Google Test as sub project
#add_subdirectory(${VulkanGameEngine_SOURCE_DIR}/thirdparty/googletest)
#enable_testing()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanGameEngine)