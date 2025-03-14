# CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(LinearAllocator VERSION 0.1.0 LANGUAGES CXX)

# Set C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})

# Fetch fmt library
include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.1.2
)
FetchContent_MakeAvailable(fmt)

# Create the LinearAllocator as an INTERFACE library
add_library(linear_allocator INTERFACE)
target_include_directories(linear_allocator INTERFACE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(linear_allocator INTERFACE cxx_std_23)

# Add example executable
add_executable(simplified_example example/simplified_example.cpp)
target_link_libraries(simplified_example PRIVATE fmt::fmt linear_allocator)

# Set up Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_INPUT_DIR ${CMAKE_SOURCE_DIR}/src)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/doc)
    set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/doc/DOXYGEN.in)
    set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/Doxyfile)

    # Include additional documentation files
    set(DOXYGEN_EXTRA_INPUT "${CMAKE_SOURCE_DIR}/doc/mainpage.dox ${CMAKE_SOURCE_DIR}/doc/library_usage.dox")

    # Configure the Doxyfile
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    # Add the docs target
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Build settings
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
