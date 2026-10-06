#!/usr/bin/env python3
"""Prepare verified original or constructed controlled-scene inputs under /tmp."""
import argparse
import pathlib

from prepare_terrain_preview import BUILD, original_inputs, synthetic_inputs, write_manifest
from test_terrain_scene import fixture_models
from test_units import scenario_data
from test_vehicle_motion import start


def synthetic_driving_inputs(directory):
    scenario = synthetic_inputs(directory)
    fixture_models(directory)
    raw = bytearray(start(x=128 * 256, y=1024 * 256))
    raw[22] |= 32
    raw[0xa9:0xac] = b'\x80\0\0'
    (directory / scenario).write_bytes(scenario_data([(9, 12, bytes(raw))]))
    return scenario


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--scenario', type=pathlib.Path, help='Pinned original; default: constructed player/terrain/models')
    parser.add_argument('--output-dir', type=pathlib.Path, default=BUILD / 'wasm/assets')
    parser.add_argument('--build-root', type=pathlib.Path, default=BUILD)
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if not output.is_relative_to(pathlib.Path('/tmp')) or output == pathlib.Path('/tmp'):
        parser.error('Preview inputs belong in a dedicated directory under /tmp')
    if output.exists() and any(output.iterdir()):
        parser.error('Output directory must be empty')
    scenario = (original_inputs(args.scenario, output, args.build_root, vehicle=True)
                if args.scenario else synthetic_driving_inputs(output))
    write_manifest(output, scenario, None, vehicle=True)
    print(f'Prepared controlled {scenario} in {output}')


if __name__ == '__main__':
    main()
