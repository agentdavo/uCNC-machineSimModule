# Minimum CMake version
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(libmxml VERSION 4.0.4 LANGUAGES C)

set(MXML_VERSION ${PROJECT_VERSION})

# name library
set(TargetName mxml_static)

# Specify the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(MXML_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(MXML_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(MXML_VERSION_MICRO ${PROJECT_VERSION_PATCH})

# add cached variable for include directory
set(MXML_INCLUDES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" CACHE PATH "Path to mxml include directory" )

# Define the source directory containing the header files
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Extract function to handle directory creation
function(create_directory dir)
  file(MAKE_DIRECTORY ${dir})
endfunction()

# Create the directories using the extracted function
create_directory(${MXML_INCLUDES_DIR})
create_directory(${MXML_INCLUDES_DIR}/mxml)

# config.h file
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.lnx.in" "${CMAKE_CURRENT_SOURCE_DIR}/config.h")

# Copy all .h files from SOURCE_DIR to DEST_DIR
file(GLOB HEADER_FILES "${SOURCE_DIR}/*.h")
foreach(header ${HEADER_FILES})
  file(COPY ${header} DESTINATION ${MXML_INCLUDES_DIR}/mxml)
endforeach()

# Source files for the library
set(SOURCES
   mxml-attr.c
   mxml-file.c
   mxml-get.c
   mxml-index.c
   mxml-node.c
   mxml-options.c
   mxml-private.c
   mxml-search.c
   mxml-set.c
)

# Header files to install
set(HEADERS
   mxml-private.h
   config.h
)

# Create the library
add_library(${TargetName} STATIC ${SOURCES} ${HEADERS})
add_library(${TargetName}::${TargetName} ALIAS ${TargetName})

# Add version info to the library
set_target_properties(${TargetName} PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION 1
)

target_include_directories(${TargetName} PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Enable secure build settings
target_compile_definitions(${TargetName} PRIVATE -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE)
target_compile_options(${TargetName} PRIVATE -Wall -Werror -fstack-protector-strong -Wformat -Wformat-security -O2)
target_link_options(${TargetName} PRIVATE -Wl,-z,relro -Wl,-z,now)

# Remove -Werror=unused-parameter for this specific target
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${TargetName} PRIVATE -Wno-unused-parameter)
endif ()

# If you are using Clang, you might need similar condition:
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${TargetName} PRIVATE -Wno-unused-parameter)
endif ()

# Link libraries based on the OS
if (WIN32)
  target_link_libraries(${TargetName} wsock32)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "QNX")
  target_link_libraries(${TargetName} socket)
endif ()

# Install library and headers
install(TARGETS ${TargetName}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${HEADERS}
   DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mxml
)
