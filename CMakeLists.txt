cmake_minimum_required(VERSION 3.24)
project(ThanLongNpcDialogProbe VERSION 0.1.0 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "Windows-only probe")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "x64 build required")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ThanLongNpcDialogProbeBridge SHARED src/bridge.cpp)
target_include_directories(ThanLongNpcDialogProbeBridge PRIVATE src)
target_compile_definitions(ThanLongNpcDialogProbeBridge PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(ThanLongNpcDialogProbeBridge PRIVATE user32 kernel32)

add_executable(ThanLongNpcDialogProbe src/probe.cpp)
target_include_directories(ThanLongNpcDialogProbe PRIVATE src)
target_compile_definitions(ThanLongNpcDialogProbe PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(ThanLongNpcDialogProbe PRIVATE user32 kernel32)

if(MSVC)
    target_compile_options(ThanLongNpcDialogProbeBridge PRIVATE /utf-8 /W4 /permissive-)
    target_compile_options(ThanLongNpcDialogProbe PRIVATE /utf-8 /W4 /permissive-)
endif()
