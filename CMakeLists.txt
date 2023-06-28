cmake_minimum_required(VERSION 3.11)
project(timemem C)

# Using icon.bat cannot propagate failure to stop CMake...
# add_custom_target(icon_generator
#     COMMAND call icon.bat
#     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
# )

add_custom_target(icon_generator
    COMMAND ${CMAKE_COMMAND} -P icon.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

set(CMAKE_C_STANDARD 11)
string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

add_executable(timemem
    TimeMem.c
    resource.rc # https://www.reddit.com/r/cmake/comments/aqbg0f/comment/egf2p9r
)
add_dependencies(timemem icon_generator)
