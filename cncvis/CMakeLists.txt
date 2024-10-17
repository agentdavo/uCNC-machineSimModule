# cncvis/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(cncvis LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# =============================================================================
#                                DIRECTORIES
# =============================================================================

# Define directories relative to the `cncvis` directory
set(TGL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tinygl")
set(STLIO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libstlio")
set(STB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/stb")
set(MXML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mxml")

# Define the static library for cncvis
add_library(cncvis STATIC
    utils.c
    actor.c
    assembly.c
    camera.c
    light.c
    config.c
    api.c
)

# Include directories for cncvis
target_include_directories(cncvis PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${TGL_DIR}/include
    ${STLIO_DIR}/include
    ${STB_DIR}
    ${MXML_DIR}
)

# =============================================================================
#                             SUBDIRECTORIES
# =============================================================================

# Add TinyGL subdirectory
add_subdirectory(tinygl)

# Add STLIO subdirectory
add_subdirectory(libstlio)

# Add STB subdirectory (assuming it's header-only)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${STB_DIR}")

# Add mxml subdirectory
add_subdirectory(mxml)

# =============================================================================
#                              LINKING DEPENDENCIES
# =============================================================================

# Link dependencies to cncvis
target_link_libraries(cncvis PRIVATE
    tinygl
    stlio
    stb
    mxml_static # Use mxml_static instead of mxml
    m  # Link against the math library
)

# =============================================================================
#                               INSTALLATION
# =============================================================================

# Define installation rules for cncvis library
install(TARGETS cncvis
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/cncvis"
)
