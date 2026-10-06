#!/usr/bin/env python3
"""Complete cross-target FSG metadata/framing checks against independent format evidence."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
TAGS = [b"SHDR", b"DCBS", b"PATH", b"STMP", b"PINF", b"BINF", b"TERM"]
BUILD = pathlib.Path("/tmp/wasm-fist-rewrite")
ORIGINALS = False
TARGET = "all"
NATIVE_PROBE = None


def envelope(chunks):
    return b"".join(tag + struct.pack("<H", len(body)) + body for tag, body in chunks)


def synthetic_chunks():
    # Constructed test data, not copied original game content. Exercise signed full-width values.
    positions = [-(2**31), 2**31 - 1, -1234567, 2345678, -1, 0, 65536, -65537]
    header = struct.pack("<H4B8i", 0, 1, 0, 0, 30, *positions) + bytes(range(16))
    records = [(7, 11, bytes(range(55))), (9, 12, bytes(range(251)))]
    units = struct.pack("<H", len(records)) + b"".join(
        struct.pack("<3H", len(state), index, value) + state for index, value, state in records)
    names = [b"D32.KLC ", b"C32.KLC ", b"532.pal", b"5.SKY "]
    info = b"".join(name.ljust(16, b"\0") for name in names) + bytes(range(6))
    return list(zip(TAGS, [header, units, bytes(2144), bytes(258), bytes(176), info, b""]))


def expected_transcript(data):
    # Reference interpretation: d501/d7b5/d7e1/d8d3, checked against original machine code.
    chunks = {}
    offset = 0
    while offset < len(data):
        tag = data[offset:offset + 4]
        length, = struct.unpack_from("<H", data, offset + 4)
        chunks[tag] = data[offset + 6:offset + 6 + length]
        offset += 6 + length
    header = chunks[b"SHDR"]
    version, = struct.unpack_from("<H", header)
    lines = [f"header {version} {header[2]} {header[5]}"]
    lines += [f"position {index} {value}" for index, value
              in enumerate(struct.unpack_from("<8i", header, 6))]
    info = chunks[b"BINF"]
    for index in range(4):
        name = info[index * 16:(index + 1) * 16].split(b"\0")[0].replace(b" ", b"")
        lines.append(f"asset {index} {name.decode('ascii')}")
    units = chunks[b"DCBS"]
    count, = struct.unpack_from("<H", units)
    lines.append(f"units {count}")
    offset = 2
    for _ in range(count):
        length, index, value = struct.unpack_from("<3H", units, offset)
        state = units[offset + 6:offset + 6 + length]
        lines.append(f"unit {index} {value} {length} {state.hex(' ')}")
        offset += 6 + length
    assert offset == len(units)
    for index, tag in enumerate(TAGS):
        body = chunks[tag]
        suffix = " " + body.hex(" ") if body else ""
        lines.append(f"chunk {index} {len(body)}{suffix}")
    return "\n".join(lines) + "\n"


class ScenarioTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="wasm-fist-scenario-", dir="/tmp")
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if TARGET in ("all", "native"):
            cls.commands.append([str(NATIVE_PROBE or BUILD / "native/fist_scenario_probe")])
        if TARGET in ("all", "wasm"):
            cls.commands.append(["node", str(BUILD / "wasm/fist_scenario_probe.js")])

    def run_data(self, data, valid=True):
        filename = pathlib.Path(self.temp.name) / "fixture.fsg"
        filename.write_bytes(data)
        for command in self.commands:
            with self.subTest(target=command[0]):
                result = subprocess.run([*command, str(filename)], capture_output=True,
                                        text=True, timeout=10)
                if valid:
                    self.assertEqual(result.returncode, 0, result.stderr)
                    self.assertEqual(result.stdout, expected_transcript(data))
                else:
                    self.assertNotEqual(result.returncode, 0)
                    self.assertEqual(result.stdout, "")

    def test_full_width_positions_names_records_and_all_chunk_bytes(self):
        self.run_data(envelope(synthetic_chunks()))

    def test_every_truncated_prefix_fails_and_preserves_output(self):
        filename = pathlib.Path(self.temp.name) / "fixture.fsg"
        filename.write_bytes(envelope(synthetic_chunks()))
        for command in self.commands:
            result = subprocess.run([*command, "--prefixes", str(filename)],
                                    capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout, "")

    def test_zero_unit_count_is_valid(self):
        chunks = synthetic_chunks()
        chunks[1] = (b"DCBS", b"\0\0")
        self.run_data(envelope(chunks))

    def test_full_width_space_padded_sky_without_nul(self):
        chunks = synthetic_chunks()
        info = chunks[5][1]
        chunks[5] = (b"BINF", info[:48] + b"5.SKY".ljust(16, b" ") + info[64:])
        self.run_data(envelope(chunks))

    def test_unknown_chunk_is_skipped_by_original_length_contract(self):
        chunks = synthetic_chunks()
        chunks.insert(2, (b"TEST", b"opaque extension"))
        self.run_data(envelope(chunks))

    def test_missing_duplicate_or_out_of_place_known_chunks_fail(self):
        chunks = synthetic_chunks()
        for index in range(len(chunks)):
            with self.subTest(missing=TAGS[index]):
                self.run_data(envelope(chunks[:index] + chunks[index + 1:]), valid=False)
        self.run_data(envelope(chunks[:2] + [chunks[1]] + chunks[2:]), valid=False)
        self.run_data(envelope([chunks[1], chunks[0], *chunks[2:]]), valid=False)

    def test_bad_lengths_records_terminator_and_names_fail(self):
        chunks = synthetic_chunks()
        malformed = [envelope(chunks) + b"trailing", envelope(chunks[:-1] + [(b"TERM", b"x")])]
        for tag, body in [(b"SHDR", chunks[0][1][:-1]), (b"BINF", chunks[5][1][:-1]),
                          (b"DCBS", b"\x03\0" + chunks[1][1][2:]),
                          (b"DCBS", b"\x01\0" + chunks[1][1][2:]),
                          (b"DCBS", b"\0"), (b"DCBS", b"\x01\0" + b"\xff" * 6),
                          (b"DCBS", b"\x01\0\x01\0\0\0\0\0x"),
                          (b"BINF", b"X" * 16 + chunks[5][1][16:]),
                          (b"BINF", bytes(16) + chunks[5][1][16:]),
                          (b"BINF", b" " * 15 + b"\0" + chunks[5][1][16:])]:
            changed = [(name, body if name == tag else old) for name, old in chunks]
            malformed.append(envelope(changed))
        for index, data in enumerate(malformed):
            with self.subTest(case=index):
                self.run_data(data, valid=False)

    def test_all_pinned_original_scenarios(self):
        if not ORIGINALS:
            self.skipTest("Local provisioned original coverage requested separately")
        manifest = json.loads((ROOT / "tools/rewrite/scenario_originals.json").read_text())
        files = sorted((ROOT / "armoredfist/FISTDATA").glob("*.FSG"))
        self.assertEqual([file.name for file in files], sorted(manifest))
        for file in files:
            with self.subTest(file=file.name):
                data = file.read_bytes()
                self.assertEqual(hashlib.sha256(data).hexdigest(), manifest[file.name]["sha256"])
                self.assertEqual(len(data), manifest[file.name]["size"])
                self.run_data(data)
                self.assertEqual(hashlib.sha256(file.read_bytes()).hexdigest(),
                                 manifest[file.name]["sha256"])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-root", type=pathlib.Path, default=BUILD)
    parser.add_argument("--originals", action="store_true")
    parser.add_argument("--target", choices=["all", "native", "wasm"], default=TARGET)
    parser.add_argument("--native-probe", type=pathlib.Path)
    args = parser.parse_args()
    BUILD, ORIGINALS = args.build_root, args.originals
    TARGET, NATIVE_PROBE = args.target, args.native_probe
    program = unittest.main(argv=[__file__], exit=False)
    raise SystemExit(not program.result.wasSuccessful() or
                     (ORIGINALS and bool(program.result.skipped)))
