"""Actual original startup bridge, matching physical fetch boundary and instruction contracts."""
import hashlib
import json
from pathlib import Path
import resource
import subprocess
import sys
import tempfile
import unittest
from test_port_io import ROOT
sys.path.insert(0, str(ROOT/'tools/oracle'))
from capture_cpu_task_gate import capture, observer, verify
from capture_cpu_task_gate_programs import capture as capture_programs
from capture_cpu_task_gate_flags import capture as capture_flags
from cpu_task_gate_programs import MODES, COUNTS
from cpu_task_gate_fixture import build, replay, observation

FIXTURES = ROOT
OWNER = ROOT/'re_out'
INCLUDES = ()
PREVIOUS = '3b094c769a4f308331f29a44c9e36a4ec22bea42'


class TaskGateTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.storage = tempfile.TemporaryDirectory(prefix='wasm-fist-task-gate-reaching-unit-')
        cls.addClassCleanup(cls.storage.cleanup)
        cls.directory = Path(cls.storage.name)
        limit = resource.getrlimit(resource.RLIMIT_CORE)
        resource.setrlimit(resource.RLIMIT_CORE, (0, limit[1]))
        cls.addClassCleanup(resource.setrlimit, resource.RLIMIT_CORE, limit)
        cls.source = cls.directory/'source'
        cls.case = capture(ROOT, cls.source, make_observer=observer, check_capture=verify,
            additional_inputs=(FIXTURES/'tools/oracle/capture_cpu_task_gate.py',
                ROOT/'re_out/fist_dat_image.bin', ROOT/'re_out/fist_image.bin',
                ROOT/'tools/oracle/cpu_trace.py', ROOT/'tools/oracle/resident_image.py',
                ROOT/'tools/oracle/sb_irq_frame_case.json',
                ROOT/'third_party/dosbox-build/dosbox-0.74-3/src/hardware/sblaster.cpp'),
            source_wall_seconds=180)
        cls.rows = cls.case['events'][:17]
        cls.commands = cls.build(cls.directory)

    @classmethod
    def build(cls, directory, includes=(), flags=()):
        return build(directory, driver=FIXTURES/'tests/cpu_task_gate.c',
                     include_dirs=(*includes, *INCLUDES), extra_flags=flags)

    def test_one_seed_complete17_states137_fetches_through_actual_DSP_OUT(self):
        results = replay(self.directory, self.source, self.commands, self.rows)
        self.assertEqual(len(results), 2)
        for q in results:
            self.assertEqual((q['boundaries'], q['fetches'], q['terminal_exit']), (17, 137, 0))
            self.assertTrue(q['CPU_time_equal'] and q['trace_equal'] and q['complete_context_equal'])
        self.assertEqual(self.rows[-1]['kind'], 'after-reset-write')
        self.assertEqual((self.rows[-2]['registers'][2] & 0xffff,
                          self.rows[-2]['registers'][0] & 0xff), (0x226, 1))
        self.assertEqual(self.rows[-1]['SB']['dsp.state'], 0)
        # SB snapshots are retained original evidence. This fixture does not yet
        # compare all portable SB state, mixer state or the rest of initialization.

    def test_previous_decoder_reaches_actual_missing_PUSH_immediate(self):
        directory = self.directory/'previous'
        directory.mkdir()
        for name in ('fist_exec.h', 'fist_cpu.h'):
            text = subprocess.check_output(['git', 'show', PREVIOUS+':re_out/'+name], cwd=ROOT, text=True)
            (directory/name).write_text(text)
        results = replay(directory, self.source, self.build(directory, (directory,)),
                         self.rows, complete=False, expected_rows=self.rows[:1])
        for q in results:
            self.assertNotEqual(q['terminal_exit'], 0)
            self.assertEqual((q['boundaries'], q['fetches']), (1, 1))
            self.assertTrue(q['CPU_time_equal'])
        self.assertTrue(self.rows[0]['opcode_hex'].startswith('666a00'))

    def test_missing_SS_credit_first_differs_only_in_reached_time_and_budget(self):
        directory = self.directory/'missing-credit'
        directory.mkdir()
        header = (OWNER/'fist_exec.h').read_text()
        marker = '   if(op==0x17)e->credit(e->opaque);\n'
        self.assertEqual(header.count(marker), 1)
        (directory/'fist_exec.h').write_text(header.replace(marker, ''))
        (directory/'fist_cpu.h').write_bytes((OWNER/'fist_cpu.h').read_bytes())
        results = replay(directory, self.source, self.build(directory, (directory,)),
                         self.rows[:10], strict=False)
        expected = [observation(q) for q in self.rows[:10]]
        for q in results:
            self.assertEqual(q['terminal_exit'], 0)
            self.assertFalse(q['CPU_time_equal'])
            self.assertTrue(q['complete_context_equal'])
            self.assertEqual(q['observations'][:9], expected[:9])
            a, b = q['observations'][-1].split(), expected[-1].split()
            self.assertEqual([i for i, (x, y) in enumerate(zip(a, b)) if x != y], [1, 3])
            self.assertEqual(int(a[1]), int(b[1])+1)
            self.assertEqual(int(a[3]), int(b[3])-1)

    def test_before_code_read_boundary_exposes_new_page_link_difference(self):
        directory = self.directory/'before-code-read'
        directory.mkdir()
        results = replay(directory, self.source,
            self.build(directory, flags=('-DFIST_TASK_GATE_BEFORE_FETCH',)),
            self.rows, compare_context=False)
        for q in results:
            self.assertTrue(q['CPU_time_equal'] and q['trace_equal'])
            differences = {a['boundary'] for a in q['artifacts']
                           if a['kind']=='context-diagnostic' and not a['equal']}
            self.assertEqual(differences, {'dispatcher-entry', 'device-entry'})
            self.assertFalse(q['complete_context_equal'])

    def test_unbound_IO_reaches_actual_OUT_and_fails_both_targets(self):
        directory = self.directory/'unbound-IO'
        directory.mkdir()
        results = replay(directory, self.source,
            self.build(directory, flags=('-DFIST_TASK_GATE_UNBOUND_IO',)),
            self.rows, complete=False, expected_rows=self.rows[:16])
        for q in results:
            self.assertNotEqual(q['terminal_exit'], 0)
            self.assertEqual((q['boundaries'], q['fetches']), (16, 136))
            self.assertTrue(q['CPU_time_equal'])

    def test_source_rejects_truncated_trace_missing_return_and_truncated_RAM(self):
        verify(ROOT, self.source)
        faults = [(self.source/'cpu.trace', lambda b: b[:128]),
            (self.source/'source/events.jsonl', lambda b: b'\n'.join(b.splitlines()[:-1])+b'\n'),
            (self.source/'source'/self.rows[0]['memory_file'], lambda b: b[:-1])]
        for path, mutate in faults:
            saved = path.read_bytes()
            try:
                path.write_bytes(mutate(saved))
                with self.assertRaises((AssertionError, ValueError)):
                    verify(ROOT, self.source)
            finally:
                path.write_bytes(saved)
            self.assertEqual(hashlib.sha256(path.read_bytes()).digest(), hashlib.sha256(saved).digest())
        verify(ROOT, self.source)


