# Get all subdirectories under the 'tests' directory
file(GLOB subdirectories LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/*")

# Iterate over each subdirectory
foreach(subdir ${subdirectories})
	if(IS_DIRECTORY ${subdir})
		message(note ${subdir})
		# Get the name of the current subdirectory
		get_filename_component(subdir_name ${subdir} NAME)

		# Add an executable for the tests in the current subdirectory

		# Add the source files from the current subdirectory to the executable
		file(GLOB_RECURSE test_sources ${subdir}/*.cpp)
		add_executable(${subdir_name}_tests ${test_sources})
		# Link your project's source files
		target_link_libraries(${subdir_name}_tests PRIVATE Pixie CONAN_PKG::gtest)
		add_test(NAME ${subdir_name} COMMAND ${subdir_name}_tests)
	endif()
endforeach()
