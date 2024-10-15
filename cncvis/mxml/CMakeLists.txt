cmake_minimum_required(VERSION 3.10)
project(mxml VERSION 4.0.4 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define architectures for macOS
set(CMAKE_OSX_ARCHITECTURES "x86_64")

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -Os -mmacosx-version-min=11.0 -arch x86_64 \
    -Wall -Wunused -Wno-char-subscripts -Wno-deprecated-declarations -Wno-format-y2k \
    -Wno-switch -Wno-unused-result -D_THREAD_SAFE -D_REENTRANT -D_FORTIFY_SOURCE=3")

# Linker flags for shared libraries
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-no_warn_inits -dynamiclib -lc")

# Linker flags for executables
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

# Define source files
set(PUBLIBOBJS
    mxml-attr.c
    mxml-file.c
    mxml-get.c
    mxml-index.c
    mxml-node.c
    mxml-options.c
    mxml-search.c
    mxml-set.c
)

set(LIBOBJS
    ${PUBLIBOBJS}
    mxml-private.c
)

# Generate config.h from config.h.in
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
    @ONLY
)

# Create static library
add_library(mxml_static STATIC ${LIBOBJS})
target_include_directories(mxml_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}")

# Create shared library
add_library(mxml_shared SHARED ${LIBOBJS})
set_target_properties(mxml_shared PROPERTIES
    OUTPUT_NAME "mxml4"
    VERSION ${PROJECT_VERSION}
    SOVERSION 4
    PUBLIC_HEADER "mxml.h;mxml-private.h"
)
target_include_directories(mxml_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}")

# Link shared library with pthread and math libraries
target_link_libraries(mxml_shared PRIVATE pthread m)

# Define installation directories using GNUInstallDirs for flexibility
include(GNUInstallDirs)

# Install static library
install(TARGETS mxml_static
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libmxml4
)

# Install shared library
install(TARGETS mxml_shared
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libmxml4
)

# Install pkg-config file
configure_file(mxml4.pc.in mxml4.pc @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mxml4.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# Install man page
install(FILES doc/mxml.3 DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)

# Install additional documentation
install(FILES
    doc/mxml.epub
    doc/mxml.html
    doc/mxml-cover.png
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/mxml4
)
