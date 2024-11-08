cmake_minimum_required(VERSION 3.20)

message(STATUS "toolchain file:" ${CMAKE_TOOLCHAIN_FILE})

set(VCPKG_INSTALL_PATH "${CMAKE_CURRENT_LIST_DIR}/vcpkg_installed/x64-windows/share")
if (NOT EXISTS "${VCPKG_INSTALL_PATH}")
    message(FATAL_ERROR
        "The 'vcpkg_installed' folder was not found in the project directory. "
        "Please run 'vcpkg install' in the project folder to install the required packages."
    )
endif()
list(APPEND CMAKE_PREFIX_PATH "${VCPKG_INSTALL_PATH}")

find_package(directxtk CONFIG REQUIRED)
find_package(freetype REQUIRED)
message(STATUS "Freetype found: ${FREETYPE_LIBRARIES}")

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_FIND_LIBRARY_PREFIXES "")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")

# Set the output directory for built executables and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

project(Pixie VERSION 0.2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC")  # full path in error messages
set(DIRECTX_INCLUDE_PATH "c:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include")
set(DIRECTX_LIB_PATH "c:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x64")

# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                  "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)


conan_cmake_configure(REQUIRES 
                               fmt/10.2.0
							   lua/5.4.4
							   gtest/1.13.0
                      GENERATORS cmake)
					  					 					  
conan_cmake_autodetect(settings)

conan_cmake_install(PATH_OR_REFERENCE .
                    BUILD missing
                    REMOTE conancenter
                    SETTINGS ${settings})

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

file(GLOB_RECURSE PIXIE_SOURCES src/*.cpp)

message(STATUS "FREETYPE_INCLUDE_DIRS: ${FREETYPE_INCLUDE_DIRS}")

add_library(Pixie STATIC ${PIXIE_SOURCES})
target_include_directories(Pixie 
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CONAN_INCLUDE_DIRS}
		${Lua_INCLUDE_DIRS}
		${DIRECTX_INCLUDE_PATH}
		${FREETYPE_INCLUDE_DIRS}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src)

message("CONAN_INCLUDE_DIRS: " ${CONAN_INCLUDE_DIRS})
message("Lua_INCLUDE_DIRS: " ${LUA_INCLUDE_DIRS})
		
target_link_libraries(Pixie PUBLIC freetype CONAN_PKG::fmt CONAN_PKG::lua d3d9.lib d3dx9.lib)
	
link_directories(${DIRECTX_LIB_PATH})

##############
# Sandbox

add_executable(SandBox sandbox/sandbox.cpp)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:WinMainCRTStartup")
target_link_libraries(SandBox PUBLIC Pixie)
set_target_properties(SandBox PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:WinMainCRTStartup")
target_include_directories(SandBox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

SET_TARGET_PROPERTIES(SandBox PROPERTIES LINK_FLAGS_DEBUG "/WHOLEARCHIVE:Pixie")
SET_TARGET_PROPERTIES(SandBox PROPERTIES LINK_FLAGS_RELEASE "/WHOLEARCHIVE:Pixie")
		
################
# Tests

enable_testing()
set(CMAKE_CTEST_COMMAND ctest)
set(CMAKE_CTEST_COMMAND_OPTIONS "--force-new-ctest-process")

add_subdirectory(tests)