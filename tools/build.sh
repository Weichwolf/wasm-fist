#!/usr/bin/env bash
set -euo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
output=${FIST_REWRITE_BUILD_ROOT:-/tmp/wasm-fist-rewrite}
export PYTHONPYCACHEPREFIX=${PYTHONPYCACHEPREFIX:-/tmp/wasm-fist-python-cache}
case "${1:-all}" in
    native|wasm|all) target=${1:-all} ;;
    *) echo "Usage: $0 [native|wasm|all]" >&2; exit 2 ;;
esac
if [[ $target == native || $target == all ]]; then
    cmake -S "$root" -B "$output/native" -G Ninja \
        -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build "$output/native"
    ctest --test-dir "$output/native" --output-on-failure --no-tests=error
fi
if [[ $target == wasm || $target == all ]]; then
    emcmake cmake -S "$root" -B "$output/wasm" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build "$output/wasm"
    timeout 30s node "$root/tests/check_wasm.cjs" "$output/wasm/fist_renderer_probe.js"
    python3 "$root/tests/test_scenario.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_ground_command.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_ground_bearing.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_ground_throttle.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_ground_goal.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_ground_route.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_geometry.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_proximity.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_target_discovery.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_target_acquisition.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_roster_promotion.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_orders.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_units.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_vehicle_start.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_weapon_control.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_vehicle_damage.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_primary_fire.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_mission_world.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_mission_ready.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_mission_combat.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_world_step.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_aircraft_death.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_destruction.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_other_damage.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_projectile_flight.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_collision.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_projectile_launch.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_object_pool.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_vehicle_maintenance.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_vehicle_history.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_vehicle_motion.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_mission_driving.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_driving.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_vehicle.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_model_bitmap.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_models.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_ground.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_visibility.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_heightfield.py" --target wasm --build-root "$output"
    python3 "$root/tests/test_terrain_assets.py" --target wasm --build-root "$output"
    timeout 30s node "$root/tests/check_wasm.cjs" "$output/wasm/fist_vehicle_scene_probe.js"
    python3 "$root/tests/test_terrain_scene.py" --target wasm --build-root "$output"
fi
