import importlib.util
from pathlib import Path
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location('cpu_trace', Path(__file__).resolve().parents[1] / 'tools/oracle/cpu_trace.py')
TRACE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(TRACE)


class CpuTraceTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.path = Path(self.tmp.name) / 'trace'
        registers = ' '.join(['00000000'] * 8)
        segments = ' '.join(['0000:00000000', '002b:00000000'] + ['0000:00000000'] * 4)
        self.data = ('FISTCPU1 1 2\n' +
                     f'I 1 30000 0 29999 002b 000011dd 000011dd {registers} {segments}\n' +
                     f'I 1 30000 0 29997 002b 00007135 00007135 {registers} {segments}\n' +
                     'E 2 2\n')

    def analyze(self, data):
        self.path.write_text(data)
        return TRACE.interval(self.path, (0x2b, 0x11dd), (0x2b, 0x7135))

    def test_accounts_for_interval_and_validates_the_entire_window(self):
        result = self.analyze(self.data)
        self.assertEqual((result['instructions'], result['cycles'], result['extra_cycles']), (1, 2, 1))
        self.assertEqual(result['gaps'][0]['cs_ip'], '002b:000011dd')
        self.assertEqual(result['gaps'][0]['cycles'], 2)

    def test_missing_truncated_or_invalid_evidence_fails(self):
        for data in (self.data.rsplit('E', 1)[0], self.data.replace('E 2 2', 'E 1 2'),
                     self.data.replace('E 2 2', 'E 2 1'), self.data + 'junk\n',
                     self.data.replace('29997', '30000'), self.data.replace('30000 0 29997', '40000 0 29997'),
                     self.data.replace('00007135 00007135', '00007135 00000000'),
                     self.data.replace('00007135 00007135', '00007136 00007136')):
            with self.subTest(data=data), self.assertRaises(ValueError):
                self.analyze(data)

    def test_original_zero_count_rep_and_ss_interrupt_shadow_allow_equal_times(self):
        result = self.analyze(self.data.replace('29997', '29999'))
        self.assertEqual((result['instructions'], result['cycles'], result['extra_cycles']), (1, 0, -1))

    def test_linear_instruction_address_uses_original_32_bit_width(self):
        result = self.analyze(self.data.replace('002b:00000000', '002b:fffffff0')
                              .replace('000011dd 000011dd', '000011dd 000011cd')
                              .replace('00007135 00007135', '00007135 00007125'))
        self.assertEqual(result['cycles'], 2)


if __name__ == '__main__':
    unittest.main()
