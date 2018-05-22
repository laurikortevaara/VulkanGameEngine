cmake_minimum_required (VERSION 3.11)
project (VulkanGameEngine)

#Set the c++ lastest (c++17) as the language preference
message(STATUS "Setting MSVC flags")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHc /std:c++latest")

#Set the binary directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

#Set the project version
set(VulkanGameEngine_VERSION_MAJOR 1)
set(VulkanGameEngine_VESRION_MINOR 0)

#Configure the source and header directories
subdirs(source)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanGameEngine)
