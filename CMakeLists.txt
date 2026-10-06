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

add_library(fist_assets STATIC src/assets/scenario.c src/assets/units.c src/assets/model.c src/assets/vehicle.c src/assets/klc.c
    src/assets/palette.c src/assets/resource.c src/assets/terrain.c src/assets/heightfield.c src/render/model_bitmap.c)
target_include_directories(fist_assets PUBLIC src)
target_link_libraries(fist_assets PRIVATE fist_build_options)
target_compile_options(fist_assets PRIVATE -Werror)
add_library(fist_sim STATIC src/sim/random.c src/sim/ground.c src/sim/rotation.c src/sim/vehicle_state.c src/sim/vehicle_motion.c src/sim/driver.c src/sim/weapon_control.c src/sim/object_pool.c src/sim/projectile_launch.c src/sim/collision.c src/sim/projectile_flight.c src/sim/vehicle_damage.c)
target_include_directories(fist_sim PUBLIC src)
target_link_libraries(fist_sim PUBLIC fist_assets PRIVATE fist_build_options)
target_compile_options(fist_sim PRIVATE -Werror)
add_library(fist_app STATIC src/app/driving.c src/app/driving_view.c)
target_include_directories(fist_app PUBLIC src)
target_link_libraries(fist_app PUBLIC fist_assets fist_sim fist_render PRIVATE fist_build_options)
target_compile_options(fist_app PRIVATE -Werror)
add_executable(fist_driving_probe tools/rewrite/driving_probe.c)
target_link_libraries(fist_driving_probe PRIVATE fist_app fist_probe_io fist_build_options)
target_compile_options(fist_driving_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_driving_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_driving_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_weapon_control_probe tools/rewrite/weapon_control_probe.c)
target_link_libraries(fist_weapon_control_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_weapon_control_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_weapon_control_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_weapon_control_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_object_pool_probe tools/rewrite/object_pool_probe.c)
target_link_libraries(fist_object_pool_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_object_pool_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_object_pool_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_object_pool_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)
endif()
add_executable(fist_vehicle_damage_probe tools/rewrite/vehicle_damage_probe.c)
target_link_libraries(fist_vehicle_damage_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_vehicle_damage_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_vehicle_damage_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_vehicle_damage_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_projectile_flight_probe tools/rewrite/projectile_flight_probe.c)
target_link_libraries(fist_projectile_flight_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_projectile_flight_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_projectile_flight_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_projectile_flight_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_collision_probe tools/rewrite/collision_probe.c)
target_link_libraries(fist_collision_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_collision_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_collision_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_collision_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_projectile_launch_probe tools/rewrite/projectile_launch_probe.c)
target_link_libraries(fist_projectile_launch_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_projectile_launch_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_projectile_launch_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_projectile_launch_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)
endif()
add_executable(fist_vehicle_start_probe tools/rewrite/vehicle_start_probe.c)
target_link_libraries(fist_vehicle_start_probe PRIVATE fist_sim fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_vehicle_start_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_vehicle_start_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_vehicle_start_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)
endif()
add_executable(fist_ground_probe tools/rewrite/ground_probe.c)
target_link_libraries(fist_ground_probe PRIVATE fist_sim fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_ground_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_ground_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_ground_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_vehicle_motion_probe tools/rewrite/vehicle_motion_probe.c)
target_link_libraries(fist_vehicle_motion_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_vehicle_motion_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_vehicle_motion_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_vehicle_motion_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_scenario_probe tools/rewrite/scenario_probe.c)
add_library(fist_probe_io STATIC tools/rewrite/probe_io.c tools/rewrite/probe_source.c tools/rewrite/vehicle_probe_io.c tools/rewrite/object_pool_probe_io.c)
target_include_directories(fist_probe_io PRIVATE src)
target_link_libraries(fist_probe_io PRIVATE fist_build_options)
target_compile_options(fist_probe_io PRIVATE -Werror)
target_link_libraries(fist_scenario_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_scenario_probe PRIVATE -Werror)
add_executable(fist_asset_probe tools/rewrite/asset_probe.c)
target_link_libraries(fist_asset_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_asset_probe PRIVATE -Werror)
add_executable(fist_heightfield_probe tools/rewrite/heightfield_probe.c)
target_link_libraries(fist_heightfield_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_heightfield_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_heightfield_probe PROPERTIES SUFFIX ".js")
    # A bounded probe heap makes real allocation-failure cases reproducible.
    target_link_options(fist_heightfield_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1
        -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=67108864 -sABORTING_MALLOC=0)
