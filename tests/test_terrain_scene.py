#!/usr/bin/env python3
"""Complete preview frames, periodic world placement, assets and camera behavior."""
import argparse
import pathlib
import struct
import subprocess
import tempfile
import unittest

from prepare_terrain_preview import original_inputs, synthetic_inputs
from test_models import family_files, stream_records

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path("/tmp/wasm-fist-rewrite")
TARGET = "all"
NATIVE_PREVIEW = None
ORIGINALS = False


def ppm_rgb(data):
    header, width_height, maximum, pixels = data.split(b"\n", 3)
    if (header, width_height, maximum) != (b"P6", b"640 400", b"255") or len(pixels) != 640 * 400 * 3:
        raise ValueError("Missing or incomplete 640x400 P6 output")
    return pixels


def unit_state_offsets(data):
    offset = 0
    while offset < len(data):
        tag, length = struct.unpack_from("<4sH", data, offset)
        if tag == b"DCBS":
            count, = struct.unpack_from("<H", data, offset + 6)
            offsets = []
            cursor = offset + 8
            for _ in range(count):
                size, = struct.unpack_from("<H", data, cursor)
                offsets.append(cursor + 6)
                cursor += 6 + size
            assert cursor == offset + 6 + length
            return offsets
        offset += 6 + length
    raise ValueError("Missing unit snapshots in test scenario")


