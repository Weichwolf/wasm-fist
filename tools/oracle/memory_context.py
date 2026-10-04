"""Actual CPU/system/provider/cache/VGA inputs shared by reached source captures."""
from contextlib import contextmanager
import copy
import json
from pathlib import Path
import struct
import tempfile

from capture_pit_events import digest
from capture_pit_irq_frames import SYSTEM_FIELDS
from capture_physical_provider import portable as portable_provider, validate_provider

# These identifiers describe source classes, never synthesize a physical map.
HANDLERS = ('InitPageHandler *', 'RAMPageHandler *', 'ROMPageHandler *',
            'VGA_Map_Handler *', 'VGA_ChainedVGA_Handler *')


def observer(repo, kind):
    name = {'irq':'pit_irq_frame.gdb', 'device':'device_start_prefix.gdb',
            'config':'device_config_cpu.gdb'}[kind]
    text = (repo/'tools/oracle'/name).read_text()
    cache = (repo/'tools/oracle/paging_control.gdb.inc').read_text()
    extra = (repo/'tools/oracle/physical_provider.gdb.inc').read_text()
    extra += cache[cache.index('def cache_state('):cache.index('\ndef crx_record(')]
    extra += (repo/'tools/oracle/memory_context.gdb.inc').read_text()
    if kind == 'irq':
        needle = 'def save(kind,index,full=False,extra=None):'
        assert text.count(needle) == 1
        text = text.replace(needle, extra+'\n'+needle)
        replacements = {
            "name='%s-%d-%d.memory'%(kind,index,serial);q['memory_file']=name;(root/name).write_bytes(memory())":
            "name='%s-%d-%d.memory'%(kind,index,serial);q['memory_file']=name;q['memory_context']=memory_context(name);(root/name).write_bytes(memory())",
            "(root/q['memory_file']).write_bytes(memory())":
            "q['memory_context']=memory_context(q['memory_file']);(root/q['memory_file']).write_bytes(memory())",
            "(root/q['memory_file']).write_bytes(m)":
            "q['memory_context']=memory_context(q['memory_file']);(root/q['memory_file']).write_bytes(m)"}
        for old,new in replacements.items():
            assert old in text, old
            text = text.replace(old,new)
    else:
        needle = "fields=fields+('cpu.stack.mask','cpu.stack.notmask')"
        assert text.count(needle) == 1
        text = text.replace(needle, needle+'+'+repr(SYSTEM_FIELDS)+'\n'+extra)
        needle = " with (root/"
        at = text.index(needle, text.index('def observe(q):'))
        condition = 'ip in boundaries' if kind == 'device' else 'ip in boundaries or ip==return_ip'
        text = text[:at]+" if "+condition+":q['memory_context']=memory_context('%04x.memory'%ip)\n"+text[at:]
    return text


def portable(context):
    """Keep every observed cache slot/list and provider state without host addresses."""
    c = copy.deepcopy(context)
    c['provider'] = portable_provider(c['provider'])
    cache = c['cache']; cache.pop('mem_base'); cache.pop('init_handler')
    for entry in cache['entries'].values():
        for key in ('read','write'):entry[key] = bool(entry[key])
        for key in ('readhandler','writehandler'):entry.pop(key)
    for key in ('linear','fastmem'):c['vga'][key].pop('pointer')
    return c


