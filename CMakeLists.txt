cmake_minimum_required(VERSION 3.11)

# Policy needs to be set before certain CMake features are used
cmake_policy(SET CMP0091 NEW)   # CMAKE_MSVC_RUNTIME_LIBRARY

# CMake features that configures other CMake variables need to be set before project
# Use static runtime library so that it will also work on PC without Visual C++ Redistributable
# https://learn.microsoft.com/en-us/cpp/c-runtime-library/crt-library-features?view=msvc-170
# Equivalent to adding:
# - Release: MultiThreaded /MT (-D_MT --dependent-lib=libcmt)
# - Debug: /MDd (-D_DLL -D_MT --dependent-lib=msvcrtd)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:DebugDLL>")

# ======================================================================
project(TimeMem)

# Enable c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)   # No compiler specific extensions

# A bit unnecessary but will be useful if new files were accidentally added as *.c
file(GLOB_RECURSE C_SOURCE_FILES "*.c")
set_source_files_properties(${C_SOURCE_FILES} PROPERTIES LANGUAGE CXX)

message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER_FRONTEND_VARIANT: ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")

if(NOT CMAKE_SYSTEM_NAME MATCHES Windows)
    message(FATAL_ERROR "This is an emulation of GNU Time for Windows! (CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME})")
endif()

if(NOT MSVC)
    # Clang CMAKE_CXX_COMPILER_FRONTEND_VARIANT is GNU
    add_compile_options(-Wno-writable-strings)
endif()

add_compile_definitions(_UNICODE UNICODE)

# Using icon.bat cannot propagate failure to stop CMake...
# add_custom_target(icon_generator
#     COMMAND call icon.bat
#     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
# )

add_custom_target(icon_generator
    COMMAND ${CMAKE_COMMAND} -P icon.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_executable(${CMAKE_PROJECT_NAME}
    TimeMem.cpp
    resource.rc # https://www.reddit.com/r/cmake/comments/aqbg0f/comment/egf2p9r
)
add_dependencies(${CMAKE_PROJECT_NAME} icon_generator)
