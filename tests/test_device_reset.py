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
        cls.rows = json.loads((cls.source/'proof.json').read_text())['fetches'][14:239]
        cls.assert_original_rows()

    @classmethod
    def assert_original_rows(cls):
        from memory_context_fixture import SYSTEM_FIELDS
        plain = lambda q:{k:v for k,v in q.items() if k not in (*SYSTEM_FIELDS,'memory_context','entry_return_ip')}
        assert [plain(q) for q in cls.rows] == [plain(q) for q in cls.case['fetches']]

    def check_entry(self, first):
        rows = self.rows[first:]
        label = '%04x' % rows[0]['cpu_regs.ip.dword[0]']
        check(self, rows, self.source/'source'/(label+'.memory'),
              self.source/'source/77ff.memory')

    def test_actual_tail_dispatch_matches_full_original_reset_sequence(self):
        self.check_entry(0)

    def test_actual_reset_entry_matches_full_original_sequence(self):
        self.check_entry(6)

    def test_omitted_opcode_read_reaches_cpu_ram_match_but_wrong_cache(self):
        header = (ROOT/'re_out/fist_ram.h').read_text()
        old = '    (void)fist_ram_resident_read(bus,1,ip,1);'
        self.assertEqual(header.count(old),1)
        directory = DIRECTORY/'omitted-opcode';directory.mkdir()
        (directory/'fist_ram.h').write_text(header.replace(old,'    /* omitted original Fetchb */'))
        runs = commands(directory)
        check(self,self.rows,self.source/'source/23c4.memory',self.source/'source/77ff.memory',
              runs=runs,context_equal=False)
