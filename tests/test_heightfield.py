#!/usr/bin/env python3
"""Full owned height-field outputs against independent fixtures/original returns."""
import argparse
import hashlib
import json
import pathlib
import random
import struct
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path("/tmp/wasm-fist-rewrite")
TARGET = "all"
NATIVE_PROBE = None
ORIGINALS = False
MEMORY_TESTS = True
PALETTE = bytes(range(256)) * 3


def resize(side, pixels, targets):
    """Independent separable interpolation: expand each axis as complete rows.

    Computing two independently rounded means matters at odd/odd coordinates.
    Neighbors wrap across both map seams. Decimation chooses original knots.
    """
    rows = [list(pixels[index:index + side]) for index in range(0, len(pixels), side)]
    for target in targets:
        while len(rows) < target:
            horizontal = []
            for row in rows:
                expanded = []
                for left, right in zip(row, row[1:] + row[:1]):
                    expanded.extend((left, (left + right) // 2))
                horizontal.append(expanded)
            rows = []
            for current, following in zip(horizontal, horizontal[1:] + horizontal[:1]):
                rows.extend((current, [(a + b) // 2 for a, b in zip(current, following)]))
        while len(rows) > target:
            rows = [row[::2] for row in rows[::2]]
    return len(rows), bytes(value for row in rows for value in row)


def memory_limit():
    import resource
    resource.setrlimit(resource.RLIMIT_AS, (64 * 1024 * 1024, 64 * 1024 * 1024))


class HeightfieldTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="wasm-fist-heightfield-", dir="/tmp")
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ("all", "native"):
            cls.commands.append([str(NATIVE_PROBE or BUILD / "native/fist_heightfield_probe")])
        if TARGET in ("all", "wasm"):
            cls.commands.append(["node", str(BUILD / "wasm/fist_heightfield_probe.js")])
        cls.oracle = None
        if ORIGINALS:
            from original_heightfield_oracle import OriginalHeightfieldOracle
            cls.oracle = OriginalHeightfieldOracle()  # A requested missing oracle fails.

    def run_plane(self, side, pixels, targets, palette=PALETTE, valid=True,
                  dimensions=None, limited=False, original=True):
        dimensions = dimensions or (side, side)
        request = (struct.pack("<III", *dimensions, len(targets)) +
                   struct.pack("<" + "I" * len(targets), *targets) + palette + pixels)
        filename = pathlib.Path(self.temp.name) / "input.bin"
        filename.write_bytes(request)
        expected = b""
        if valid:
            output_side, output = resize(side, pixels, targets)
            if self.oracle is not None and original:
                original_side, original_pixels = self.oracle.resample(side, pixels, targets)
                self.assertEqual(output_side, original_side)
                self.assertEqual(output, original_pixels)
                output = original_pixels
            expected = f"{output_side} {output_side}\n".encode() + palette + output
        for command in self.commands:
            with self.subTest(target=command[0], side=side, stages=targets):
                result = subprocess.run([*command, str(filename)], capture_output=True, timeout=60,
                                        preexec_fn=memory_limit if limited and command[0] != "node" else None)
                self.assertEqual(result.returncode, 0 if valid else 1, result.stderr.decode())
                self.assertEqual(result.stdout, expected)

    def test_null_alias_invalid_shapes_and_32_bit_overflow_contracts(self):
        for command in self.commands:
            result = subprocess.run([*command, "contracts"], capture_output=True, timeout=15)
            self.assertEqual(result.returncode, 0, result.stderr.decode())
            self.assertEqual(result.stdout, b"")

    def test_constant_one_cell_fields_and_independent_unchanged_copy(self):
        for value in (0, 1, 127, 128, 254, 255):
            self.run_plane(1, bytes([value]), [1])
            self.run_plane(1, bytes([value]), [16, 1, 4])

    def test_staged_rounding_differs_from_single_four_corner_mean(self):
        # Odd/odd must be 0: floor(mean(floor(mean(0,1)), floor(mean(1,2)))).
        # One direct average of all four corners would incorrectly give 1.
        pixels = bytes([0, 1, 1, 2])
        side, expected = resize(2, pixels, [4])
        self.assertEqual(side, 4)
        self.assertEqual(expected[5], 0)
        self.run_plane(2, pixels, [4])
        self.run_plane(2, bytes([0, 255, 255, 0]), [4, 16, 2])

    def test_every_byte_pair_and_both_periodic_seams(self):
        # Every possible a,b appears horizontally and vertically, including 9-bit carries.
        side = 512
        pixels = bytes(value for a in range(256) for _ in range(2)
                       for b in range(256) for value in (a, b))
        self.assertEqual(len(pixels), side * side)
        self.run_plane(side, pixels, [1024])
        transposed = bytes(pixels[column * side + row] for row in range(side) for column in range(side))
        self.run_plane(side, transposed, [1024])

    def test_reduction_even_knots_non_power_of_two_and_repeated_ownership(self):
        randomizer = random.Random(0xbc06)
        for side in (2, 3, 4, 12, 24, 32):
            pixels = bytes(randomizer.randrange(256) for _ in range(side * side))
            self.run_plane(side, pixels, [side * 8, side, side * 2, side])
            self.run_plane(side, pixels, [side])
            if side % 2 == 0:
                self.run_plane(side, pixels, [side // 2])
        self.run_plane(64, bytes(range(256)) * 16, [1, 32, 4, 16, 2, 64])

    def test_malformed_planes_targets_and_incomplete_input_fail_atomically(self):
        pixels = bytes(range(16))
        for targets in ([0], [3], [6], [0xffffffff], [8, 3]):
            self.run_plane(4, pixels, targets, valid=False)
        self.run_plane(4, pixels, [4], valid=False, dimensions=(2, 8))
        self.run_plane(4, pixels[:-1], [8], valid=False)
        self.run_plane(4, pixels + b"x", [8], valid=False)
        self.run_plane(4, pixels, [], valid=False)

    def test_real_intermediate_allocation_failure_preserves_input_and_output(self):
        if not MEMORY_TESTS:
            self.skipTest("Address-space limits conflict with sanitizer shadow memory")
        # C input/out preservation is checked inside the probe. Both heaps really fail;
        # native uses RLIMIT_AS and the WASM probe has a bounded 64 MiB heap.
        self.run_plane(1, b"\xff", [8192], valid=False, limited=True)

    def test_all_pinned_square_original_planes_and_runtime_height_sizes(self):
        if not ORIGINALS:
            self.skipTest("Original corpus/oracle is an explicit additional gate")
        manifest = json.loads((ROOT / "tests/terrain_originals.json").read_text())["files"]
        from original_asset_oracle import OriginalAssetOracle
        decoder = OriginalAssetOracle()
        count = pixels = height_maps = 0
        for name, info in manifest.items():
            if name == "PAL.RES" or info["width"] != info["height"]:
                continue
            with self.subTest(original=name):
                path = ROOT / "armoredfist/FISTDATA" / name
                data = path.read_bytes()
                self.assertEqual(hashlib.sha256(data).hexdigest(), info["sha256"])
                decoded = decoder.klc(data)
                self.assertEqual(hashlib.sha256(decoded).hexdigest(), info["decoded_sha256"])
                dimensions, body = decoded.split(b"\n", 1)
                side = info["width"]
                self.assertEqual(dimensions, f"{side} {side}".encode())
                palette, plane = body[:768], body[768:]
                targets = [side * 2, side // 2, side]
                self.run_plane(side, plane, [side * 2], palette)
                self.run_plane(side, plane, [side // 2], palette)
                self.run_plane(side, plane, targets, palette)
                count += 1
                pixels += side * side
                # Actual numerical height maps, not colormap palette interpolation.
                if name.startswith("D"):
                    height_maps += 1
                    for target in (512, 1024, 2048, 4096):
                        self.run_plane(side, plane, [target], palette)
                    self.run_plane(side, plane, [4096, side], palette)
                self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), info["sha256"])
        self.assertEqual(count, 18)
        self.assertEqual(height_maps, 8)
        print(f"Original resampling: {count} square planes, {pixels} source pixels, "
              f"{height_maps} height maps at all four runtime sizes (512/1024/2048/4096); "
              f"complete original returns/outputs on {len(self.commands)} targets", flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-root", type=pathlib.Path, default=BUILD)
    parser.add_argument("--target", choices=["all", "native", "wasm"], default=TARGET)
    parser.add_argument("--native-probe", type=pathlib.Path)
    parser.add_argument("--originals", action="store_true")
    parser.add_argument("--no-memory-tests", action="store_true")
    args = parser.parse_args()
    BUILD, TARGET, NATIVE_PROBE, ORIGINALS = args.build_root, args.target, args.native_probe, args.originals
    MEMORY_TESTS = not args.no_memory_tests
    program = unittest.main(argv=[__file__], exit=False)
    # Only the two explicitly disabled supplemental gates may be skipped.
    permitted_skips = (not ORIGINALS) + (not MEMORY_TESTS)
    raise SystemExit(not program.result.wasSuccessful() or len(program.result.skipped) != permitted_skips)