endif()
add_executable(fist_terrain_probe tools/rewrite/terrain_probe.c)
target_link_libraries(fist_terrain_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_terrain_probe PRIVATE -Werror)
add_executable(fist_unit_probe tools/rewrite/unit_probe.c)
target_link_libraries(fist_unit_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_unit_probe PRIVATE -Werror)
add_executable(fist_model_probe tools/rewrite/model_probe.c)
target_link_libraries(fist_model_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_model_probe PRIVATE -Werror)
add_executable(fist_vehicle_probe tools/rewrite/vehicle_probe.c)
target_link_libraries(fist_vehicle_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_vehicle_probe PRIVATE -Werror)
add_executable(fist_model_bitmap_probe tools/rewrite/model_bitmap_probe.c)
target_link_libraries(fist_model_bitmap_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_model_bitmap_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_model_bitmap_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_model_bitmap_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
    set_target_properties(fist_vehicle_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_vehicle_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)
    set_target_properties(fist_model_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_model_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
    set_target_properties(fist_unit_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_unit_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)
    set_target_properties(fist_scenario_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_scenario_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)
    set_target_properties(fist_asset_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_asset_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)
    set_target_properties(fist_terrain_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_terrain_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)
endif()

add_library(fist_render STATIC src/render/renderer.c src/render/terrain_scene.c src/render/vehicle_scene.c src/render/hud.c)
target_include_directories(fist_render PUBLIC src)
target_link_libraries(fist_render PRIVATE softgl fist_assets fist_build_options)
target_compile_options(fist_render PRIVATE -Werror)

add_executable(fist_driving_preview tools/rewrite/driving_preview.c)
target_link_libraries(fist_driving_preview PRIVATE fist_app fist_probe_io fist_build_options)
target_compile_options(fist_driving_preview PRIVATE -Werror)
if(EMSCRIPTEN)
    target_compile_options(fist_driving_preview PRIVATE -msimd128 -pthread)
    target_link_options(fist_driving_preview PRIVATE
        "--js-library=${CMAKE_SOURCE_DIR}/src/platform/wasm/present.js"
        -msimd128 -pthread -sPTHREAD_POOL_SIZE=3 -sEXIT_RUNTIME=0
        -sALLOW_MEMORY_GROWTH=1 -sMODULARIZE=1 -sINVOKE_RUN=0
        -sEXPORT_NAME=createFistDrivingPreview -sENVIRONMENT=web,node
        "-sEXPORTED_RUNTIME_METHODS=['FS','callMain','PThread']"
        "-sEXPORTED_FUNCTIONS=['_main','_fist_preview_advance','_fist_preview_key','_fist_preview_freeze','_fist_preview_frame','_fist_preview_pixels','_fist_preview_value','_fist_preview_destroy']")
    set_target_properties(fist_driving_preview PROPERTIES SUFFIX ".js")
    configure_file(src/platform/wasm/driving.html driving.html COPYONLY)
    configure_file(src/platform/wasm/driving.js driving.js COPYONLY)
    configure_file(src/platform/wasm/preview_assets.js preview_assets.js COPYONLY)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
    target_link_libraries(fist_driving_preview PRIVATE PkgConfig::SDL2)
endif()

add_executable(fist_terrain_preview tools/rewrite/terrain_preview.c)
target_link_libraries(fist_terrain_preview PRIVATE fist_render fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_terrain_preview PRIVATE -Werror)
if(EMSCRIPTEN)
    target_compile_options(fist_terrain_preview PRIVATE -msimd128 -pthread)
    target_link_options(fist_terrain_preview PRIVATE
        "--js-library=${CMAKE_SOURCE_DIR}/src/platform/wasm/present.js"
        -msimd128 -pthread -sPTHREAD_POOL_SIZE=3 -sEXIT_RUNTIME=1
        -sALLOW_MEMORY_GROWTH=1 -sMODULARIZE=1 -sINVOKE_RUN=0
        -sEXPORT_NAME=createFistTerrainPreview -sENVIRONMENT=web,node
        "-sEXPORTED_RUNTIME_METHODS=['FS','callMain']")
    set_target_properties(fist_terrain_preview PROPERTIES SUFFIX ".js")
    configure_file(src/platform/wasm/terrain.html terrain.html COPYONLY)
    configure_file(src/platform/wasm/terrain.js terrain.js COPYONLY)
endif()

add_executable(fist_vehicle_scene_probe tools/rewrite/vehicle_scene_probe.c)
target_link_libraries(fist_vehicle_scene_probe PRIVATE fist_render fist_assets fist_build_options)
target_compile_options(fist_vehicle_scene_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    target_compile_options(fist_vehicle_scene_probe PRIVATE -msimd128 -pthread)
    target_link_options(fist_vehicle_scene_probe PRIVATE -msimd128 -pthread
        -sPTHREAD_POOL_SIZE=3 -sEXIT_RUNTIME=1 -sMODULARIZE=1 -sENVIRONMENT=node)
    set_target_properties(fist_vehicle_scene_probe PROPERTIES SUFFIX ".js")
endif()

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
    add_test(NAME vehicle_damage_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_vehicle_damage.py
        --target native --native-probe $<TARGET_FILE:fist_vehicle_damage_probe>)
    add_test(NAME projectile_flight_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_projectile_flight.py
        --target native --native-probe $<TARGET_FILE:fist_projectile_flight_probe>)
    add_test(NAME collision_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_collision.py
        --target native --native-probe $<TARGET_FILE:fist_collision_probe>)
    add_test(NAME projectile_launch_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_projectile_launch.py
        --target native --native-probe $<TARGET_FILE:fist_projectile_launch_probe>)
    add_test(NAME object_pool_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_object_pool.py
        --target native --native-probe $<TARGET_FILE:fist_object_pool_probe>)
    add_test(NAME driving_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_driving.py
        --target native --native-probe $<TARGET_FILE:fist_driving_probe>)
    add_test(NAME ground_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_ground.py
        --target native --native-probe $<TARGET_FILE:fist_ground_probe>)
    add_test(NAME heightfield_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_heightfield.py
        --target native --native-probe $<TARGET_FILE:fist_heightfield_probe>)
    add_test(NAME vehicle_motion_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_vehicle_motion.py
        --target native --native-probe $<TARGET_FILE:fist_vehicle_motion_probe>)
    add_test(NAME vehicle_start_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_vehicle_start.py
        --target native --native-probe $<TARGET_FILE:fist_vehicle_start_probe>)
    add_test(NAME weapon_control_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_weapon_control.py
        --target native --native-probe $<TARGET_FILE:fist_weapon_control_probe>)
    add_test(NAME vehicle_scene_pixels COMMAND fist_vehicle_scene_probe)
    add_test(NAME renderer_integration COMMAND fist_renderer_probe)
    add_test(NAME scenario_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_scenario.py
        --target native --native-probe $<TARGET_FILE:fist_scenario_probe>)
    add_test(NAME unit_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_units.py
        --target native --native-probe $<TARGET_FILE:fist_unit_probe>)
    add_test(NAME vehicle_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_vehicle.py
        --target native --native-probe $<TARGET_FILE:fist_vehicle_probe>)
    add_test(NAME model_bitmap_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_model_bitmap.py
        --target native --native-probe $<TARGET_FILE:fist_model_bitmap_probe>)
    add_test(NAME model_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_models.py
        --target native --native-probe $<TARGET_FILE:fist_model_probe>)
    add_test(NAME terrain_asset_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_terrain_assets.py
        --target native --native-probe $<TARGET_FILE:fist_asset_probe>
        --terrain-probe $<TARGET_FILE:fist_terrain_probe>)
    add_test(NAME terrain_scene_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/rewrite/test_terrain_scene.py
        --target native --native-preview $<TARGET_FILE:fist_terrain_preview>)
endif()
