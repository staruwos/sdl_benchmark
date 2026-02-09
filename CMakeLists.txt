cmake_minimum_required(VERSION 3.10)
project(SDL2Benchmark)

set(CMAKE_CXX_STANDARD 17)

# --- Architecture Detection and Flags ---
# Note: In a true cross-compilation scenario, CMake usually detects CMAKE_SYSTEM_PROCESSOR 
# automatically based on the toolchain file provided.

option(USE_GLES2 "Force usage of OpenGL ES 2.0" OFF)

if(USE_GLES2)
    add_compile_definitions(USE_GLES2)
    message(STATUS "Build Config: Forcing OpenGL ES 2.0 Driver")
else()
    message(STATUS "Build Config: Using Default SDL2 Driver Selection")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Target Architecture: ${CMAKE_SYSTEM_PROCESSOR}")

# Define architecture specific flags (optional, compiler usually defaults correctly)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "amd64")
    message(STATUS "Configuring for x64 (64-bit)")
    add_compile_options(-m64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i[3-6]86" OR CMAKE_SYSTEM_PROCESSOR MATCHES "x86")
    message(STATUS "Configuring for x86 (32-bit)")
    add_compile_options(-m32)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    message(STATUS "Configuring for ARM64")
    # ARM64 specific flags (e.g., NEON is usually implicit in arm64)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    message(STATUS "Configuring for ARM32")
    # Explicitly enable NEON for 32-bit ARM if desired for performance
    # add_compile_options(-mfpu=neon) 
endif()

# --- Dependencies ---

# Find SDL2
find_package(SDL2 REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})

# --- Executable ---

add_executable(SDL2Benchmark main.cpp)

# Link SDL2
target_link_libraries(SDL2Benchmark ${SDL2_LIBRARIES})

# Copy DLLs on Windows (Optional helper)
if(WIN32)
    add_custom_command(TARGET SDL2Benchmark POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL2_LIBDIR}/SDL2.dll
        $<TARGET_FILE_DIR:SDL2Benchmark>)
endif()
