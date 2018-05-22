################################
# Testing
################################
if (test)
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
	include_directories(${VulkanGameEngine_SOURCE_DIR}/thirdparty/glfw/include)
	include_directories(${VulkanGameEngine_SOURCE_DIR}/thirdparty/GLM/include)

	#Link with GLFW
	#target_link_libraries(runUnitTests ${VulkanGameEngine_SOURCE_DIR}/thirdparty/glfw/build/src/Debug/glfw3.lib)
	
	target_link_libraries(runUnitTests VulkanGameEngine)

	# Add the Vulkan SDK
	find_package(VULKAN REQUIRED)
	include_directories(${Vulkan_INCLUDE_DIR})
 
endif()
