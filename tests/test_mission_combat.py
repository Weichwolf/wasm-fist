#!/usr/bin/env python3
"""Canonical combat publication and delivered visits at explicit class boundaries."""
import argparse
import hashlib
import json
import pathlib
import struct
import subprocess
import tempfile
import unittest

from orders_contract import orders_lines
from test_aircraft_death import expected as aircraft_expected, fixture as aircraft_fixture, new_effect
from test_collision import expected as collision_expected, from_raw, delta
from test_destruction import advance_parent, advance_smoke
from test_ground import contact
from test_mission_world import install, payload_lines, SEEDS
from test_object_pool import NONE
from test_other_damage import expected as other_expected, fixture as other_fixture
from test_primary_fire import expected as fire_expected, fire_fixture
from test_projectile_flight import line, advance_explosion
from test_units import scenario_data, records_from_scenario
from test_vehicle_damage import expected as ground_expected, fixture as ground_fixture, vehicle_lines
from test_vehicle_start import random_line, step
from test_weapon_control import update as weapon_update

ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = pathlib.Path('/tmp/wasm-fist-rewrite')
TARGET = 'all'
NATIVE_PROBE = None
ORIGINALS = False
ORACLE = None


def words(raw, offset, *values):
    struct.pack_into('<'+'H'*len(values), raw, offset, *values)


def effect_from_line(text):
    fields = list(map(int, text.split()[1:]))
    allocation = 4, *fields[:3]
    template = fields[6], fields[7], fields[9], fields[12], fields[13]
    _, raw = new_effect(allocation, tuple(fields[3:6]), template)
    words(raw, 2, allocation[1]); words(raw, 28, fields[10])
    raw[25], raw[32], raw[22] = fields[11], fields[14], fields[15]
    return allocation, raw


def records(kind=2, *, mode=0, damage=99, heading=4096, hull=40960, source_index=179,
            target_index=180, altitude=66560, target_flags=96, y=2556, source_altitude=65536):
    result=[]
    for type_id,index,py,pz,bearing,member in ((0,source_index,0,source_altitude,0,0),
                                             (kind,target_index,y,altitude,heading,1)):
        raw=bytearray(251 if type_id<4 else 55)
        words(raw,0,type_id);struct.pack_into('<3iH',raw,4,0,py,pz,bearing)
        raw[22]=96 if type_id==0 else target_flags
        raw[28]=member
        if type_id<4:
            words(raw,38,hull if member else 0);raw[58]=damage if member else 0
            raw[146]=1 if not member else 0
        elif type_id in (5,6): raw[37],raw[51]=0,damage
        elif type_id==26: raw[25],raw[26],raw[27]=mode,damage,90
        elif type_id==27: raw[26]=damage
        result.append((index,1,bytes(raw)))
    return result


def encode(commands, *, seeds=SEEDS, cursor=0, link=0, scales=(256,256), selected=NONE,
           tick=300, animation=0, wind=(0,0), enabled=0, trees=(0,0), coarse=0,
           heights=bytes(4)):
    return (struct.pack('<4HBB2HHHH2i4BI',*seeds,cursor,link,*scales,selected,tick,animation,
                        *wind,enabled,*trees,coarse,len(commands))+bytes(heights)+
            b''.join(struct.pack('<B3H',*command) for command in commands))


