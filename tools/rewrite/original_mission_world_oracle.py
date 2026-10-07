"""Actual d84a/43c1 loader continuation and complete c296/class returns.

Only the declared DOS snapshot read is supplied by the host. No original code,
constructors, initialization calls or method returns are replaced by hooks.
"""
import struct

from original_object_pool_oracle import OriginalObjectPoolOracle
from original_unit_oracle import DGROUP
from test_mission_world import install, payload_lines, tree_lines
from test_projectile_flight import line
from test_vehicle_start import random_line


class OriginalMissionWorldOracle(OriginalObjectPoolOracle):
    def world_lines(self, machine, objects):
        words,cursor=self.random_state(machine)
        roster=struct.unpack('<32H',machine.mem_read(DGROUP+0x6d3c,64))
        output=self.state(machine)+random_line(words,cursor)+line('roster',[self.slot(p) if p else 65535 for p in roster])
        for slot,(allocation,pointer,size) in sorted(objects.items()):
            raw=bytes(machine.mem_read(DGROUP+pointer,size))
            output+=line('object',[slot,allocation[0]])+payload_lines(raw,allocation)
        return output

    def prepare(self, records, seeds, cursor, link):
        from unicorn.x86_const import UC_X86_REG_AX,UC_X86_REG_BX,UC_X86_REG_CX,UC_X86_REG_DI,UC_X86_REG_EFLAGS
        status,golden=install(records,seeds,cursor,link)
        if status: raise AssertionError('Unsupported/invalid/exhausted inputs are explicit C-only gates')
        machine=self.machine(seeds,cursor,link)
        machine.mem_write(DGROUP+0x6db4,bytes(2))
        self.far_call(machine,0x1b176);self.call(machine,0x4413)
        objects={}
        for index,value,saved in records:
            kind=int.from_bytes(saved[:2],'little')
            machine.reg_write(UC_X86_REG_AX,kind);machine.reg_write(UC_X86_REG_BX,index);machine.reg_write(UC_X86_REG_CX,value)
            self.far_call(machine,0x1b1a2)
            if machine.reg_read(UC_X86_REG_EFLAGS)&1: raise AssertionError('Original valid allocation failed')
            pointer=machine.reg_read(UC_X86_REG_DI);slot=self.slot(pointer);size=len(saved)
            constructor=bytes(machine.mem_read(DGROUP+pointer,size))
            if constructor[:2]!=saved[:2] or constructor[4:]!=bytes(size-4):
                raise AssertionError('Original complete constructor changed')
            # The d82e..d847 DOS read preserves the newly allocated word +2.
            machine.mem_write(DGROUP+pointer,saved[:2]+constructor[2:4]+saved[4:])
            unrelated=[(p,bytes(machine.mem_read(DGROUP+p,n))) for _,p,n in objects.values()]
            self.call(machine,0xd84a)
            if machine.reg_read(UC_X86_REG_DI)!=pointer: raise AssertionError('Original loader changed DI')
            objects[slot]=((kind,slot,index,value),pointer,size)
            if any(bytes(machine.mem_read(DGROUP+p,len(data)))!=data for p,data in unrelated):
                raise AssertionError('Original initialization changed another payload')
            actual=bytes(machine.mem_read(DGROUP+pointer,size))
            if actual!=bytes(golden[1][slot][1]): raise AssertionError('Complete original saved/init payload differs')
        if self.random_state(machine)!=(golden[3],golden[4]): raise AssertionError('Original complete installation RNG differs')
        return machine, objects

    def install(self, records, seeds, cursor, link, commands, reload=False):
        from unicorn.x86_const import UC_X86_REG_DI
        machine, objects = self.prepare(records, seeds, cursor, link)
        output=line('status',[0])+self.world_lines(machine,objects)
        if reload:
            words,end=self.random_state(machine)
            next_output=self.install(records,words,end,link,commands)
            return output+line('reload',[0])+next_output.removeprefix('status 0\n')
        for slot,changed,variant in commands:
            status=-1
            if slot in objects and objects[slot][0][0]==21:
                allocation,pointer,size=objects[slot]
                binding=struct.unpack('<HH',machine.mem_read(DGROUP+0xdfbc+allocation[2]*4,4))
                if binding==(pointer,allocation[3]):
                    before=bytearray(machine.mem_read(DGROUP+pointer,size))
                    machine.mem_write(DGROUP+0x930d,bytes([changed]));machine.mem_write(DGROUP+0x930c,bytes([variant]))
                    machine.reg_write(UC_X86_REG_DI,pointer);self.call(machine,0x9c4f)
                    if changed==1:before[25]=variant
                    if bytes(machine.mem_read(DGROUP+pointer,size))!=before:
                        raise AssertionError('Original tree changed unrelated payload fields')
                    status=0
                output+=line('update',[slot,status])+tree_lines(bytes(machine.mem_read(DGROUP+pointer,size)),allocation)
            else:output+=line('update',[slot,status])
        return output+self.world_lines(machine,objects)
