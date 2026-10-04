import json
import subprocess
import unittest
from pathlib import Path
from test_port_io import ROOT,build_pic_probe
from device_cpu_fixture import DIRECTORY,commands,original,check,clock


class DeviceConfigurationTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.commands = commands()
        cls.source,cls.controlled = original()
        cls.default_rows = json.loads((cls.source/'proof.json').read_text())['fetches'][2:11]
        cls.controlled_rows = json.loads((cls.controlled/'proof.json').read_text())['fetches']
        cls.pic = build_pic_probe(DIRECTORY)

    def check_phase(self, start, controlled=False):
        root = self.controlled if controlled else self.source
        rows = self.controlled_rows if controlled else self.default_rows
        if start == clock(rows[0])-1:
            timing = None
        else:
            script = DIRECTORY/'instructions.txt'
            script.write_text('n 0 0\n' * len(rows))
            ticks = [list(map(int,line.split())) for line in subprocess.check_output(
                [self.pic,'device-io',str(start),str(script)],text=True).splitlines()]
            self.assertEqual(len(ticks),len(rows))
            timing = [(t[0],t[2],30000-t[0]%30000-t[2]) for t in ticks]
        check(self,rows,root/'source/1280.memory',root/'source/77ee.memory',timing)

    def test_reached_configuration_matches_original_registers_memory_and_fetches(self):
        self.check_phase(clock(self.default_rows[0])-1)

    def test_port_word_store_preserves_adjacent_bytes_across_thin_budgets(self):
        for index in (29992,29998,29999):
            self.check_phase(525*30000+index,controlled=True)

    def test_original_controlled_words_preserve_ax_width_and_zero_extend_irq_dma(self):
        for start in (clock(self.controlled_rows[0])-1,
                      *(525*30000+index for index in (29992,29998,29999))):
            self.check_phase(start,controlled=True)