def validate_context(context, folder, repo):
    p,c,v = context['provider'],context['cache'],context['vga']
    validate_provider(repo,p)
    assert p['pages'] == len(p['slots']) == 4096, 'incomplete physical map'
    assert set(p['handlers']) == {str(slot) for slot in p['slots']}
    assert len(p['firstmb']) == 272 and all(0 <= page < 1048576 for page in p['firstmb'])
    assert type(p['a20_enabled']) is bool and 0 <= p['a20_controlport'] < 256
    assert len(c['linked_pages']) <= 32768
    assert set(c['entries']) == {str(page) for page in c['linked_pages']}, 'incomplete linked cache'
    for page,entry in c['entries'].items():
        assert 0 <= int(page) < 1048576 and 0 <= entry['phys_page'] < 1048576
        read,write = entry['handler_types']['readhandler'],entry['handler_types']['writehandler']
        assert read == write and read in HANDLERS, 'unsupported reached cache handler'
        if read == HANDLERS[0]:
            assert entry['readhandler'] == entry['writehandler'] == c['init_handler']
            assert not entry['read'] and not entry['write']
            continue
        physical = entry['phys_page']; assert physical < p['pages']
        h = p['handlers'][str(p['slots'][physical])]
        assert h['type'] == read and all(entry['handler_flags'][k] == h['flags'] for k in ('readhandler','writehandler'))
        assert entry['readhandler'] == entry['writehandler'] == p['slots'][physical]
        for key,flag,bank in (('read',1,'svga.bank_read_full'),('write',2,'svga.bank_write_full')):
            assert bool(entry[key]) == bool(h['flags'] & flag)
            if not entry[key]:continue
            offset = physical*4096
            base = c['mem_base']
            if read == HANDLERS[3]:
                base = v['linear']['pointer']
                offset = (v[bank]+(physical-v['pages']['base'])*4096) & (v['vmemwrap']-1)
            assert entry[key]+int(page)*4096 == base+offset, 'cached host page differs'
    assert v['vmemsize'] > 0 and v['vmemwrap'] > 0
    for key,multiplier in (('linear',1),('fastmem',2)):
        q = v[key]; assert q['bytes'] == multiplier*v['vmemsize']
        assert (folder/q['file']).stat().st_size == q['bytes'], 'incomplete VGA storage'
    return portable(context)


@contextmanager
def legacy_view(root, kind, repo):
    """Only remove added metadata; original CPU/RAM/output expectations remain strict."""
    with tempfile.TemporaryDirectory(prefix='wasm-fist-memory-view-') as temp:
        view = Path(temp); (view/'source').mkdir()
        for path in root.iterdir():
            if path.name not in ('source','proof.json','producers.json'):(view/path.name).symlink_to(path)
        producers = json.loads((root/'producers.json').read_text())
        if kind == 'device':
            from verify_device_start_prefix import producer_paths
            producers = {str(p):producers[str(p)] for p in producer_paths(repo)}
        (view/'producers.json').write_text(json.dumps(producers)+'\n')
        names = {'irq':{'irq-events.jsonl','system-events.jsonl','boot-fetches.jsonl'},
                 'device':{'prefix-fetches.jsonl'}, 'config':{'reset-fetches.jsonl'}}[kind]
        for path in (root/'source').iterdir():
            if path.name in names or (path.suffix == '.json' and kind != 'irq'):
                rows = [json.loads(line) for line in path.read_text().splitlines()] if path.suffix == '.jsonl' else [json.loads(path.read_text())]
                for q in rows:
                    q.pop('memory_context',None)
                    if kind != 'irq':
                        for field in SYSTEM_FIELDS:q.pop(field,None)
                content = ''.join(json.dumps(q)+'\n' for q in rows) if path.suffix == '.jsonl' else json.dumps(rows[0])+'\n'
                (view/'source'/path.name).write_text(content)
            else:(view/'source'/path.name).symlink_to(path)
        yield view


def packet(context):
    """Portable fixture input: explicit provider slots, first-MB map and cache list."""
    p,c,v = context['provider'],context['cache'],context['vga']
    ids = [HANDLERS.index(p['handlers'][str(slot)]['type']) for slot in p['slots']]
    assert all(ids), 'InitPageHandler cannot be a physical provider'
    header = [context['architecture'],int(context['normal_core']),context['auto_determine'],
              p['pages'],int(p['a20_enabled']),p['a20_controlport'],v['vmemsize'],v['vmemwrap'],
              v['svga.bank_read_full'],v['svga.bank_write_full'],v['pages']['base'],v['pages']['mask'],
              len(c['linked_pages']),len(c['entries'])]
    data = header+p['firstmb']+ids+c['linked_pages']
    for page,entry in sorted(c['entries'].items(),key=lambda pair:int(pair[0])):
        data += [int(page),entry['phys_page'],HANDLERS.index(entry['handler_types']['readhandler']),
                 int(bool(entry['read'])),int(bool(entry['write']))]
    return struct.pack('<%dI'%len(data),*data)