class World:
    def __init__(self, saved, **options):
        status,state=install(saved,options.get('seeds',SEEDS),options.get('cursor',0),options.get('link',0))
        if status: raise ValueError(f'Unsupported fixture installation {status}')
        self.pool,self.objects,self.roster,self.words,self.cursor=state
        self.original_kind={slot:allocation[0] for slot,(allocation,_) in self.objects.items()}
        self.scales=options.get('scales',(256,256));self.selected=options.get('selected',NONE)
        self.counters=[0]*5;self.sizes=[0]*4;self.flash=0;self.pending=NONE;self.failed=0
        self.heights=options.get('heights',bytes(4));self.wind=options.get('wind',(0,0))
        self.enabled=options.get('enabled',0);self.trees=options.get('trees',(0,0))
        self.coarse=options.get('coarse',0);self.tick=options.get('tick',300)
        self.animation=options.get('animation',0);self.phases={}

    def publish(self, allocation, raw):
        raw=bytearray(raw);words(raw,2,allocation[1] if allocation[1]<150 else allocation[1]-150)
        self.objects[allocation[1]]=(allocation,raw)
        self.original_kind[allocation[1]]=allocation[0]
        self.phases[allocation[1]]=0

    def state(self):
        text=line('combat',[*self.scales,self.selected,self.flash,*self.counters,self.pending])
        text+=line('sizes',self.sizes)+self.pool.state()+random_line(self.words,self.cursor)+line('roster',self.roster)+orders_lines()
        for slot,(allocation,raw) in sorted(self.objects.items()):
            if not self.pool.slots[slot][0]: continue
            kind=self.pool.slots[slot][1]
            text+=line('object',[slot,kind])
            if kind==19:
                lines=vehicle_lines(raw,self.original_kind[slot],allocation[2],allocation[3]).splitlines()
                text+='\n'.join(lines[:-1])+'\n'+line('damage',[raw[58],raw[149]])
            elif kind==4:
                from test_aircraft_death import effect_lines
                text+=effect_lines((allocation,raw))
            elif kind==8:
                origin=(struct.unpack_from('<H',raw,39)[0]-0xc05c)//251+150
                text+=line('shell',[*allocation,*struct.unpack_from('<3iH',raw,4),
                                    *struct.unpack_from('<3h',raw,29),struct.unpack_from('<h',raw,27)[0],
                                    struct.unpack_from('<H',raw,35)[0],struct.unpack_from('<H',raw,45)[0],
                                    origin,NONE,0,*raw[22:26],self.phases[slot],raw[41],raw[42]])
            elif kind==18:
                text+=line('muzzle',[*allocation,*struct.unpack_from('<3iH',raw,4),
                                     struct.unpack_from('<H',raw,20)[0],
                                     struct.unpack_from('<H',raw,26)[0],raw[25],raw[22]])
            else: text+=payload_lines(raw,allocation)
        return text

    def fire(self, slot, tick):
        allocation,raw=self.objects[slot]
        if self.pending!=NONE: return 'fire -1 0 0 0 0 '+str(self.failed)+' 0\n'
        if raw[145]!=0: return 'fire -1 0 0 0 0 '+str(self.failed)+' 0\n'
        case=fire_fixture(snapshot=raw,coarse=self.coarse)
        _,observations=fire_expected(case,tick,self.failed,state=(bytearray(raw),allocation[2],allocation[3],slot),pool=self.pool)
        raw,_,created,requested,dispatched,panel,voice,self.failed=observations[0]
        self.objects[slot]=(allocation,bytearray(raw))
        for physical,index,value,record in created:
            self.publish((int.from_bytes(record[:2],'little'),physical,index,value),record)
        outcome=0 if created else 2 if dispatched and struct.unpack_from('<H',raw,173)[0] else 1
        # The complete launch result is observed independently of remaining ammunition.
        if dispatched and not created:
            previous=struct.unpack_from('<H',case[0],173)[0]
            outcome=2 if previous else 1
        return line('fire',[0,int(requested),int(dispatched),int(panel),voice,self.failed,outcome])

    def damage(self, source, hit):
        slot,index,value,aspect=hit
        target,raw=self.objects[slot];projectile=self.objects[source][1]
        kind=target[0];captured=[]
        if kind<4:
            case=ground_fixture(kind,raw=raw,scales=self.scales,source_flags=projectile[22],
                                aspect=aspect,finish=0,retire=0)
            case['raw']=bytes(raw)
            prepared=(self.pool,raw,target,self.objects[source][0],self.roster,self.sizes,
                      self.counters[:3],self.selected,self.flash,self.words,self.cursor)
            ground_expected(case,prepared=prepared,capture=captured.append)
            data=captured[0];self.roster=data['roster'];self.sizes=data['sizes'];self.flash=data['flash']
            self.counters[:3]=data['counters'];self.cursor=data['cursor']
            for effect in data['effects']: self.publish(*effect_from_line(effect))
            if data['wreck']:
                f=list(map(int,data['wreck'].split()[1:]));wreck=bytearray(55)
                words(wreck,0,23,f[0]);struct.pack_into('<3iH',wreck,4,*f[3:7])
                words(wreck,20,f[9]);words(wreck,27,*f[7:9]);words(wreck,33,f[10])
                wreck[35],wreck[36],wreck[22],wreck[23]=f[11:15]
                self.publish((23,*f[:3]),wreck)
            fatal=data['event'][3]
            return kind,line('ground_event',data['event'])+'voices'+''.join(f' {v}' for v in data['voices'])+'\n',bool(fatal)
        if kind==23:
            return kind,'other_event 0 0 0 1 255 255 0\n',False
        case=other_fixture(kind,raw=raw,scales=self.scales,source_flags=projectile[22],aspect=aspect,finish=0,effects=0)
        case['raw']=bytes(raw)
        prepared=(self.pool,raw,target,self.objects[source][0],self.counters,self.words,
                  self.cursor,self.selected,self.roster)
        other_expected(case,prepared=prepared,capture=captured.append)
        data=captured[0];self.counters=data['counters'];self.cursor=data['cursor']
        for effect in data['effects']:
            self.publish(*new_effect(effect['allocation'],effect['pose'],effect['template']))
        return kind,line('other_event',data['event']),False

    def impact(self, slot, phase):
        allocation,raw=self.objects[slot]
        effect=self.pool.explosion()
        if effect:
            template=(20,256,4,10,5) if phase==2 else (16,768,0,22,6)
            self.publish(*new_effect(effect,struct.unpack_from('<3i',raw,4),template))
        self.pool.release(allocation);raw[22]|=1;self.phases[slot]=3
        return [int(effect is not None),phase,15,int(phase==2)]

    def shell(self, slot):
        allocation,raw=self.objects[slot];phase=0;hit=[NONE,NONE,0,0]
        age=(struct.unpack_from('<H',raw,45)[0]+1)%65536;words(raw,45,age)
        if age>=480:
            self.pool.release(allocation);raw[22]|=1;phase=3
        else:
            position=struct.unpack_from('<3i',raw,4);velocity=struct.unpack_from('<3h',raw,29)
            position=tuple(delta(a+b,0) for a,b in zip(position,velocity));struct.pack_into('<3i',raw,4,*position)
            if (position[2]>>8)%65536<128:
                raw[24]=contact(2,self.heights,(*position[:2],struct.unpack_from('<H',raw,16)[0]))[0]
                if ((position[2]>>8)-raw[24])%256>=128: phase=1
            if not phase:
                grace=struct.unpack_from('<H',raw,35)[0]
                if grace: words(raw,35,grace-1)
                else:
                    active=[physical for physical in sorted(self.objects) if self.pool.slots[physical][0]]
                    bodies=[from_raw(self.objects[p][0][2],self.objects[p][0][3],self.objects[p][1]) for p in active]
                    mapping={index:active.index(physical) for index,(physical,_) in enumerate(self.pool.registry) if physical!=NONE}
                    result=collision_expected((bodies,[active.index(slot)],self.words,self.cursor),physical=active,bindings=mapping).splitlines()
                    candidate=list(map(int,result[0].split()[1:]));rng=list(map(int,result[1].split()[1:]));self.cursor,self.words=rng[0],rng[1:]
                    origin=(struct.unpack_from('<H',raw,39)[0]-0xc05c)//251+150
                    if candidate[0] not in (NONE,origin): phase=2;hit=candidate
        self.phases[slot]=phase
        damage_text='';kind=28;damaged=0;loss=False;impact=None
        if phase==2:
            kind,damage_text,loss=self.damage(slot,hit);damaged=1
            if loss: self.pending=slot
        if phase in (1,2) and not loss: impact=self.impact(slot,phase)
        return line('event',[phase,*hit,kind,damaged,int(impact is not None),int(loss)])+damage_text+'births 0 0 0 0 0\n'+(line('impact',impact) if impact else '')

    def spawn(self, pose, strength):
        allocation=self.pool.allocate_short(17,low_priority=True) if self.enabled==1 else None
        if allocation is None: return None
        roll,self.cursor=step(self.words,self.cursor);extent=(strength+(roll&63))%65536
        raw=bytearray(55);words(raw,0,17,allocation[1])
        struct.pack_into('<3i3H',raw,4,*pose[:2],delta(pose[2]+768,0),0,extent,extent*4%65536)
        self.publish(allocation,raw);return allocation

    def visit(self, index, tick):
        actual=next((n for n in range(index,182) if self.pool.registry[n][0]!=NONE),None)
        if actual is None: return '',182
        slot,value=self.pool.registry[actual];kind=self.pool.slots[slot][1]
        allocation,raw=self.objects[slot]
        status=-1 if self.pending!=NONE else 2 if kind<4 or kind in (5,6) and raw[37]!=12 or kind==26 and raw[25]<4 else 0
        header=line('visit',[actual,slot,kind,status]);event='event 0 0 0 0 0 28 0 0 0\n';births=[0,0,0,0,0]
        if status: return header+self.state(),actual+1
        if kind==8: return header+self.shell(slot)+self.state(),actual+1
        if kind==4:
            frame,countdown,height,released=advance_explosion(raw[25],raw[30],raw[31],raw[32],struct.unpack_from('<H',raw,26)[0],struct.unpack_from('<H',raw,28)[0])
            raw[25],raw[32]=frame,countdown;words(raw,28,height)
            if released: self.pool.release(allocation);raw[22]|=1
        elif kind==18:
            counter=(struct.unpack_from('<H',raw,26)[0]+1)%65536
            if counter>=8:
                counter=0;raw[25]=(raw[25]+1)%256
                if raw[25]>=7: self.pool.release(allocation);raw[22]|=1
            words(raw,26,counter)
        elif kind==19:
            raw[23]=(raw[23]-1)%256
            if not raw[23]: self.pool.release(allocation)
        elif kind==17:
            if advance_smoke(raw,self.wind,self.enabled): self.pool.release(allocation)
        elif kind==21:
            if self.trees[0]==1: raw[25]=self.trees[1]
        elif kind in (23,26,27):
            pose=struct.unpack_from('<3i',raw,4)
            births[0]=int(advance_parent(raw,lambda strength:self.spawn(pose,strength)) is not None)
        elif kind in (5,6):
            case=aircraft_fixture(kind,raw=raw,tick=tick,animation=self.animation,side=2,pixels=self.heights,enabled=self.enabled,coarse=self.coarse)
            capture=[]
            aircraft_expected(case,prepared=(self.pool,raw,allocation,self.words,self.cursor),capture=capture.append,class_only=True)
            data=capture[0];self.cursor=data['cursor']
            if data['effect']: self.publish(*data['effect'])
            if data['smoke']: self.publish(*data['smoke'])
            births=[0,int(data['smoke'] is not None),int(data['effect'] is not None),int(data['released']),data['sound']]
        else: raise AssertionError('No invented class fallback')
        return header+event+line('births',births)+self.state(),actual+1

    def run(self, commands):
        yield self.state()
        for operation,a,b,repeat in commands:
            if operation==6:
                for turn in range(repeat):
                    index=a
                    while index<b:
                        actual=next((n for n in range(index,182) if self.pool.registry[n][0]!=NONE),182)
                        if actual>=b: break
                        text,index=self.visit(index,(self.tick+turn)%65536)
                        yield text
                        if self.pending!=NONE: break
                    if self.pending!=NONE: break
                continue
            if operation==1:
                self.tick=b;text,_=self.visit(a,b);yield text;continue
            if operation==2:
                if self.pending==NONE: yield 'resume -1 0 0 0 0\n'
                else:
                    values=self.impact(self.pending,2);self.pending=NONE;yield line('resume',[0,*values])
            elif operation==0: yield self.fire(a,b)
            else:
                allocation,raw=self.objects[a]
                if operation==3:
                    raw,_=weapon_update(raw,0,b);raw=bytearray(raw)
                    flags=struct.unpack_from('<H',raw,64)[0]
                    if flags&1: words(raw,64,flags&65534);raw[199]=3
                elif operation==4:
                    for _ in range(repeat):
                        raw[61]=(raw[61]+2)%256;raw,_=weapon_update(raw,4,0);raw=bytearray(raw)
                elif operation==5: raw[146]=48
                self.objects[a]=(allocation,raw);yield line('command',[operation,0])
            yield self.state()


class MissionCombatTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp=tempfile.TemporaryDirectory(prefix='wasm-fist-mission-combat-',dir='/tmp');cls.addClassCleanup(cls.temp.cleanup)
        cls.directory=pathlib.Path(cls.temp.name);cls.commands=[]
        if TARGET in ('all','native'): cls.commands.append([str(NATIVE_PROBE or BUILD/'native/fist_mission_combat_probe')])
        if TARGET in ('all','wasm'): cls.commands.append(['node',str(BUILD/'wasm/fist_mission_combat_probe.js')])
        cls.fixtures=cls.visits=0
        cls.original_calls=cls.original_objects=0

    @classmethod
    def tearDownClass(cls):
        print(f'Canonical combat: {cls.fixtures} fixtures, {cls.visits} declared class visits per target; complete unfiltered state streams and lengths compared',flush=True)
        if ORACLE:
            print(f'Original consuming boundary: {ORACLE.class_calls} complete class calls, {ORACLE.continuations} impact acknowledgements, {ORACLE.living_boundaries} explicit unexecuted living boundaries, {cls.original_objects} complete physical payload observations; all metadata, RNG and combat state checked',flush=True)

    def test_malformed_inputs_fail_without_output(self):
        request=self.directory/'bad-request.bin';scenario=self.directory/'bad-input.fsg'
        valid=encode([]);saved=scenario_data(records())
        bad=[(valid[:n],saved) for n in (0,1,8,32,39)]
        bad.append((valid+b'\0',saved))
        for offset,value in ((8,4),(31,2),(32,1)):
            raw=bytearray(valid);raw[offset]=value;bad.append((raw,saved))
        bad.append((encode([(7,0,0,0)]),saved))
        bad.extend((valid,saved[:n]) for n in (0,1,16,40,len(saved)-1))
        for raw,data in bad:
            request.write_bytes(raw);scenario.write_bytes(data)
            for command in self.commands:
                result=subprocess.run([*command,str(request),str(scenario)],capture_output=True,timeout=30)
                self.assertEqual(result.returncode,1,'Malformed request/scenario must report ordinary failure, not crash')
                self.assertEqual(result.stdout,b'','Rejected input must not produce a partial success trace')
                self.assertEqual(result.stderr,b'','Rejected input must not produce a runtime or sanitizer error')

    def test_required_original_consuming_boundaries(self):
        if ORACLE is None: self.skipTest('Original instruction execution is an explicitly required additional gate')
        fixtures=[]
        for kind in range(4):
            for aspect in range(16):
                for damage in (0,99):
                    fixtures.append((records(kind,damage=damage,heading=(-aspect*4096)%65536),3,{}))
        for kind,mode in [(5,0),(6,0),(23,0),(26,0),(26,1),(26,2),(26,3),(27,0)]:
            for seed in (2,4):
                saved=records(kind,mode=mode,target_flags=64,altitude=67584 if kind in (5,6) else 66560,heading=0)
                if kind in (5,6):
                    i,v,raw=saved[1];raw=bytearray(raw);raw[24]=1;words(raw,35,1);saved[1]=i,v,bytes(raw)
                fixtures.append((saved,3,dict(seeds=(seed,2,2,2),enabled=1)))
        fixtures.append((records(),140,dict(enabled=1,wind=(17,-23))))
        fixtures.append((records(),3,dict(selected=151)))
        fixtures.append((records(y=1000000),485,{}))
        for height in (0,1,127,128,255):
            for altitude in (0,1,127,128,65535):
                fixtures.append((records(source_altitude=altitude*256-2048,y=1000000),3,dict(heights=bytes([height])*4)))
        from test_mission_world import record
        for count in (118,119,120,148,149):
            fixtures.append((records()+[record(21,n) for n in range(count)],5,dict(trees=(1,251))))
        saved=records();index,value,raw=saved[0];raw=bytearray(raw)
        raw[28]=2;struct.pack_into('<i',raw,8,1000000)
        fixtures.append((saved+[(index,value,bytes(raw))],5,{}))
        from test_destruction import fixture as wreck_fixture
        for index in (0,181):
            fixtures.append((records()+[(index,1,wreck_fixture(counter=63)['raw'])],365,
                             dict(enabled=1,wind=(17,-23))))
        for saved,turns,options in fixtures:
            limit=179 if any(int.from_bytes(raw[:2],'little') in (5,6) for _,_,raw in saved) else 182
            commands=[(0,150,300,0),(6,0,limit,turns)]
            if options.get('selected')==151: commands.append((2,0,0,0))
            self.check(saved,commands,**options)
            world=World(saved,**options);world.fire(150,300)
            machine=ORACLE.prepare_world(world)
            type(self).original_objects+=ORACLE.assert_world(machine,world)
            for turn in range(turns):
                index=0
                while index<limit:
                    current=next((n for n in range(index,limit) if world.pool.registry[n][0]!=NONE),None)
                    if current is None: break
                    observed,index=ORACLE.visit_world(machine,world,current,world.tick+turn)
                    type(self).original_objects+=observed;type(self).original_calls+=1
                    if world.pending!=NONE: break
                if world.pending!=NONE: break
            if world.pending!=NONE:
                type(self).original_objects+=ORACLE.resume_world(machine,world);type(self).original_calls+=1

    def check(self,saved,commands,**options):
        world=World(saved,**options);wanted_hash=hashlib.sha256();wanted_size=0;monitor=[]
        for text in world.run(commands):
            data=text.encode();wanted_hash.update(data);wanted_size+=len(data)
            monitor.extend(row+'\n' for row in text.splitlines() if row.startswith(('event ','visit ','births ','fire ','resume ','command ','smoke ')))
        request=self.directory/'request.bin';scenario=self.directory/'input.fsg'
        request.write_bytes(encode(commands,**options));scenario.write_bytes(scenario_data(saved))
        for command in self.commands:
            with tempfile.TemporaryFile(dir='/tmp') as output:
                result=subprocess.run([*command,str(request),str(scenario)],stdout=output,stderr=subprocess.PIPE,timeout=180)
                self.assertEqual(result.returncode,0,result.stderr.decode())
                output.seek(0);actual_hash=hashlib.sha256();actual_size=0
                while data:=output.read(1024*1024):actual_hash.update(data);actual_size+=len(data)
                if actual_size!=wanted_size or actual_hash.digest()!=wanted_hash.digest():
                    output.seek(0);row=0
                    for text in World(saved,**options).run(commands):
                        for expected in text.splitlines(keepends=True):
                            actual=output.readline().decode();row+=1
                            if actual!=expected: self.fail(f'First complete output difference at line {row}: actual={actual.rstrip()!r}, expected={expected.rstrip()!r}; bytes {actual_size}/{wanted_size}')
                    self.fail(f'Extra output after complete golden trace; bytes {actual_size}/{wanted_size}')
        type(self).fixtures+=1;type(self).visits+=sum(row.startswith('visit ') for row in monitor)
        return world,''.join(monitor)

    def test_reaching_distinct_hull_turret_damage_publication_and_lifetimes(self):
        commands=[(0,150,300,0),(6,0,182,140)]
        world,trace=self.check(records(),commands)
        self.assertTrue('event 2 151 180 1 15 2 1 1 0\n' in trace,'Missing original aspect 15 at complete damage boundary')
        self.assertEqual(world.counters[1],1)
        self.assertEqual(world.pool.slots[151],(0,0))

    def test_all_ground_aspects_reactions_roster_and_selected_loss(self):
        for kind in range(4):
            for aspect in range(16):
                for damage in (0,99):
                    saved=records(kind,damage=damage,heading=(-aspect*4096)%65536,hull=(aspect*123)%65536)
                    world,trace=self.check(saved,[(0,150,300,0),(6,0,182,5)])
                    self.assertTrue(f'event 2 151 180 1 {aspect} {kind} 1 1 0\n' in trace,f'Missing complete ground hit: type {kind}, aspect {aspect}, initial damage {damage}')
        world,trace=self.check(records(),[(0,150,300,0),(6,0,182,5),
                                          (1,1,310,0),(0,150,310,0),(2,0,0,0),
                                          (6,0,182,5),(2,0,0,0)],selected=151)
        self.assertTrue('event 2 151 180 1 15 2 1 0 1\n' in trace,'Selected loss must suspend before impact')
        self.assertTrue('visit 1 1 18 -1\n' in trace,'Selected loss must block subsequent visits')
        self.assertTrue('resume 0 1 2 15 1\n' in trace,'Explicit player-loss acknowledgement must finish impact once')
        self.assertEqual(world.pending,NONE)
        self.assertEqual(world.counters[1],1)

    def test_other_damage_class_transitions_and_complete_lifetimes(self):
        for kind,mode in [(5,0),(6,0),(23,0),(26,0),(26,1),(26,2),(26,3),(27,0)]:
            for seed in (2,4):
                saved=records(kind,mode=mode,target_flags=64,
                              altitude=67584 if kind in (5,6) else 66560,heading=0)
                if kind in (5,6):
                    index,value,raw=saved[1];raw=bytearray(raw);raw[24]=1;words(raw,35,1);saved[1]=index,value,bytes(raw)
                world,trace=self.check(saved,[(0,150,300,0),(6,0,182,140)],seeds=(seed,2,2,2),enabled=1,wind=(17,-23))
                self.assertTrue(f' {kind} 1 1 0\n' in trace,f'Missing complete reached other damage for {kind}/{mode}/{seed}')

    def test_ground_gate_expiry_muzzle_and_allocation_pressure(self):
        for height in (0,1,127,128,255):
            for altitude in (0,1,127,128,65535):
                self.check(records(source_altitude=altitude*256-2048,y=1000000),
                           [(0,150,300,0),(6,0,2,3)],heights=bytes([height])*4)
        world,trace=self.check(records(y=1000000),[(0,150,300,0),(6,0,182,485)])
        self.assertTrue('event 3 65535 65535 0 0 28 0 0 0\n' in trace,'Shell must expire at the complete age boundary')
        self.assertTrue(all(not used for used,_ in world.pool.slots[:150]))
        from test_mission_world import record
        for count in (118,119,120,148,149):
            saved=records()+[record(21,index,flags=0) for index in range(count)]
            world,trace=self.check(saved,[(0,150,300,0),(6,count,179,5)])
            self.assertEqual(world.counters[1],1)

    def test_mutable_current_entry_births_orphans_and_reused_payloads(self):
        from test_destruction import fixture
        for index in (0,181):
            saved=[(index,1,fixture(counter=63)['raw'])]
            world,trace=self.check(saved,[(6,0,182,365)],enabled=1,wind=(17,-23))
            self.assertTrue('births 1 0 0 0 0\n' in trace,'Wreck must publish its smoke before the next mutable registry visit')
            first=trace.index('births 1 0 0 0 0\n')
            first_smoke=next(row for row in trace[first:].splitlines() if row.startswith('smoke '))
            self.assertEqual(int(first_smoke.split()[-1]),0)
        saved=records()
        index,value,raw=saved[0];raw=bytearray(raw);raw[28]=2;struct.pack_into('<i',raw,8,1000000)
        saved.append((index,value,bytes(raw)))
        world,trace=self.check(saved,[(0,150,300,0),(6,0,182,5)])
        self.assertEqual(world.pool.registry[179][0],152)
        self.assertEqual(struct.unpack_from('<H',world.objects[150][1],173)[0],14)
        self.assertEqual(world.counters[1],1)
        from test_aircraft_death import fixture as death_fixture
        for kind in (5,6):
            saved=[(0,1,death_fixture(kind,altitude=0,countdown=1,ground=0)['raw'])]
            world,trace=self.check(saved,[(6,0,182,365)],seeds=(2,2,2,2),tick=0,enabled=1)
            self.assertTrue('births 0 1 1 1 9\n' in trace,'Released aircraft must publish actual same-slot smoke and effect')

    def test_required_real_train1_owned_fire_flight_and_explicit_living_boundaries(self):
        if not ORIGINALS: self.skipTest('Real TRAIN1 is an explicitly required additional corpus gate')
        manifest=json.loads((ROOT/'tests/scenario_originals.json').read_text())
        data=(ROOT/'armoredfist/FISTDATA/TRAIN1.FSG').read_bytes()
        self.assertEqual(hashlib.sha256(data).hexdigest(),manifest['TRAIN1.FSG']['sha256'])
        saved=records_from_scenario(data);self.assertEqual(len(saved),85)
        initial=World(saved);player=initial.roster[0];self.assertEqual(initial.objects[player][0][0],0)
        world,trace=self.check(saved,[(3,player,0,0),(4,player,0,320),(5,player,0,0),
                                     (0,player,320,0),(6,0,182,485)])
        self.assertTrue('fire 0 1 1 1 12 0 0\n' in trace,'Real TRAIN1 selected M1 must consume the actual fire request')
        self.assertTrue(f'visit {initial.objects[player][0][2]} {player} 0 2\n' in trace,'Original physical player slot must retain its actual registry entry')
        self.assertEqual(world.pending,NONE)
        print('TRAIN1: all 85 owned objects, declared selection/reload/fire and combat visits; explicit living-method boundaries, constructed 2x2 height input; no complete mission-tick claim',flush=True)


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--target',choices=('all','native','wasm'),default=TARGET)
    parser.add_argument('--build-root',type=pathlib.Path,default=BUILD)
    parser.add_argument('--native-probe',type=pathlib.Path)
    parser.add_argument('--originals',action='store_true')
    parser.add_argument('--oracle',action='store_true')
    args=parser.parse_args();TARGET,BUILD,NATIVE_PROBE,ORIGINALS=args.target,args.build_root,args.native_probe,args.originals
    if args.oracle:
        from original_mission_combat_oracle import OriginalMissionCombatOracle
        ORACLE=OriginalMissionCombatOracle()
    suite=unittest.defaultTestLoader.loadTestsFromTestCase(MissionCombatTests)
    result=unittest.TextTestRunner().run(suite)
    expected_skips=int(not ORIGINALS)+int(ORACLE is None)
    if len(result.skipped)!=expected_skips:
        raise SystemExit(f'Expected {expected_skips} explicit corpus skips, got {len(result.skipped)}')
    raise SystemExit(0 if result.wasSuccessful() else 1)
