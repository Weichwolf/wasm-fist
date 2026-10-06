#!/usr/bin/env python3
"""KLC planes, resource members and VGA palettes: complete cross-target contracts."""
import argparse
import hashlib
import json
import pathlib
import random
import struct
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path("/tmp/wasm-fist-rewrite")
TARGET = "all"
NATIVE_PROBE = None
ORIGINALS = False
PALETTE = bytes(range(256)) * 3  # Embedded KLC bytes are not six-bit DAC components.
KEY = bytes.fromhex("ad de ed ac")


def make_klc(width, height, blocks):
    """Encode supplied block fixtures and independently place their expected pixels."""
    if len(blocks) != width * height // 16:
        raise ValueError("Wrong block fixture count")
    payload = bytearray()
    for index in range(0, len(blocks), 4):
        group = blocks[index:index + 4]
        control = sum(mode << (slot * 2) for slot, (mode, _, _) in enumerate(group))
        # The original ignores unused control pairs, even when they are nonzero.
        control |= (0xff << (len(group) * 2)) & 0xff
        payload.append(control)
        payload.extend(b"".join(encoded for _, encoded, _ in group))
    rows = [[0] * width for _ in range(height)]
    for index, (_, _, pixels) in enumerate(blocks):
        y, x = divmod(index, width // 4)
        for row in range(4):
            rows[y * 4 + row][x * 4:x * 4 + 4] = pixels[row * 4:row * 4 + 4]
    expected = f"{width} {height}\n".encode() + PALETTE + bytes(sum(rows, []))
    return b"KLC1" + struct.pack("<II", width, height) + PALETTE + payload, expected


def block_fixtures():
    # Every two-bit selector, including implicit zero, in a deliberately asymmetric tile.
    four = [3, 2, 1, 0, 0, 1, 2, 3, 1, 1, 3, 0, 2, 3, 0, 2]
    four_values = [0, 13, 201, 255]
    four_mask = sum(value << (index * 2) for index, value in enumerate(four))
    two = [1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1]
    two_mask = sum(value << index for index, value in enumerate(two))
    return [(0, bytes(four_values[1:]) + struct.pack("<I", four_mask),
             [four_values[value] for value in four]),
            (1, b"\xff\0" + struct.pack("<H", two_mask),
             [255 if value == 0 else 0 for value in two]),
            (2, b"\xe1", [225] * 16),
            (3, b"", [0] * 16),
            (2, b"\0" + bytes(range(16)), list(range(16))),
            (1, b"\x11\xf2\0\x80", [17] * 15 + [242])]


def make_resource(members):
    offset = 16 + (len(members) + 1) * 16
    directory = bytearray()
    for name, data in members:
        canonical = bytes(value - 32 if value >= 96 else value
                          for value in name.encode("ascii")).ljust(12, b"\0")
        directory.extend(bytes(value ^ KEY[index % 4] for index, value in enumerate(canonical)))
        directory.extend(struct.pack("<I", offset))
        offset += len(data)
    directory.extend(bytes(12) + struct.pack("<I", offset))
    return (b"RESOURCE1\r\n\x1a" + struct.pack("<I", len(members)) + directory +
            b"".join(data for _, data in members))


class TerrainAssetTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="wasm-fist-terrain-assets-", dir="/tmp")
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ("all", "native"):
            cls.commands.append([str(NATIVE_PROBE or BUILD / "native/fist_asset_probe")])
        if TARGET in ("all", "wasm"):
            cls.commands.append(["node", str(BUILD / "wasm/fist_asset_probe.js")])

    def run_data(self, kind, data, expected=None, name=None, valid=True):
        filename = pathlib.Path(self.temp.name) / "fixture.bin"
        filename.write_bytes(data)
        for command in self.commands:
            with self.subTest(target=command[0], kind=kind, name=name):
                args = [*command, kind, str(filename)] + ([name] if name is not None else [])
                result = subprocess.run(args, capture_output=True, timeout=15)
                self.assertEqual(result.returncode, 0 if valid else 1, result.stderr.decode())
                self.assertEqual(result.stdout, expected if valid else b"")

    def test_klc_all_modes_selector_width_and_row_orientation(self):
        data, expected = make_klc(24, 4, block_fixtures())
        self.run_data("klc", data, expected)

    def test_klc_control_pairs_continue_across_block_rows(self):
        data, expected = make_klc(12, 8, block_fixtures())
        self.run_data("klc", data, expected)

    def test_klc_varied_block_arrangements_and_unused_control_pairs(self):
        randomizer = random.Random(0x643c)
        fixtures = block_fixtures()
        for width, height in [(4, 4), (8, 12), (20, 12), (28, 8)]:
            blocks = [randomizer.choice(fixtures) for _ in range(width * height // 16)]
            data, expected = make_klc(width, height, blocks)
            self.run_data("klc", data, expected)
        # A single control-only zero tile proves that no payload byte is required.
        data, expected = make_klc(4, 4, [fixtures[3]])
        self.run_data("klc", data, expected)

    def test_every_truncated_klc_prefix_fails_and_preserves_output(self):
        data, _ = make_klc(12, 8, block_fixtures())
        self.run_data("klc-prefixes", data, b"")

    def test_invalid_klc_signature_dimensions_overflow_and_trailing_data(self):
        data, _ = make_klc(12, 8, block_fixtures())
        malformed = [b"BAD!" + data[4:], data + b"\0", data[:780]]
        for width, height in [(0, 8), (12, 0), (6, 8), (12, 7),
                              (0xfffffffc, 0xfffffffc), (0xfffffffc, 4), (4, 0xfffffffc)]:
            malformed.append(data[:4] + struct.pack("<II", width, height) + data[12:])
        for case, changed in enumerate(malformed):
            with self.subTest(case=case):
                self.run_data("klc", changed, valid=False)

    def test_resource_all_members_case_folding_lengths_and_first_duplicate(self):
        members = [("a.pal", bytes(range(256))), ("EMPTY", b""),
                   ("12345678.PAL", b"tail\0\xff"), ("A.PAL", b"second match"),
                   ("^.PAL", b"DOS punctuation folding")]
        data = make_resource(members)
        for name, expected in [("A.PAL", members[0][1]), ("a.pal", members[0][1]),
                               ("empty", b""), ("12345678.pal", members[2][1]),
                               ("~.pal", members[4][1])]:
            self.run_data("resource", data, expected, name)

    def test_invalid_resource_directory_offsets_count_and_trailing_data(self):
        data = make_resource([("A.PAL", b"a"), ("B.PAL", b"b")])
        malformed = [b"BAD!" + data[4:], data[:31], data[:60], data + b"\0"]
        for position, value in [(12, 0xffffffff), (28, 63), (28, 65),
                                (44, 63), (44, len(data) + 1), (60, len(data) - 1),
                                (60, len(data) + 1)]:
            changed = bytearray(data)
            struct.pack_into("<I", changed, position, value)
            malformed.append(bytes(changed))
        for case, changed in enumerate(malformed):
            with self.subTest(case=case):
                self.run_data("resource", changed, name="A.PAL", valid=False)

    def test_resource_missing_or_invalid_names_fail_without_output(self):
        data = make_resource([("A.PAL", b"a")])
        for name in ["", "MISSING.PAL", "123456789.PALX", "a pal", "../A.PAL",
                     "C:A.PAL", "dir\\A.PAL", "\x7f.PAL"]:
            self.run_data("resource", data, name=name, valid=False)
        self.run_data("resource", make_resource([]), name="A.PAL", valid=False)

    def test_palette_complete_six_bit_rgb_and_invalid_components_or_lengths(self):
        data = bytes(range(64)) * 12
        self.run_data("palette", data, data)
        for length in [0, 1, 767, 769]:
            self.run_data("palette", (data + b"\0")[:length], valid=False)
        for index in [0, 383, 767]:
            changed = bytearray(data)
            changed[index] = 64
            self.run_data("palette", changed, valid=False)

    def test_null_arguments_and_repeatable_image_release(self):
        for command in self.commands:
            result = subprocess.run([*command, "contracts"], capture_output=True, timeout=15)
            self.assertEqual(result.returncode, 0, result.stderr.decode())
            self.assertEqual(result.stdout, b"")

    def test_all_pinned_original_planes_and_palette_members(self):
        if not ORIGINALS:
            self.skipTest("Provisioned original instruction oracle requested separately")
        from original_asset_oracle import OriginalAssetOracle
        oracle = OriginalAssetOracle()  # Missing dependency/reference fails; it never skips.
        manifest = json.loads((ROOT / "tools/rewrite/terrain_originals.json").read_text())
        directory = ROOT / "armoredfist/FISTDATA"
        files = sorted([*directory.glob("*.KLC"), *directory.glob("*.SKY"),
                        directory / "PAL.RES"])
        self.assertEqual([file.name for file in files], sorted(manifest["files"]))
        plane_bytes = 0
        for file in files:
            data = file.read_bytes()
            info = manifest["files"][file.name]
            self.assertEqual(len(data), info["size"])
            self.assertEqual(hashlib.sha256(data).hexdigest(), info["sha256"])
            if file.name == "PAL.RES":
                count, = struct.unpack_from("<I", data, 12)
                self.assertEqual(count, len(info["members"]))
                for name, member in info["members"].items():
                    expected = oracle.resource(data, name)
                    self.assertEqual(len(expected), member["size"])
                    self.assertEqual(hashlib.sha256(expected).hexdigest(), member["sha256"])
                    self.run_data("resource", data, expected, name.lower())
                    self.run_data("palette", expected, expected)
            else:
                self.assertEqual(struct.unpack_from("<II", data, 4),
                                 (info["width"], info["height"]))
                expected = oracle.klc(data)
                self.assertEqual(hashlib.sha256(expected).hexdigest(), info["decoded_sha256"])
                self.run_data("klc", data, expected)
                plane_bytes += info["width"] * info["height"]
            self.assertEqual(hashlib.sha256(file.read_bytes()).hexdigest(), info["sha256"])
        print(f"Original corpus: {len(files) - 1} complete KLC/SKY planes, "
              f"{plane_bytes} pixels, {len(manifest['files']['PAL.RES']['members'])} palettes; "
              f"full byte comparison required on {len(self.commands)} targets", flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-root", type=pathlib.Path, default=BUILD)
    parser.add_argument("--target", choices=["all", "native", "wasm"], default=TARGET)
    parser.add_argument("--native-probe", type=pathlib.Path)
    parser.add_argument("--originals", action="store_true")
    args = parser.parse_args()
    BUILD, TARGET, NATIVE_PROBE, ORIGINALS = (args.build_root, args.target,
                                             args.native_probe, args.originals)
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or
                     (ORIGINALS and bool(program.result.skipped)))
