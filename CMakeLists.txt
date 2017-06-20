cmake_minimum_required(VERSION 3.4)
project (ipc-project C)
cmake_policy(SET CMP0065 NEW)
option(BUILD_SHARED_LIBS "Default build library as shared" ON)
if (NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Set Release as default build type" FORCE)
endif()
message(STATUS "-------------------------------------------")
message(STATUS "Build type   (CMAKE_BUILD_TYPE)  : " ${CMAKE_BUILD_TYPE})
message(STATUS "Build shared (BUILD_SHARED_LIBS) : " ${BUILD_SHARED_LIBS})
message(STATUS "-------------------------------------------")

# Sources
file(GLOB sources ipc*.c ipc*.h)
file(GLOB sample-server-src samples/msg-server.c)
file(GLOB sample-client-src samples/msg-client.c)
file(GLOB sample-hello-src samples/hello.c)

# Targets
add_library(ipc ${sources})
add_custom_target(samples)
add_executable(sample-server ${sample-server-src})
add_executable(sample-client ${sample-client-src})
add_executable(sample-hello ${sample-hello-src})
add_dependencies(samples sample-server sample-client sample-hello ipc)

# Options
# Additional: -march=core-avx2 -mtune=core-avx2
set(_COMPILE_OPT -pipe -ftree-vectorize -ffast-math -flto -Wall -Wextra -Wpedantic)
set(_LINK_OPT "-flto -fuse-ld=gold")
set(_COMPILE_DEF -D_POSIX_C_SOURCE=199309L)

# ipc
set_target_properties(ipc PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
target_compile_options(ipc PRIVATE ${_COMPILE_OPT})
set_target_properties(ipc PROPERTIES LINK_FLAGS ${_LINK_OPT})
target_compile_definitions(ipc PRIVATE ${_COMPILE_DEF})

# sample-server
set_target_properties(sample-server PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
target_compile_options(sample-server PRIVATE ${_COMPILE_OPT})
set_target_properties(sample-server PROPERTIES LINK_FLAGS ${_LINK_OPT})
target_compile_definitions(sample-server PRIVATE ${_COMPILE_DEF})
target_include_directories(sample-server BEFORE PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(sample-server ipc)

# sample-client
set_target_properties(sample-client PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
target_compile_options(sample-client PRIVATE ${_COMPILE_OPT})
set_target_properties(sample-client PROPERTIES LINK_FLAGS ${_LINK_OPT})
target_compile_definitions(sample-client PRIVATE ${_COMPILE_DEF})
target_include_directories(sample-client BEFORE PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(sample-client ipc)

# sample-hello
set_target_properties(sample-hello PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
target_compile_options(sample-hello PRIVATE ${_COMPILE_OPT})
set_target_properties(sample-hello PROPERTIES LINK_FLAGS ${_LINK_OPT})
target_compile_definitions(sample-hello PRIVATE ${_COMPILE_DEF})
target_include_directories(sample-hello BEFORE PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(sample-hello ipc)
