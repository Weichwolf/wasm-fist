cmake_minimum_required(VERSION 3.20)
project(fist_rewrite VERSION 0.1.0 LANGUAGES C)

file(REAL_PATH "${CMAKE_SOURCE_DIR}" source_path)
file(REAL_PATH "${CMAKE_BINARY_DIR}" build_path)
string(FIND "${build_path}/" "${source_path}/" inside_source)
if(inside_source EQUAL 0)
    message(FATAL_ERROR "Use an external build directory under /tmp.")
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/deps/softgl/libsoftgl/CMakeLists.txt")
    message(FATAL_ERROR "Run git submodule update --init --recursive first.")
endif()
add_subdirectory(deps/softgl/libsoftgl)
target_compile_definitions(softgl PRIVATE _POSIX_C_SOURCE=200809L)

add_library(fist_build_options INTERFACE)
target_compile_options(fist_build_options INTERFACE
    -Wall -Wextra -Wpedantic
    -Wno-unused-parameter -Wno-unused-function
    -fno-strict-aliasing -ffast-math)
target_link_libraries(softgl PRIVATE fist_build_options)

add_library(fist_assets STATIC src/assets/scenario.c src/assets/klc.c
    src/assets/palette.c src/assets/resource.c)
target_include_directories(fist_assets PUBLIC src)
target_link_libraries(fist_assets PRIVATE fist_build_options)
target_compile_options(fist_assets PRIVATE -Werror)
add_executable(fist_scenario_probe tools/rewrite/scenario_probe.c)
add_library(fist_probe_io STATIC tools/rewrite/probe_io.c)
target_link_libraries(fist_probe_io PRIVATE fist_build_options)
target_compile_options(fist_probe_io PRIVATE -Werror)
target_link_libraries(fist_scenario_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_scenario_probe PRIVATE -Werror)
add_executable(fist_asset_probe tools/rewrite/asset_probe.c)
target_link_libraries(fist_asset_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_asset_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_scenario_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_scenario_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)
    set_target_properties(fist_asset_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_asset_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)
endif()

add_library(fist_render STATIC src/render/renderer.c)
target_include_directories(fist_render PUBLIC src)
target_link_libraries(fist_render PRIVATE softgl fist_build_options)
target_compile_options(fist_render PRIVATE -Werror)

add_executable(fist_renderer_probe tools/rewrite/renderer_probe.c)
target_link_libraries(fist_renderer_probe PRIVATE fist_render fist_build_options)
target_compile_options(fist_renderer_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    target_compile_options(softgl PRIVATE -msimd128 -pthread)
    target_compile_options(fist_render PRIVATE -msimd128 -pthread)
    target_compile_options(fist_renderer_probe PRIVATE -msimd128 -pthread)
    target_link_options(fist_renderer_probe PRIVATE
        "--js-library=${CMAKE_SOURCE_DIR}/src/platform/wasm/present.js"
        -msimd128 -pthread -sPTHREAD_POOL_SIZE=3 -sEXIT_RUNTIME=1
        -sALLOW_MEMORY_GROWTH=1 -sMODULARIZE=1
        -sEXPORT_NAME=createFistRewrite -sENVIRONMENT=web,node)
    set_target_properties(fist_renderer_probe PROPERTIES SUFFIX ".js")
    configure_file(src/platform/wasm/index.html index.html COPYONLY)
endif()

include(CTest)
if(BUILD_TESTING AND NOT EMSCRIPTEN)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_test(NAME renderer_integration COMMAND fist_renderer_probe)
    add_test(NAME scenario_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_scenario.py
        --target native --native-probe $<TARGET_FILE:fist_scenario_probe>)
    add_test(NAME terrain_asset_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_terrain_assets.py
        --target native --native-probe $<TARGET_FILE:fist_asset_probe>)
endif()
