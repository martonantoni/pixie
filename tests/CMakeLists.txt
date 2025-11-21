# Get all subdirectories under the 'tests' directory
file(GLOB subdirectories LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/*")

# Iterate over each subdirectory
foreach(subdir ${subdirectories})
	if(IS_DIRECTORY ${subdir})
		# Get the name of the current subdirectory
		get_filename_component(subdir_name ${subdir} NAME)

		# Add an executable for the tests in the current subdirectory

		# Add the source files from the current subdirectory to the executable
		file(GLOB_RECURSE test_sources ${subdir}/*.cpp)
		add_executable(${subdir_name}_tests ${test_sources})
		# Link your project's source files
		target_link_libraries(${subdir_name}_tests PRIVATE Pixie GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
		
		
     set(files_list "${subdir}/test_files.txt")

        if(EXISTS ${files_list})
            file(STRINGS ${files_list} files_to_copy)

            foreach(file_rel ${files_to_copy})
                set(src "${subdir}/${file_rel}")

                if(EXISTS "${src}")
                    add_custom_command(
                        TARGET ${subdir_name}_tests POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                "${src}"
                                $<TARGET_FILE_DIR:${subdir_name}_tests>
                        COMMENT "Copying test data file ${src}"
                    )
                else()
                    message(WARNING "Test file listed but not found: ${src}")
                endif()
            endforeach()
        endif()
		
		
		
		add_test(NAME ${subdir_name} COMMAND ${subdir_name}_tests)
	endif()
endforeach()
