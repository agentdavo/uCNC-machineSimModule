# libstlio/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(libstlio LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Collect all source files in the src directory
file(GLOB STLIO_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# Define the static library 'stlio' with the collected source files
add_library(stlio STATIC ${STLIO_SRC})

# Specify include directories (only 'include', not 'src')
target_include_directories(stlio PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Define installation rules for the 'stlio' library and its public headers
install(TARGETS stlio
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libstlio"
)

# Optionally, install header files if they are not covered by PUBLIC_HEADER
# Uncomment the following lines if needed
# install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
#         DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libstlio"
#         FILES_MATCHING PATTERN "*.h")
