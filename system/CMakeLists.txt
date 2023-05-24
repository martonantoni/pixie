# Set the library name
set(LIBRARY_NAME PixieSystem)

# Add all the source files recursively from the current directory
file(GLOB_RECURSE LIBRARY_SOURCES *.cpp)
file(GLOB_RECURSE EXCLUDE_SOURCES lua/*)

list(REMOVE_ITEM LIBRARY_SOURCES ${EXCLUDE_SOURCES})

# Create the library
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

