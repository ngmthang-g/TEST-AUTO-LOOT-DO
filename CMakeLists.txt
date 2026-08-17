cmake_minimum_required(VERSION 3.24)
project(ThanLongItemConsolidator VERSION 0.2.0 LANGUAGES CXX RC)

if(NOT WIN32)
    message(FATAL_ERROR "ThanLongItemConsolidator is Windows-only")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "ThanLongItemConsolidator must be built x64")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# GitHub connector transport: the large verified donor source is stored as
# small ZIP chunks. Rehydrate the exact .inc files at configure time.
set(DONOR_ARCHIVE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/donor159")
set(DONOR_GEN_ROOT "${CMAKE_BINARY_DIR}/generated")
set(DONOR_GEN_DIR "${DONOR_GEN_ROOT}/donor159")
file(MAKE_DIRECTORY "${DONOR_GEN_DIR}")
file(GLOB DONOR_ARCHIVES "${DONOR_ARCHIVE_DIR}/*.zip")
if(NOT DONOR_ARCHIVES)
    message(FATAL_ERROR "Missing vendor/donor159/*.zip source chunks")
endif()
foreach(archive IN LISTS DONOR_ARCHIVES)
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E tar xvf "${archive}"
        WORKING_DIRECTORY "${DONOR_GEN_DIR}"
        RESULT_VARIABLE unpack_result
        OUTPUT_QUIET
        ERROR_VARIABLE unpack_error)
    if(NOT unpack_result EQUAL 0)
        message(FATAL_ERROR "Cannot unpack ${archive}: ${unpack_error}")
    endif()
endforeach()

add_library(ThanLongCleanRouteBridge SHARED src/bridge.cpp)
target_include_directories(ThanLongCleanRouteBridge PRIVATE src "${DONOR_GEN_ROOT}")
target_compile_definitions(ThanLongCleanRouteBridge PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)

add_executable(ThanLongItemConsolidator WIN32 src/controller.cpp resources/app.rc)
target_include_directories(ThanLongItemConsolidator PRIVATE src "${DONOR_GEN_ROOT}")
target_compile_definitions(ThanLongItemConsolidator PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX _WIN32_WINNT=0x0A00)
target_link_libraries(ThanLongItemConsolidator PRIVATE comctl32 user32 kernel32 gdi32)

add_executable(route_logic_tests src/route_logic_test.cpp)
target_include_directories(route_logic_tests PRIVATE src)
add_executable(rotation_logic_tests src/rotation_logic_test.cpp)
target_include_directories(rotation_logic_tests PRIVATE src)

if(MSVC)
    foreach(t ThanLongCleanRouteBridge ThanLongItemConsolidator route_logic_tests rotation_logic_tests)
        target_compile_options(${t} PRIVATE /utf-8 /W4 /permissive-)
    endforeach()
endif()
