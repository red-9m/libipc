cmake_minimum_required(VERSION 3.0)
project (ipc-project C)
file(GLOB sources ipc*.c ipc*.h)
file(GLOB sample-server-src sample-server*.c sample-server*.h)
file(GLOB sample-client-src sample-client*.c sample-client*.h)

add_library(ipc SHARED ${sources})
add_library(ipc-static STATIC ${sources})
add_custom_target(samples)
add_executable(sample-server EXCLUDE_FROM_ALL ${sample-server-src})
add_executable(sample-client EXCLUDE_FROM_ALL ${sample-client-src})
add_dependencies(samples sample-server sample-client ipc)

set(_C_FLAGS "-std=c11 -pipe -Wall -Wextra")
set(_L_FLAGS "-fuse-ld=gold -s")
set_target_properties(ipc PROPERTIES COMPILE_FLAGS ${_C_FLAGS} LINK_FLAGS ${_L_FLAGS})
set_target_properties(ipc-static PROPERTIES COMPILE_FLAGS ${_C_FLAGS} LINK_FLAGS ${_L_FLAGS})
set_target_properties(ipc-static PROPERTIES OUTPUT_NAME ipc)
set_target_properties(sample-server PROPERTIES COMPILE_FLAGS ${_C_FLAGS} LINK_FLAGS ${_L_FLAGS})
set_target_properties(sample-client PROPERTIES COMPILE_FLAGS ${_C_FLAGS} LINK_FLAGS ${_L_FLAGS})

#add_definitions(-D_POSIX_C_SOURCE=199309L)
#add_definitions(-D_IPC_VERSION=0.0.1)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE} -flto")

target_link_libraries(sample-server ipc-static)
target_link_libraries(sample-client ipc-static)