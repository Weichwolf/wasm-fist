"""Untouched original persistent-parent and type-17 smoke methods.

Calls begin at explicit snapshot or already-reached damage boundaries. Scripted
parent→smoke→impact-effect order is an integration fixture, not the world scheduler.
"""
import struct

from original_other_damage_oracle import OriginalOtherDamageOracle
from original_unit_oracle import DGROUP
from test_destruction import parent_lines, smoke_lines
from test_projectile_flight import line


class OriginalDestructionOracle(OriginalOtherDamageOracle):
    def prepare(self, case):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_BX, UC_X86_REG_CX, UC_X86_REG_DI, UC_X86_REG_EFLAGS
        machine=self.machine(case['seeds'],case['cursor']);self.far_call(machine,0x1b176)
        pointers=[]
        for kind,index,value in case['bindings']:
            machine.reg_write(UC_X86_REG_AX,kind);machine.reg_write(UC_X86_REG_BX,index);machine.reg_write(UC_X86_REG_CX,value)
            self.far_call(machine,0x1b1a2)
            if machine.reg_read(UC_X86_REG_EFLAGS)&1: raise AssertionError('Original import failed')
            pointers.append(machine.reg_read(UC_X86_REG_DI))
        primary=pointers[case['target']];raw=bytearray(case['raw']);raw[2:4]=machine.mem_read(DGROUP+primary+2,2)
        machine.mem_write(DGROUP+primary,bytes(raw));machine.mem_write(DGROUP+0x8b4f,bytes([case['enabled']]))
        machine.mem_write(DGROUP+0x92f2,struct.pack('<2i',*case['wind']))
        return machine,pointers

    def shared(self,machine):
        words,cursor=self.random_state(machine)
        return line('random',[cursor,*words])+self.state(machine)

    def allocations(self,machine,before):
        return [(address,index,value) for index,(address,value) in enumerate(self.bindings(machine))
                if address and (address,value)!=before[index]]

    def smoke(self,machine,entry):
        address,index,value=entry
        return smoke_lines(bytes(machine.mem_read(DGROUP+address,55)),(17,self.slot(address),index,value))

    def initial_smoke(self,machine,entry,source,strength):
        address,index,value=entry;raw=bytes(machine.mem_read(DGROUP+address,55))
        extent,scale=struct.unpack_from('<2H',raw,18)
        expected=bytearray(55);struct.pack_into('<HH',expected,0,17,self.slot(address))
        x,y,z=struct.unpack_from('<3i',source,4)
        z=(z+768+2**31)%2**32-2**31
        struct.pack_into('<3iH2H',expected,4,x,y,z,0,extent,scale)
        if raw!=expected or scale!=extent*4%65536 or (extent-strength)%65536>63:
            raise AssertionError('Original complete smoke constructor/initializer changed')
        return self.smoke(machine,entry)

    def trace(self,case):
        from unicorn.x86_const import UC_X86_REG_AX, UC_X86_REG_DI
        machine,pointers=self.prepare(case);primary=pointers[case['target']]
        kind,index,value=case['bindings'][case['target']]
        allocation=(kind,self.slot(primary),index,value)
        smokes={};effects=[];output=''
        raw=bytes(machine.mem_read(DGROUP+primary,55))
        excluded={primary}
        if case['operation']==3:
            excluded.add(pointers[0])
        unrelated=[(pointer,bytes(machine.mem_read(DGROUP+pointer,251 if self.type_flags[typ]&1 else 55)))
                   for pointer,(typ,_,_) in zip(pointers,case['bindings']) if pointer not in excluded]
        if case['operation']==3:
            source=pointers[0];machine.mem_write(DGROUP+source+4,raw[4:18]);machine.mem_write(DGROUP+source+0x2a,b'\x05')
            machine.mem_write(DGROUP+0x9a25,struct.pack('<H',primary));machine.mem_write(DGROUP+0x9bd7,b'\0\0')
            machine.mem_write(DGROUP+0xe3ae,struct.pack('<2H',256,256));machine.mem_write(DGROUP+0x6d34,b'\0\0')
            machine.mem_write(DGROUP+0x6d3c,bytes(64));machine.mem_write(DGROUP+0x6ce6,b'\x02')
            machine.mem_write(DGROUP+0x9fea,b'\xff');machine.mem_write(DGROUP+0x9fdf,b'\0\0')
            before=self.bindings(machine);source_saved=bytes(machine.mem_read(DGROUP+source,55))
            voices,sound,destruction,selected=self.dispatch(machine,source,voice_entries=(0xbefb,))
            if voices or sound!=255 or selected or bytes(machine.mem_read(DGROUP+source,55))!=source_saved:
                raise AssertionError('Unexpected critical-hit source/UI mutation')
            effects=self.allocations(machine,before);changed=bytes(machine.mem_read(DGROUP+primary,55))
            destroyed=bool(changed[25]&4) if kind==26 else changed[25]==1
            output+=line('damage',[(changed[26]-raw[26])%256,int(destroyed),destruction,int(bool(effects))])
            machine.reg_write(UC_X86_REG_DI,source);machine.reg_write(UC_X86_REG_AX,0x9c4d)
            before=self.bindings(machine);self.call(machine,0xba33);impact=self.allocations(machine,before)
            output+=line('impact',[int(bool(impact))]);effects+=impact
            machine.reg_write(UC_X86_REG_DI,source);self.call(machine,0xb6be)
            output+=parent_lines(changed,allocation)+self.shared(machine)
            raw=changed
        if case['operation']==0:
            before=self.bindings(machine);machine.reg_write(UC_X86_REG_DI,primary);machine.reg_write(UC_X86_REG_AX,case['strength'])
            self.far_call(machine,0x19caa);created=self.allocations(machine,before)
            if len(created)>1:raise AssertionError('Multiple initial smoke allocations')
            if bytes(machine.mem_read(DGROUP+primary,55))!=raw:
                raise AssertionError('Original smoke creation modified its source')
            output+=line('creation',[int(not created)])
            for entry in created:
                output+=self.initial_smoke(machine,entry,raw,case['strength']);smokes[self.slot(entry[0])]=entry
        elif case['operation']==1:
            entry=(primary,index,value);smokes[self.slot(primary)]=entry;output+=self.smoke(machine,entry)
        else:output+=parent_lines(raw,allocation)
        output+=self.shared(machine)
        if any(bytes(machine.mem_read(DGROUP+pointer,len(saved)))!=saved for pointer,saved in unrelated):
            raise AssertionError('Unrelated original imported payload changed during admission/damage')
        parent_writes={23:{22,23,31,32},26:{22,28,29,30},27:{22,23,27,28,29,30}}.get(kind,set())
        for tick in range(case['ticks']):
            if case['operation']>=2 and tick<case['parent_ticks']:
                before=self.bindings(machine);old=bytes(machine.mem_read(DGROUP+primary,55))
                parameter=struct.unpack_from('<H',old,{23:33,26:28,27:29}[kind])[0]
                machine.reg_write(UC_X86_REG_DI,primary);self.call(machine,{23:0xbc0c,26:0xbc46,27:0xb355}[kind])
                current=bytes(machine.mem_read(DGROUP+primary,55))
                if any(current[i]!=old[i] for i in range(55) if i not in parent_writes):raise AssertionError('Unrelated original parent field changed')
                created=self.allocations(machine,before)
                if len(created)>1:raise AssertionError('Multiple parent smoke allocations')
                output+=line('emission',[int(bool(created))])+parent_lines(current,allocation)
                for entry in created:
                    output+=self.initial_smoke(machine,entry,current,parameter);smokes[self.slot(entry[0])]=entry
            for slot,entry in sorted(list(smokes.items())):
                pointer,entry_index,saved=entry;old=bytes(machine.mem_read(DGROUP+pointer,55))
                machine.reg_write(UC_X86_REG_DI,pointer);self.call(machine,0x9b11)
                current=bytes(machine.mem_read(DGROUP+pointer,55))
                if any(current[i]!=old[i] for i in range(55) if i not in {*range(4,14),22,25,26,27}):
                    raise AssertionError('Unrelated original smoke field changed')
                output+=self.smoke(machine,entry)
                if self.bindings(machine)[entry_index][0]==0:del smokes[slot]
            active=[]
            for entry in effects:
                pointer,_,_=entry
                machine.reg_write(UC_X86_REG_DI,pointer);self.call(machine,0xbab4)
                if not machine.mem_read(DGROUP+pointer+22,1)[0]&1:active.append(entry)
            effects=active
            if any(bytes(machine.mem_read(DGROUP+pointer,len(saved)))!=saved for pointer,saved in unrelated):
                raise AssertionError('Unrelated original imported payload changed')
            output+=self.shared(machine)
        return output
