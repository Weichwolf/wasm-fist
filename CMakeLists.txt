cmake_minimum_required(VERSION 3.20)
project(fist_rewrite VERSION 0.1.0 LANGUAGES C)

if(EMSCRIPTEN)
    # Keep source locations without DWARF restricting Binaryen optimization.
    string(REGEX REPLACE "(^|[ \t])-g([ \t]|$)" "\\1-gsource-map\\2"
        CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
endif()

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

if(EMSCRIPTEN)
    target_link_options(fist_build_options INTERFACE -Werror)
endif()

if(EMSCRIPTEN)
    # Reserve the previous wasm32 maximum up front to keep shared views stable.
    add_library(fist_wasm_render_options INTERFACE)
    target_link_options(fist_wasm_render_options INTERFACE
        -sINITIAL_MEMORY=2147483648 -sALLOW_MEMORY_GROWTH=0 -sABORTING_MALLOC=0)
endif()

add_library(fist_assets STATIC src/assets/scenario.c src/assets/orders.c src/assets/units.c src/assets/model.c src/assets/vehicle.c src/assets/klc.c
    src/assets/palette.c src/assets/resource.c src/assets/terrain.c src/assets/heightfield.c src/render/model_bitmap.c)
target_include_directories(fist_assets PUBLIC src)
target_link_libraries(fist_assets PRIVATE fist_build_options)
target_compile_options(fist_assets PRIVATE -Werror)
add_library(fist_sim STATIC src/sim/random.c src/sim/ground.c src/sim/rotation.c src/sim/geometry.c src/sim/vehicle_state.c src/sim/vehicle_motion.c src/sim/driver.c src/sim/weapon_control.c src/sim/object_pool.c src/sim/projectile_launch.c src/sim/collision.c src/sim/projectile_flight.c src/sim/vehicle_damage.c src/sim/damage_common.c src/sim/other_damage.c src/sim/smoke_animation.c src/sim/smoke.c src/sim/destruction_updates.c src/sim/aircraft_death.c src/sim/world_step.c src/sim/tree.c src/sim/mission_world.c src/sim/mission_ready.c src/sim/ground_command.c src/sim/roster_promotion.c src/sim/ground_maneuver.c src/sim/automatic_fire.c src/sim/ground_station.c src/sim/ground_support.c src/sim/ground_phase.c src/sim/mission_view.c src/sim/target_discovery.c src/sim/target_acquisition.c src/sim/voice.c src/sim/primary_fire.c src/sim/mission_update.c)
target_include_directories(fist_sim PUBLIC src)
target_link_libraries(fist_sim PUBLIC fist_assets PRIVATE fist_build_options)
target_compile_options(fist_sim PRIVATE -Werror)
add_library(fist_app STATIC src/app/driving.c src/app/driving_view.c)
target_include_directories(fist_app PUBLIC src)
target_link_libraries(fist_app PUBLIC fist_assets fist_sim fist_render PRIVATE fist_build_options)
target_compile_options(fist_app PRIVATE -Werror)
add_executable(fist_driving_probe tests/driving_probe.c)
target_link_libraries(fist_driving_probe PRIVATE fist_app fist_probe_io fist_build_options)
target_compile_options(fist_driving_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_driving_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_driving_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_weapon_control_probe tests/weapon_control_probe.c)
target_link_libraries(fist_weapon_control_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_weapon_control_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_weapon_control_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_weapon_control_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_ground_elevation_probe tests/ground_elevation_probe.c)
target_link_libraries(fist_ground_elevation_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_ground_elevation_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_ground_elevation_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_ground_elevation_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_object_pool_probe tests/object_pool_probe.c)
target_link_libraries(fist_object_pool_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_object_pool_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_object_pool_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_object_pool_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)
endif()
add_executable(fist_ground_phase_probe tests/ground_phase_probe.c)
target_link_libraries(fist_ground_phase_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_ground_phase_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_ground_phase_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_ground_phase_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_remaining_ground_probe tests/remaining_ground_probe.c)
target_link_libraries(fist_remaining_ground_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_remaining_ground_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_remaining_ground_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_remaining_ground_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_target_discovery_probe tests/target_discovery_probe.c)
target_link_libraries(fist_target_discovery_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_target_discovery_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_target_discovery_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_target_discovery_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_ground_target_motion_lifetime_probe tests/ground_target_motion_lifetime_probe.c)
target_link_libraries(fist_ground_target_motion_lifetime_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_ground_target_motion_lifetime_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_ground_target_motion_lifetime_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_ground_target_motion_lifetime_probe PRIVATE
        -sEXIT_RUNTIME=1 -sENVIRONMENT=node -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_mission_combat_probe tests/mission_combat_probe.c)
target_link_libraries(fist_mission_combat_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_mission_combat_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_mission_combat_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_mission_combat_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_mission_ready_probe tests/mission_ready_probe.c)
target_link_libraries(fist_mission_ready_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_mission_ready_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_mission_ready_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_mission_ready_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_mission_world_probe tests/mission_world_probe.c)
target_link_libraries(fist_mission_world_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_mission_world_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_mission_world_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_mission_world_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_ground_command_probe tests/ground_command_probe.c)
target_link_libraries(fist_ground_command_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_ground_command_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_ground_command_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_ground_command_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_ground_goal_probe tests/ground_goal_probe.c)
target_link_libraries(fist_ground_goal_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_ground_goal_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_ground_goal_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_ground_goal_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_ground_throttle_probe tests/ground_throttle_probe.c)
target_link_libraries(fist_ground_throttle_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_ground_throttle_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_ground_throttle_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_ground_throttle_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_geometry_probe tests/geometry_probe.c)
target_link_libraries(fist_geometry_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_geometry_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_geometry_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_geometry_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_world_step_probe tests/world_step_probe.c)
target_link_libraries(fist_world_step_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_world_step_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_world_step_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_world_step_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_aircraft_death_probe tests/aircraft_death_probe.c)
target_link_libraries(fist_aircraft_death_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_aircraft_death_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_aircraft_death_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_aircraft_death_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_destruction_probe tests/destruction_probe.c)
target_link_libraries(fist_destruction_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_destruction_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_destruction_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_destruction_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_other_damage_probe tests/other_damage_probe.c)
target_link_libraries(fist_other_damage_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_other_damage_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_other_damage_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_other_damage_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_vehicle_damage_probe tests/vehicle_damage_probe.c)
target_link_libraries(fist_vehicle_damage_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_vehicle_damage_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_vehicle_damage_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_vehicle_damage_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_projectile_flight_probe tests/projectile_flight_probe.c)
target_link_libraries(fist_projectile_flight_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_projectile_flight_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_projectile_flight_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_projectile_flight_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_collision_probe tests/collision_probe.c)
target_link_libraries(fist_collision_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_collision_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_collision_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_collision_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_projectile_launch_probe tests/projectile_launch_probe.c)
target_link_libraries(fist_projectile_launch_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_projectile_launch_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_projectile_launch_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_projectile_launch_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)
endif()
add_executable(fist_vehicle_start_probe tests/vehicle_start_probe.c)
target_link_libraries(fist_vehicle_start_probe PRIVATE fist_sim fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_vehicle_start_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_vehicle_start_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_vehicle_start_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1)
endif()
add_executable(fist_visibility_probe tests/visibility_probe.c)
target_link_libraries(fist_visibility_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_visibility_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_visibility_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_visibility_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_ground_probe tests/ground_probe.c)
target_link_libraries(fist_ground_probe PRIVATE fist_sim fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_ground_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_ground_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_ground_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_vehicle_motion_probe tests/vehicle_motion_probe.c)
target_link_libraries(fist_vehicle_motion_probe PRIVATE fist_sim fist_probe_io fist_build_options)
target_compile_options(fist_vehicle_motion_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_vehicle_motion_probe PROPERTIES SUFFIX ".js")
    target_link_options(fist_vehicle_motion_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1)
endif()
add_executable(fist_scenario_probe tests/scenario_probe.c)
add_library(fist_probe_io STATIC tests/probe_io.c tests/probe_source.c tests/vehicle_probe_io.c tests/object_pool_probe_io.c tests/combat_probe_io.c tests/mission_probe_io.c)
target_include_directories(fist_probe_io PRIVATE src)
target_link_libraries(fist_probe_io PRIVATE fist_build_options)
target_compile_options(fist_probe_io PRIVATE -Werror)
target_link_libraries(fist_scenario_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_scenario_probe PRIVATE -Werror)
add_executable(fist_asset_probe tests/asset_probe.c)
target_link_libraries(fist_asset_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_asset_probe PRIVATE -Werror)
add_executable(fist_heightfield_probe tests/heightfield_probe.c)
target_link_libraries(fist_heightfield_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_heightfield_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    set_target_properties(fist_heightfield_probe PROPERTIES SUFFIX ".js")
    # A bounded probe heap makes real allocation-failure cases reproducible.
    target_link_options(fist_heightfield_probe PRIVATE -sNODERAWFS=1 -sEXIT_RUNTIME=1
        -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=67108864 -sABORTING_MALLOC=0)
endif()
add_executable(fist_terrain_probe tests/terrain_probe.c)
target_link_libraries(fist_terrain_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_terrain_probe PRIVATE -Werror)
add_executable(fist_unit_probe tests/unit_probe.c)
target_link_libraries(fist_unit_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_unit_probe PRIVATE -Werror)
add_executable(fist_model_probe tests/model_probe.c)
target_link_libraries(fist_model_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_model_probe PRIVATE -Werror)
add_executable(fist_vehicle_probe tests/vehicle_probe.c)
target_link_libraries(fist_vehicle_probe PRIVATE fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_vehicle_probe PRIVATE -Werror)
add_executable(fist_model_bitmap_probe tests/model_bitmap_probe.c)
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

add_executable(fist_driving_preview tests/driving_preview.c)
target_link_libraries(fist_driving_preview PRIVATE fist_app fist_probe_io fist_build_options)
target_compile_options(fist_driving_preview PRIVATE -Werror)
if(EMSCRIPTEN)
    target_link_libraries(fist_driving_preview PRIVATE fist_wasm_render_options)
    target_compile_options(fist_driving_preview PRIVATE -msimd128 -pthread)
    target_link_options(fist_driving_preview PRIVATE
        "--js-library=${CMAKE_SOURCE_DIR}/src/platform/wasm/present.js"
        -msimd128 -pthread -sPTHREAD_POOL_SIZE=3 -sEXIT_RUNTIME=0
        -sMODULARIZE=1 -sINVOKE_RUN=0
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

add_executable(fist_terrain_preview tests/terrain_preview.c)
target_link_libraries(fist_terrain_preview PRIVATE fist_render fist_assets fist_probe_io fist_build_options)
target_compile_options(fist_terrain_preview PRIVATE -Werror)
if(EMSCRIPTEN)
    target_link_libraries(fist_terrain_preview PRIVATE fist_wasm_render_options)
    target_compile_options(fist_terrain_preview PRIVATE -msimd128 -pthread)
    target_link_options(fist_terrain_preview PRIVATE
        "--js-library=${CMAKE_SOURCE_DIR}/src/platform/wasm/present.js"
        -msimd128 -pthread -sPTHREAD_POOL_SIZE=3 -sEXIT_RUNTIME=1
        -sMODULARIZE=1 -sINVOKE_RUN=0
        -sEXPORT_NAME=createFistTerrainPreview -sENVIRONMENT=web,node
        "-sEXPORTED_RUNTIME_METHODS=['FS','callMain']")
    set_target_properties(fist_terrain_preview PROPERTIES SUFFIX ".js")
    configure_file(src/platform/wasm/terrain.html terrain.html COPYONLY)
    configure_file(src/platform/wasm/terrain.js terrain.js COPYONLY)
endif()

add_executable(fist_vehicle_scene_probe tests/vehicle_scene_probe.c)
target_link_libraries(fist_vehicle_scene_probe PRIVATE fist_render fist_assets fist_build_options)
target_compile_options(fist_vehicle_scene_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    target_compile_options(fist_vehicle_scene_probe PRIVATE -msimd128 -pthread)
    target_link_options(fist_vehicle_scene_probe PRIVATE -msimd128 -pthread
        -sPTHREAD_POOL_SIZE=3 -sEXIT_RUNTIME=1 -sMODULARIZE=1 -sENVIRONMENT=node)
    set_target_properties(fist_vehicle_scene_probe PROPERTIES SUFFIX ".js")
endif()

add_executable(fist_renderer_probe tests/renderer_probe.c)
target_link_libraries(fist_renderer_probe PRIVATE fist_render fist_build_options)
target_compile_options(fist_renderer_probe PRIVATE -Werror)
if(EMSCRIPTEN)
    target_link_libraries(fist_renderer_probe PRIVATE fist_wasm_render_options)
    target_compile_options(softgl PRIVATE -msimd128 -pthread)
    target_compile_options(fist_render PRIVATE -msimd128 -pthread)
    target_compile_options(fist_renderer_probe PRIVATE -msimd128 -pthread)
    target_link_options(fist_renderer_probe PRIVATE
        "--js-library=${CMAKE_SOURCE_DIR}/src/platform/wasm/present.js"
        -msimd128 -pthread -sPTHREAD_POOL_SIZE=3 -sEXIT_RUNTIME=1
        -sMODULARIZE=1
        -sEXPORT_NAME=createFistRewrite -sENVIRONMENT=web,node)
    set_target_properties(fist_renderer_probe PROPERTIES SUFFIX ".js")
    configure_file(src/platform/wasm/index.html index.html COPYONLY)
endif()

include(CTest)
if(BUILD_TESTING AND NOT EMSCRIPTEN)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_test(NAME mission_combat_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_mission_combat.py
        --target native --native-probe $<TARGET_FILE:fist_mission_combat_probe>)
    add_test(NAME primary_fire_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_primary_fire.py
        --target native --native-probe $<TARGET_FILE:fist_projectile_launch_probe>)
    add_test(NAME mission_ready_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_mission_ready.py
        --target native --native-probe $<TARGET_FILE:fist_mission_ready_probe>)
    add_test(NAME mission_world_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_mission_world.py
        --target native --native-probe $<TARGET_FILE:fist_mission_world_probe>)
    add_test(NAME ground_phase_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_ground_phase.py
        --target native --native-probe $<TARGET_FILE:fist_ground_phase_probe>)
    add_test(NAME ground_target_motion_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_ground_target_motion.py
        --target native --native-probe $<TARGET_FILE:fist_target_discovery_probe>)
    add_test(NAME ground_target_motion_lifetime_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_ground_target_motion_lifetime.py
        --target native --native-probe $<TARGET_FILE:fist_ground_target_motion_lifetime_probe>)
    add_test(NAME ground_command_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_ground_command.py
        --target native --native-probe $<TARGET_FILE:fist_ground_command_probe>)
    add_test(NAME ground_bearing_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_ground_bearing.py
        --target native --native-probe $<TARGET_FILE:fist_ground_goal_probe>)
    add_test(NAME ground_throttle_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_ground_throttle.py
        --target native --native-probe $<TARGET_FILE:fist_ground_throttle_probe>)
    add_test(NAME ground_goal_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_ground_goal.py
        --target native --native-probe $<TARGET_FILE:fist_ground_goal_probe>)
    add_test(NAME proximity_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_proximity.py
        --target native --native-probe $<TARGET_FILE:fist_geometry_probe>)
    add_test(NAME target_discovery_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_target_discovery.py
        --target native --native-probe $<TARGET_FILE:fist_target_discovery_probe>)
    add_test(NAME target_acquisition_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_target_acquisition.py
        --target native --native-probe $<TARGET_FILE:fist_target_discovery_probe>)
    add_test(NAME roster_promotion_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_roster_promotion.py
        --target native --native-probe $<TARGET_FILE:fist_target_discovery_probe>)
    add_test(NAME ground_maneuver_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_ground_maneuver.py
        --target native --native-probe $<TARGET_FILE:fist_target_discovery_probe>)
    add_test(NAME remaining_ground_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_remaining_ground.py
        --target native --native-probe $<TARGET_FILE:fist_remaining_ground_probe>)
    add_test(NAME automatic_fire_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_automatic_fire.py
        --target native --native-probe $<TARGET_FILE:fist_target_discovery_probe>)
    add_test(NAME geometry_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_geometry.py
        --target native --native-probe $<TARGET_FILE:fist_geometry_probe>)
    add_test(NAME ground_route_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_ground_route.py
        --target native --native-probe $<TARGET_FILE:fist_ground_goal_probe>)
    add_test(NAME world_step_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_world_step.py
        --target native --native-probe $<TARGET_FILE:fist_world_step_probe>)
    add_test(NAME aircraft_death_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_aircraft_death.py
        --target native --native-probe $<TARGET_FILE:fist_aircraft_death_probe>)
    add_test(NAME destruction_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_destruction.py
        --target native --native-probe $<TARGET_FILE:fist_destruction_probe>)
    add_test(NAME other_damage_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_other_damage.py
        --target native --native-probe $<TARGET_FILE:fist_other_damage_probe>)
    add_test(NAME vehicle_damage_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_vehicle_damage.py
        --target native --native-probe $<TARGET_FILE:fist_vehicle_damage_probe>)
    add_test(NAME projectile_flight_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_projectile_flight.py
        --target native --native-probe $<TARGET_FILE:fist_projectile_flight_probe>)
    add_test(NAME collision_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_collision.py
        --target native --native-probe $<TARGET_FILE:fist_collision_probe>)
    add_test(NAME projectile_launch_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_projectile_launch.py
        --target native --native-probe $<TARGET_FILE:fist_projectile_launch_probe>)
    add_test(NAME object_pool_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_object_pool.py
        --target native --native-probe $<TARGET_FILE:fist_object_pool_probe>)
    add_test(NAME mission_driving_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_mission_driving.py
        --target native --native-probe $<TARGET_FILE:fist_driving_probe>)
    add_test(NAME driving_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_driving.py
        --target native --native-probe $<TARGET_FILE:fist_driving_probe>)
    add_test(NAME visibility_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_visibility.py
        --target native --native-probe $<TARGET_FILE:fist_visibility_probe>)
    add_test(NAME ground_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_ground.py
        --target native --native-probe $<TARGET_FILE:fist_ground_probe>)
    add_test(NAME heightfield_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_heightfield.py
        --target native --native-probe $<TARGET_FILE:fist_heightfield_probe>)
    add_test(NAME vehicle_maintenance_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_vehicle_maintenance.py
        --target native --native-probe $<TARGET_FILE:fist_vehicle_motion_probe>)
    add_test(NAME vehicle_history_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_vehicle_history.py
        --target native --native-probe $<TARGET_FILE:fist_vehicle_motion_probe>)
    add_test(NAME vehicle_motion_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_vehicle_motion.py
        --target native --native-probe $<TARGET_FILE:fist_vehicle_motion_probe>)
    add_test(NAME vehicle_start_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_vehicle_start.py
        --target native --native-probe $<TARGET_FILE:fist_vehicle_start_probe>)
    add_test(NAME weapon_control_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_weapon_control.py
        --target native --native-probe $<TARGET_FILE:fist_weapon_control_probe>)
    add_test(NAME ground_elevation_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_ground_elevation.py
        --target native --native-probe $<TARGET_FILE:fist_ground_elevation_probe>)
    add_test(NAME vehicle_scene_pixels COMMAND fist_vehicle_scene_probe)
    add_test(NAME renderer_integration COMMAND fist_renderer_probe)
    add_test(NAME scenario_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_scenario.py
        --target native --native-probe $<TARGET_FILE:fist_scenario_probe>)
    add_test(NAME orders_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_orders.py
        --target native --native-probe $<TARGET_FILE:fist_scenario_probe>)
    add_test(NAME unit_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_units.py
        --target native --native-probe $<TARGET_FILE:fist_unit_probe>)
    add_test(NAME vehicle_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_vehicle.py
        --target native --native-probe $<TARGET_FILE:fist_vehicle_probe>)
    add_test(NAME model_bitmap_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_model_bitmap.py
        --target native --native-probe $<TARGET_FILE:fist_model_bitmap_probe>)
    add_test(NAME model_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_models.py
        --target native --native-probe $<TARGET_FILE:fist_model_probe>)
    add_test(NAME terrain_asset_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_terrain_assets.py
        --target native --native-probe $<TARGET_FILE:fist_asset_probe>
        --terrain-probe $<TARGET_FILE:fist_terrain_probe>)
    add_test(NAME terrain_scene_contract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tests/test_terrain_scene.py
        --target native --native-preview $<TARGET_FILE:fist_terrain_preview>)
endif()