class TaskGateProgramsTest(unittest.TestCase):
    pass


def program_test(mode):
    def test(self):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-task-gate-programs-unit-') as directory:
            proof = capture_programs(ROOT, Path(directory)/'programs', mode, headers=OWNER)
            self.assertEqual(proof['cases'], COUNTS[mode])
            self.assertEqual(len(set(proof['outputs_sha256'].values())), 1)
            self.assertEqual(len(proof['causal_results']),
                             {'push-byte': 12, 'push-ss': 8, 'sub-word': 8, 'push-full': 8,
                              'pop-ss': 10, 'xchg': 12, 'shl': 12}[mode])
    return test


for mode in MODES:
    setattr(TaskGateProgramsTest, 'test_complete_original_'+mode.replace('-', '_')+'_programs', program_test(mode))


class TaskGateFlagsTest(unittest.TestCase):
    def check(self, mode, cases, faults):
        with tempfile.TemporaryDirectory(prefix='wasm-fist-task-gate-flags-unit-') as directory:
            proof = capture_flags(ROOT, Path(directory)/'flags', mode, headers=OWNER, sources=FIXTURES)
            self.assertEqual({q['programs'] for q in proof['results']}, {cases})
            self.assertEqual(len({q['sha256'] for q in proof['results']}), 1)
            self.assertEqual(len(proof['causal_results']), faults)

    def test_complete131360_original_SUBW_flags_and_INC_programs(self):
        self.check('sub-word', 131360, 12)

    def test_complete410816_original_SHL_flags_and_INC_programs(self):
        self.check('shl', 410816, 10)


if __name__ == '__main__':
    unittest.main()
