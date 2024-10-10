# CMakeLists.txt for CNC Machine Rendering Framework

# =============================================================================
#                                  PROJECT
# =============================================================================

cmake_minimum_required(VERSION 3.10)
project(cncvis LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# =============================================================================
#                                DIRECTORIES
# =============================================================================

# TinyGL, STLIO, and STB directories
set(TGL_DIR ${CMAKE_SOURCE_DIR}/tinygl)
set(STLIO_DIR ${CMAKE_SOURCE_DIR}/libstlio)
set(STB_DIR ${CMAKE_SOURCE_DIR}/stb)

# =============================================================================
#                              COMPILER FLAGS
# =============================================================================

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall -g -Wno-unused-function")

# Include directories
include_directories(
    ${TGL_DIR}/include
    ${STLIO_DIR}/include
    ${STB_DIR}
    ${CMAKE_SOURCE_DIR}  # Include current directory for utils.h, etc.
)

# =============================================================================
#                               SOURCE FILES
# =============================================================================

# Main application source
set(MAIN_SRC ${CMAKE_SOURCE_DIR}/main.c)

# Additional application sources
set(ADDITIONAL_SRC
    ${CMAKE_SOURCE_DIR}/utils.c
    ${CMAKE_SOURCE_DIR}/actor.c
    ${CMAKE_SOURCE_DIR}/assembly.c
    ${CMAKE_SOURCE_DIR}/camera.c
    ${CMAKE_SOURCE_DIR}/light.c
    ${CMAKE_SOURCE_DIR}/config.c
    ${CMAKE_SOURCE_DIR}/api.c
)

# TinyGL source files
file(GLOB TGL_SRC ${TGL_DIR}/src/*.c)

# STLIO source files
file(GLOB STLIO_SRC ${STLIO_DIR}/src/*.c)

# =============================================================================
#                              STATIC LIBRARIES
# =============================================================================

# Create static library for TinyGL
add_library(tinygl STATIC ${TGL_SRC})

# Create static library for libstlio
add_library(stlio STATIC ${STLIO_SRC})

# =============================================================================
#                             FINAL EXECUTABLE
# =============================================================================

# Define the executable
add_executable(${PROJECT_NAME} ${MAIN_SRC} ${ADDITIONAL_SRC})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE tinygl stlio m mxml)

# =============================================================================
#                                  CLEANUP
# =============================================================================

# Clean target for removing build artifacts
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_BINARY_DIR}/CMakeFiles")

# =============================================================================
#                                 END
# =============================================================================
