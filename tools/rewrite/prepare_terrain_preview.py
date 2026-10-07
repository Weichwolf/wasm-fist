#!/usr/bin/env python3
"""Prepare isolated original or constructed terrain-preview inputs under /tmp."""
import argparse
import hashlib
import json
import math
import pathlib
import shutil
import struct
import subprocess

from test_scenario import envelope, synthetic_chunks
from test_terrain_assets import make_klc
from test_units import snapshot

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path("/tmp/wasm-fist-rewrite")


def synthetic_inputs(directory, *, flat=False, positions=(128 * 256, 1024 * 256)):
    """Deliberately asymmetric height/color fixtures; no original bytes are embedded."""
    directory.mkdir(parents=True, exist_ok=True)
    chunks = synthetic_chunks()
    header = bytearray(chunks[0][1])
    struct.pack_into("<2i", header, 6, *positions)
    chunks[0] = (b"SHDR", header)
    # Static object first, player vehicle second: record order is not player identity.
    records = [(7, 11, snapshot(26, flags=0, pose=(100 * 256, 64 * 256, 0), heading=16384)),
               (9, 12, snapshot(0, pose=(*positions, 1280), heading=0))]
    chunks[1] = (b"DCBS", struct.pack("<H", len(records)) + b"".join(
        struct.pack("<3H", len(state), index, generation) + state
        for index, generation, state in records))
    (directory / "TEST.FSG").write_bytes(envelope(chunks))
    colors = [(0, 0, 0)] * 256
    colors[80:88] = [(34, 24, 12), (12, 32, 12), (16, 26, 40), (38, 16, 12),
                     (28, 36, 18), (24, 16, 8), (18, 30, 28), (42, 32, 18)]
    palette = bytes(component for color in colors for component in color)
    embedded = bytes(component * 4 for color in colors for component in color)
    (directory / "532.PAL").write_bytes(palette)

    def encode_plane(name, side, sampler):
        blocks = []
        for row in range(0, side, 4):
            for column in range(0, side, 4):
                pixels = [sampler(column + x, row + y) for y in range(4) for x in range(4)]
                blocks.append((2, b"\0" + bytes(pixels), pixels))
        data, _ = make_klc(side, side, blocks)
        # make_klc supplies its own decoder-test palette; this fixture supplies ours.
        (directory / name).write_bytes(data[:12] + embedded + data[780:])

    def height(column, row):
        if flat:
            return 32
        # Continuous periodic hills with different frequencies on the two axes.
        return round(48 + 22 * math.sin(column * math.tau / 256) +
                     18 * math.cos(row * math.tau / 128) +
                     14 * math.sin((column + 2 * row) * math.tau / 256))

    def color(column, row):
        return [80, 81, 83, 84][(column // 64 + 2 * (row // 64)) % 4]

    encode_plane("D32.KLC", 256, height)
    encode_plane("C32.KLC", 512, color)
    encode_plane("5.SKY", 4, lambda column, row: 82)
    return "TEST.FSG"


def original_inputs(scenario, directory, build_root=BUILD, *, vehicle=False):
    """Let the shared C readers resolve and validate every required original input."""
    scenario = scenario.resolve()
    source = scenario.parent
    scenario_manifest = json.loads((ROOT / "tools/rewrite/scenario_originals.json").read_text())
    asset_manifest = json.loads((ROOT / "tools/rewrite/terrain_originals.json").read_text())["files"]
    expected = scenario_manifest[scenario.name.upper()]
    verify_file(scenario, expected)
    result = subprocess.run([str(build_root / "native/fist_scenario_probe"), str(scenario)],
                            check=True, capture_output=True, text=True, timeout=15)
    names = {line.split(" ", 2)[2].upper() for line in result.stdout.splitlines()
             if line.startswith("asset ")}
    palette = next(name for name in names if name.endswith(".PAL"))
    if not (source / palette).is_file():
        names.remove(palette)
        names.add("PAL.RES")
    for name in names:
        verify_file(source / name, asset_manifest[name])
    subprocess.run([str(build_root / "native/fist_terrain_probe"), str(scenario), str(source)],
                   check=True, stdout=subprocess.DEVNULL, timeout=15)
    subprocess.run([str(build_root / "native/fist_unit_probe"), str(scenario)],
                   check=True, stdout=subprocess.DEVNULL, timeout=15)
    directory.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(scenario, directory / scenario.name.upper())
    for name in names:
        shutil.copyfile(source / name, directory / name)
        verify_file(source / name, asset_manifest[name])
        verify_file(directory / name, asset_manifest[name])
    if vehicle:
        models = json.loads((ROOT / 'tools/rewrite/model_originals.json').read_text())
        for family in ('M1_C', 'M3_C', 'T80_C', 'BMP_C'):
            for suffix in ('MAL', 'M00', 'M08', 'M16', 'M32'):
                name = family + '.' + suffix
                verify_file(source / name, models[name])
                shutil.copyfile(source / name, directory / name)
                verify_file(directory / name, models[name])
                verify_file(source / name, models[name])
    verify_file(scenario, expected)
    verify_file(directory / scenario.name.upper(), expected)
    return scenario.name.upper()


def verify_file(path, expected):
    data = path.read_bytes()
    if len(data) != expected["size"] or hashlib.sha256(data).hexdigest() != expected["sha256"]:
        raise ValueError(f"Changed or incomplete original: {path}")


def write_manifest(directory, scenario, heading=None, *, vehicle=False, mission=False):
    files = []
    for path in sorted(directory.iterdir()):
        if path.is_file() and path.suffix.upper() in (".FSG", ".KLC", ".SKY", ".PAL", ".RES", ".MAL", ".M00", ".M08", ".M16", ".M32"):
            data = path.read_bytes()
            files.append(dict(name=path.name, size=len(data), sha256=hashlib.sha256(data).hexdigest()))
    manifest = dict(scenario=scenario, files=files)
    if vehicle:
        manifest["vehicle"] = True
    if mission:
        manifest["mission"] = True
    if heading is not None:
        manifest["heading"] = heading
    (directory / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--vehicle", action="store_true", help="Draw the selected original ground vehicle; requires --scenario")
    parser.add_argument("--scenario", type=pathlib.Path, help="Pinned local original; default: constructed scene")
    parser.add_argument("--output-dir", type=pathlib.Path, default=BUILD / "wasm/assets")
    parser.add_argument("--build-root", type=pathlib.Path, default=BUILD)
    parser.add_argument("--heading", type=int, choices=range(65536), metavar="0..65535")
    args = parser.parse_args()
    output = args.output_dir.resolve()
    if not output.is_relative_to(pathlib.Path("/tmp")) or output == pathlib.Path("/tmp"):
        parser.error("Preview inputs belong in a dedicated directory under /tmp")
    if output.exists() and any(output.iterdir()):
        parser.error("Output directory must be empty; do not mix old or unrelated inputs")
    if args.vehicle and args.scenario is None:
        parser.error("Original vehicle preview requires a pinned --scenario")
    scenario = original_inputs(args.scenario, output, args.build_root, vehicle=args.vehicle) if args.scenario else synthetic_inputs(output)
    write_manifest(output, scenario, args.heading, vehicle=args.vehicle)
    print(f"Prepared {scenario} in {output}")


if __name__ == "__main__":
    main()
