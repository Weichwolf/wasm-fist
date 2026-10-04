import json
import unittest

from test_port_io import ROOT
from device_cpu_fixture import DIRECTORY, commands, original, check


class DeviceResetTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = DIRECTORY
        cls.commands = commands()
        cls.source, cls.controlled = original()
        cls.case = json.loads((ROOT/'tools/oracle/device_reset_case.json').read_text())

    def check_entry(self, first):
        rows = self.case['fetches'][first:]
        label = '%04x' % rows[0]['cpu_regs.ip.dword[0]']
        check(self, rows, self.source/'source'/(label+'.memory'),
              self.source/'source/77ff.memory')

    def test_actual_tail_dispatch_matches_full_original_reset_sequence(self):
        self.check_entry(0)

    def test_actual_reset_entry_matches_full_original_sequence(self):
        self.check_entry(6)
