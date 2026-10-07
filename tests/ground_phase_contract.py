"""Shared owned-state protocol and complete original comparison for ground phases."""
import pathlib
import struct
import subprocess
import tempfile
import unittest

from test_vehicle_start import state_lines


class GroundPhaseTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        build, target, native_probe, _ = cls.configuration()
        cls.temp = tempfile.TemporaryDirectory(prefix=f'wasm-fist-{cls.mode}-', dir='/tmp')
        cls.addClassCleanup(cls.temp.cleanup)
        cls.commands = []
        if target in ('all', 'native'):
            cls.commands.append([str(native_probe or build / 'native/fist_vehicle_motion_probe')])
        if target in ('all', 'wasm'):
            cls.commands.append(['node', str(build / 'wasm/fist_vehicle_motion_probe.js')])
        cls.cases = cls.boundaries = 0

    @classmethod
    def tearDownClass(cls):
        print(f'{cls.label} per target: {cls.cases} fixtures, {cls.boundaries} complete boundaries', flush=True)

    def run_data(self, data, wanted='', valid=True):
        path = pathlib.Path(self.temp.name) / 'request.bin'
        path.write_bytes(data)
        for command in self.commands:
            result = subprocess.run([*command, self.mode, str(path)], capture_output=True,
                                    text=True, timeout=60)
            self.assertEqual(result.returncode, int(not valid), result.stderr)
            if result.stdout != wanted:
                actual = result.stdout.splitlines()
                expected = wanted.splitlines()
                first = next((index for index, (a, b) in enumerate(zip(actual, expected)) if a != b), min(len(actual), len(expected)))
                self.fail(f'Complete {self.mode} state stream differs at line {first}; '
                          f'{len(result.stdout)} versus {len(wanted)} bytes')

    def run_cases(self, cases):
        observed = []
        for raw, steps in cases:
            for _ in range(steps):
                raw = self.update(raw)
                observed.append(raw)
        oracle = self.configuration()[3]
        if oracle is not None:
            self.assertEqual(getattr(oracle, self.original_method)(cases), observed,
                             'Complete original callback state differs')
        expected = state_lines([(0, 0, raw) for raw in observed])
        data = struct.pack('<I', len(cases)) + b''.join(raw + struct.pack('<H', steps) for raw, steps in cases)
        self.run_data(data, expected)
        self.__class__.cases += len(cases)
        self.__class__.boundaries += len(observed)
