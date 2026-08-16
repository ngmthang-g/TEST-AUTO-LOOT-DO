cmake_minimum_required(VERSION 3.24)
project(ThanLongItemConsolidator LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "ThanLongItemConsolidator is Windows-only")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "ThanLongItemConsolidator must be built x64")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(ThanLongItemConsolidator src/main.cpp)
target_compile_definitions(ThanLongItemConsolidator PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX _WIN32_WINNT=0x0A00)
if(MSVC)
    target_compile_options(ThanLongItemConsolidator PRIVATE /utf-8 /W4 /permissive-)
endif()
target_link_libraries(ThanLongItemConsolidator PRIVATE user32 gdi32)

install(TARGETS ThanLongItemConsolidator RUNTIME DESTINATION .)
