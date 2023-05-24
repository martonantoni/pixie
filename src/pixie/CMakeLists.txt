# Set the library name
set(LIBRARY_NAME PixiePixie)

# Add all the source files recursively from the current directory
file(GLOB_RECURSE LIBRARY_SOURCES *.cpp)

# Create the library
add_library(${LIBRARY_NAME} ${LIBRARY_SOURCES})

target_include_directories(${LIBRARY_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)