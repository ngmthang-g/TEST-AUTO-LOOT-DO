cmake_minimum_required(VERSION 3.24)
project(RemoteLootPoC LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "RemoteLootPoC is Windows-only")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "RemoteLootPoC must be built x64")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(RemoteLootBridge SHARED
    src/Bridge.cpp
    src/Protocol.h
)
target_compile_definitions(RemoteLootBridge PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(RemoteLootBridge PRIVATE user32)
set_target_properties(RemoteLootBridge PROPERTIES OUTPUT_NAME "RemoteLootBridge")

add_executable(RemoteLootProbe
    src/Probe.cpp
    src/Protocol.h
)
target_compile_definitions(RemoteLootProbe PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(RemoteLootProbe PRIVATE user32)

install(TARGETS RemoteLootBridge RemoteLootProbe RUNTIME DESTINATION . LIBRARY DESTINATION .)
