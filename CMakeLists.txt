cmake_minimum_required(VERSION 3.11)
project(timemem)

# Enable c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) #No compiler specific extensions

# A bit unnecessary but will be useful if new files were accidentally added as *.c
file(GLOB_RECURSE C_SOURCE_FILES "*.c")
set_source_files_properties(${C_SOURCE_FILES} PROPERTIES LANGUAGE CXX)

# Use static runtime library so that it will also work on PC without Visual C++ Redistributable
string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

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

add_executable(timemem
    TimeMem.cpp
    resource.rc # https://www.reddit.com/r/cmake/comments/aqbg0f/comment/egf2p9r
)
add_dependencies(timemem icon_generator)
