################################
# Testing
################################
if (VULKAN_GAME_ENGINE_UNIT_TESTS)

# Download and unpack googletest at configure time
configure_file(${CMAKE_SOURCE_DIR}/thirdparty/googletest/CMakeLists.txt.in ${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)

  if (APPLE)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
    add_definitions(-D__GLIBCXX__)
  endif (APPLE)

  # Include the gtest library. gtest_SOURCE_DIR is available due to
  # 'project(gtest)' above.
  include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
  include_directories(${VulkanGameEngine_SOURCE_DIR}/include)
  
  ##############
  # Unit Tests
  ##############
  add_executable(runUnitTests unittest_VulkanGameEngine.cpp)

  # Standard linking to gtest stuff.
  target_link_libraries(runUnitTests gtest gtest_main)

  # Extra linking for the project.
  # target_link_libraries(runUnitTests project1_lib)

  # This is so you can do 'make test' to see all your tests run, instead of
  # manually running the executable runUnitTests to see those specific tests.
  add_test(NAME that-test-I-made COMMAND runUnitTests)

  # You can also omit NAME and COMMAND. The second argument could be some other
  # test executable.
  add_test(that-other-test-I-made runUnitTests)

  #Add GLM And GLFW to include directories
	include_directories(${CMAKE_BINARY_DIR}/glm-src)
	include_directories(${CMAKE_BINARY_DIR}/glfw-src/include)

	#Link with GLFW
	#target_link_libraries(runUnitTests ${VulkanGameEngine_SOURCE_DIR}/thirdparty/glfw/build/src/Debug/glfw3.lib)
	
	target_link_libraries(runUnitTests VulkanGameEngine)

	# Add the Vulkan SDK
	find_package(VULKAN REQUIRED)
	include_directories(${Vulkan_INCLUDE_DIR})
 
endif()
