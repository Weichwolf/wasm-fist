#!/usr/bin/env python3
"""Complete preview frames, periodic world placement, assets and camera behavior."""
import argparse
import pathlib
import struct
import subprocess
import tempfile
import unittest

from prepare_terrain_preview import original_inputs, synthetic_inputs

ROOT = pathlib.Path(__file__).resolve().parents[2]
BUILD = pathlib.Path("/tmp/wasm-fist-rewrite")
TARGET = "all"
NATIVE_PREVIEW = None
ORIGINALS = False


def ppm_rgb(data):
    header, width_height, maximum, pixels = data.split(b"\n", 3)
    if (header, width_height, maximum) != (b"P6", b"640 400", b"255") or len(pixels) != 640 * 400 * 3:
        raise ValueError("Missing or incomplete 640x400 P6 output")
    return pixels


class TerrainSceneTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="wasm-fist-terrain-scene-", dir="/tmp")
        cls.addClassCleanup(cls.temp.cleanup)
        cls.directory = pathlib.Path(cls.temp.name)
        cls.scenario = cls.directory / synthetic_inputs(cls.directory)
        cls.commands = []
        if TARGET in ("all", "native"):
            cls.commands.append([str(NATIVE_PREVIEW or BUILD / "native/fist_terrain_preview")])
        if TARGET in ("all", "wasm"):
            cls.commands.append(["node", str(ROOT / "tools/rewrite/check_terrain_wasm.cjs"),
                                 str(BUILD / "wasm/fist_terrain_preview.js")])

    def render(self, command, scenario=None, directory=None, heading=None, valid=True):
        output = self.directory / "frame.ppm"
        output.unlink(missing_ok=True)
        args = [*command, str(scenario or self.scenario), str(directory or self.directory), str(output)]
        if heading is not None:
            args.append(str(heading))
        result = subprocess.run(args, capture_output=True, text=True, timeout=30)
        self.assertEqual(result.returncode, 0 if valid else 1, result.stderr)
        if not valid:
            self.assertEqual(result.stdout, "")
            self.assertFalse(output.exists(), "Failure must not publish a frame")
            return None
        self.assertRegex(result.stdout, r"^terrain inspection: 640x400 RGBA8 fnv1a=[0-9a-f]{8}\n$")
        return ppm_rgb(output.read_bytes())

    def test_full_frame_contains_sky_and_textured_terrain(self):
        for command in self.commands:
            with self.subTest(target=command[0]):
                pixels = self.render(command)
                sky = pixels[:3]
                self.assertNotEqual(sky, b"\0\0\0")
                self.assertEqual(pixels[:640 * 3], sky * 640)
                bottom = pixels[-640 * 3:]
                self.assertNotIn(sky, [bottom[x:x + 3] for x in range(0, len(bottom), 3)])
                self.assertGreater(len({pixels[x:x + 3] for x in range(0, len(pixels), 3)}), 256)

    def test_heading_changes_view_and_full_map_period_repeats(self):
        shifted = self.directory / "SHIFT.FSG"
        data = bytearray(self.scenario.read_bytes())
        # SHDR begins at byte 6; original position fields begin at header byte 6.
        x, y = struct.unpack_from("<2i", data, 12)
        struct.pack_into("<2i", data, 12, x + 524288, y - 524288)
        shifted.write_bytes(data)
        try:
            for command in self.commands:
                with self.subTest(target=command[0]):
                    frame = self.render(command)
                    self.assertEqual(frame, self.render(command, scenario=shifted))
                    self.assertNotEqual(frame, self.render(command, heading=16384))
                    self.assertNotEqual(frame, self.render(command, heading=32768))
        finally:
            shifted.unlink()

    def test_height_field_changes_visible_surface(self):
        with tempfile.TemporaryDirectory(prefix="flat-", dir=self.temp.name) as directory:
            directory = pathlib.Path(directory)
            scenario = directory / synthetic_inputs(directory, flat=True)
            for command in self.commands:
                with self.subTest(target=command[0]):
                    hills = self.render(command)
                    flat = self.render(command, scenario=scenario, directory=directory)
                    self.assertEqual(hills[:640 * 3], flat[:640 * 3])
                    self.assertNotEqual(hills[640 * 3:], flat[640 * 3:])

    def test_signed_coordinate_limits_wrap_like_positive_domain(self):
        scenarios = []
        for name, positions in [("LIMIT.FSG", (-(2**31), 2**31 - 1)),
                                ("DOMAIN.FSG", (0, 524287))]:
            data = bytearray(self.scenario.read_bytes())
            struct.pack_into("<2i", data, 12, *positions)
            path = self.directory / name
            path.write_bytes(data)
            scenarios.append(path)
        try:
            for command in self.commands:
                with self.subTest(target=command[0]):
                    self.assertEqual(self.render(command, scenario=scenarios[0]),
                                     self.render(command, scenario=scenarios[1]))
        finally:
            for scenario in scenarios:
                scenario.unlink()

    def test_invalid_heading_and_missing_required_input_fail(self):
        for command in self.commands:
            for heading in ["-1", "65536", "1x", ""]:
                with self.subTest(target=command[0], heading=heading):
                    self.render(command, heading=heading, valid=False)
        sky = self.directory / "5.SKY"
        data = sky.read_bytes()
        sky.unlink()
        try:
            for command in self.commands:
                self.render(command, valid=False)
        finally:
            sky.write_bytes(data)

    def test_pinned_original_azer1_scene(self):
        if not ORIGINALS:
            self.skipTest("Local provisioned original scene requested separately")
        with tempfile.TemporaryDirectory(prefix="original-", dir=self.temp.name) as directory:
            directory = pathlib.Path(directory)
            scenario = directory / original_inputs(ROOT / "armoredfist/FISTDATA/AZER1.FSG", directory, BUILD)
            for command in self.commands:
                with self.subTest(target=command[0]):
                    frame = self.render(command, scenario=scenario, directory=directory, heading=26729)
                    self.assertGreater(len({frame[x:x + 3] for x in range(0, len(frame), 3)}), 256)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-root", type=pathlib.Path, default=BUILD)
    parser.add_argument("--target", choices=["all", "native", "wasm"], default=TARGET)
    parser.add_argument("--native-preview", type=pathlib.Path)
    parser.add_argument("--originals", action="store_true")
    args, remaining = parser.parse_known_args()
    BUILD, TARGET, NATIVE_PREVIEW, ORIGINALS = args.build_root, args.target, args.native_preview, args.originals
    unittest.main(argv=[__file__, *remaining])