def fixture_models(directory, *, transparent=False):
    files = family_files()
    palette = bytearray(768)
    palette[3:6] = bytes((63, 0, 0))
    palette[6:9] = bytes((0, 0, 63))
    files['MAL'] = bytes(palette)
    for suffix in ('M00', 'M08', 'M16', 'M32'):
        records = []
        for original in stream_records(files[suffix]):
            data = bytearray(original)
            header = struct.unpack_from('<8H', data)
            if suffix != 'M00':
                for table in set(header[3:5]):
                    for part in range(2):
                        variants, = struct.unpack_from('<H', data, table + part * 2)
                        pointer, = struct.unpack_from('<H', data, variants)
                        data[pointer] = 1
                        struct.pack_into('<Hbb', data, pointer + 2, part, -16, 0)
            atlas = header[7]
            start = atlas + 12
            pixels = bytes(0 if transparent or column == 16 else (1 if row < 12 else 2)
                           for column in range(32) for row in range(24))
            data = (data[:atlas] + struct.pack('<HBBHBBHH', start, 32, 24,
                    start + len(pixels), 32, 24, start + 2 * len(pixels), 0) +
                    pixels + bytes(len(pixels)))
            struct.pack_into('<H', data, 0, len(data))
            records.append(data)
        files[suffix] = b''.join(records) + b'\0\0'
    for name in ('M1_C', 'M3_C', 'T80_C', 'BMP_C'):
        for suffix, data in files.items():
            (directory / (name + '.' + suffix)).write_bytes(data)


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
            cls.commands.append(["node", str(ROOT / "tests/check_terrain_wasm.cjs"),
                                 str(BUILD / "wasm/fist_terrain_preview.js")])

    def render(self, command, scenario=None, directory=None, heading=None, valid=True, vehicle=False):
        output = self.directory / "frame.ppm"
        output.unlink(missing_ok=True)
        args = [*command, str(scenario or self.scenario), str(directory or self.directory), str(output)]
        if vehicle:
            args += [str(heading) if heading is not None else 'default', 'vehicle']
        elif heading is not None:
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
        player = unit_state_offsets(data)[1]
        x, y = struct.unpack_from("<2i", data, player + 4)
        struct.pack_into("<2i", data, player + 4, x + 524288, y - 524288)
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
            struct.pack_into("<2i", data, unit_state_offsets(data)[1] + 4, *positions)
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

    def test_roster_pose_and_heading_control_view_not_header_or_first_record(self):
        changed = self.directory / "POSE.FSG"
        original = self.scenario.read_bytes()
        first, player = unit_state_offsets(original)
        try:
            for command in self.commands:
                with self.subTest(target=command[0]):
                    frame = self.render(command)
                    data = bytearray(original)
                    struct.pack_into("<2i", data, 12, -1234567, 2345678)  # Header only.
                    struct.pack_into("<3iH", data, first + 4, -1, 1, 99, 50000)
                    changed.write_bytes(data)
                    self.assertEqual(frame, self.render(command, scenario=changed))
                    data = bytearray(original)
                    struct.pack_into("<2i", data, player + 4, 80000, 90000)
                    changed.write_bytes(data)
                    self.assertNotEqual(frame, self.render(command, scenario=changed))
                    data = bytearray(original)
                    struct.pack_into("<H", data, player + 16, 16384)
                    changed.write_bytes(data)
                    turned = self.render(command, scenario=changed)
                    self.assertEqual(turned, self.render(command, heading=16384))
                    self.assertNotEqual(frame, turned)
        finally:
            changed.unlink(missing_ok=True)

    def test_missing_or_nonvehicle_roster_zero_fails_without_publishing(self):
        changed = self.directory / "NO_PLAYER.FSG"
        original = self.scenario.read_bytes()
        first, player = unit_state_offsets(original)
        cases = []
        data = bytearray(original)
        data[player + 22] = 0  # Vehicle does not participate in the roster.
        cases.append(data)
        data = bytearray(original)
        data[player + 27] = 1  # Slot four exists, slot zero is absent.
        cases.append(data)
        data = bytearray(original)
        data[player + 22] = 0
        data[first + 22] = 32  # Static object is assigned to slot zero instead.
        cases.append(data)
        try:
            for data in cases:
                changed.write_bytes(data)
                for command in self.commands:
                    self.render(command, scenario=changed, valid=False)
        finally:
            changed.unlink(missing_ok=True)

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


    def test_vehicle_transparency_scale_and_wrapped_scene_placement(self):
        with tempfile.TemporaryDirectory(prefix='vehicles-', dir=self.temp.name) as directory:
            directory = pathlib.Path(directory)
            scenario = directory / synthetic_inputs(directory, flat=True)
            original = bytearray(scenario.read_bytes())
            player = unit_state_offsets(original)[1]
            original[player + 169:player + 171] = bytes((128, 0))
            struct.pack_into('<H', original, player + 38, 0)
            original = bytes(original)
            scenario.write_bytes(original)
            for command in self.commands:
                fixture_models(directory, transparent=True)
                background = self.render(command, scenario=scenario, directory=directory, vehicle=True)
                fixture_models(directory)
                frames = []
                for kind in range(4):
                    data = bytearray(original)
                    struct.pack_into('<H', data, player, kind)
                    scenario.write_bytes(data)
                    frame = self.render(command, scenario=scenario, directory=directory, vehicle=True)
                    self.assertNotEqual(frame, background)
                    frames.append(frame)
                self.assertEqual(frames[0], frames[1])  # Equal original C-model world scale.
                counts = [sum(frame[index:index + 3] != background[index:index + 3]
                              for index in range(0, len(frame), 3)) for frame in frames]
                self.assertGreater(counts[2], counts[3])
                self.assertGreater(counts[3], counts[0])
                data = bytearray(original)
                x, y = struct.unpack_from('<2i', data, player + 4)
                struct.pack_into('<2i', data, player + 4, x + 524288, y - 524288)
                scenario.write_bytes(data)
                self.assertEqual(frames[0], self.render(command, scenario=scenario,
                                 directory=directory, vehicle=True))
                scenario.write_bytes(original)
                missing = directory / 'M1_C.M16'
                saved = missing.read_bytes()
                missing.unlink()
                self.render(command, scenario=scenario, directory=directory, vehicle=True, valid=False)
                missing.write_bytes(saved)
                data = bytearray(original)
                data[player + 169] = 127  # Absent variant; never clamp it into a valid pose.
                scenario.write_bytes(data)
                self.render(command, scenario=scenario, directory=directory, vehicle=True, valid=False)
                scenario.write_bytes(original)

    def test_pinned_original_vehicle_models_in_scene(self):
        if not ORIGINALS:
            self.skipTest('Local provisioned original vehicles requested separately')
        for name in ('AZER1.FSG', 'TRAIN1.FSG', 'INDIA3.FSG'):
            with tempfile.TemporaryDirectory(prefix='models-', dir=self.temp.name) as directory:
                directory = pathlib.Path(directory)
                scenario = directory / original_inputs(ROOT / 'armoredfist/FISTDATA' / name,
                              directory, BUILD, vehicle=True)
                saved_models = {path: path.read_bytes() for path in directory.iterdir()
                                if path.suffix in ('.M00', '.M08', '.M16', '.M32')}
                for command in self.commands:
                    for path, data in saved_models.items():
                        path.write_bytes(data)
                    frames = [self.render(command, scenario=scenario, directory=directory,
                                          heading=heading, vehicle=True)
                              for heading in (0, 16384, 32768, 49152)]
                    self.assertEqual(len(set(frames)), 4)
                    # Isolated input changes only: remove all model texels so the
                    # same four camera views prove the original vehicle was drawn.
                    for path, data in saved_models.items():
                        records = []
                        for record in stream_records(data):
                            cleared = bytearray(record)
                            atlas, = struct.unpack_from('<H', cleared, 14)
                            start, = struct.unpack_from('<H', cleared, atlas)
                            cleared[start:] = bytes(len(cleared) - start)
                            records.append(cleared)
                        path.write_bytes(b''.join(records) + b'\0\0')
                    empty = [self.render(command, scenario=scenario, directory=directory,
                                         heading=heading, vehicle=True)
                             for heading in (0, 16384, 32768, 49152)]
                    self.assertTrue(any(left != right for left, right in zip(frames, empty)),
                                    'Original vehicle must contribute visible texels')

    def test_pinned_original_vehicle_views(self):
        if not ORIGINALS:
            self.skipTest("Local provisioned original scene requested separately")
        for name, heading in [("AZER1.FSG", 26729), ("TRAIN1.FSG", 51700)]:
            with tempfile.TemporaryDirectory(prefix="original-", dir=self.temp.name) as directory:
                directory = pathlib.Path(directory)
                scenario = directory / original_inputs(ROOT / "armoredfist/FISTDATA" / name, directory, BUILD)
                for command in self.commands:
                    with self.subTest(target=command[0], scenario=name):
                        frame = self.render(command, scenario=scenario, directory=directory)
                        self.assertEqual(frame, self.render(command, scenario=scenario,
                                                           directory=directory, heading=heading))
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
