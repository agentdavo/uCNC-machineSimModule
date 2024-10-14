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
    ${CMAKE_SOURCE_DIR}/src  # Updated to include the src folder
)

# =============================================================================
#                               SOURCE FILES
# =============================================================================

# Additional application sources (with corrected paths)
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

# Create static library for cncvis (this is the core library)
add_library(cncvis STATIC ${ADDITIONAL_SRC})

# Link the TinyGL and STLIO libraries with cncvis
target_link_libraries(cncvis PRIVATE tinygl stlio m mxml)

# =============================================================================
#                                 INSTALLATION
# =============================================================================

# If you want to install this library somewhere for later use:
# install(TARGETS cncvis DESTINATION lib)
# install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include)

# =============================================================================
#                                 END
# =============================================================================
