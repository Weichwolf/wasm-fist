#!/usr/bin/env python3
"""Capture startup's first complete indexed frame and its actual device producers."""
import argparse
import ast
import json
import struct
from pathlib import Path

from capture_cpu_core_exit import capture
from capture_cpu_pit_writes import observer as pit_observer
from capture_cpu_task_gate import observer as task_observer, verify as verify_task, source_inputs
from memory_context import validate_context
from sequence_format import exact

KINDS = ('before-startup-draw-1', 'after-startup-draw-1',
         'before-startup-draw-2', 'before-startup-present', 'after-startup-draw-2')
RENDER_PARTS = ('renderer_cache', 'renderer_palette', 'renderer_aspect',
                'renderer_changed', 'renderer_skip')


def observer(repo):
    text = task_observer(repo, True)
    pit = ast.parse(pit_observer(repo).split('python\n', 1)[1].rsplit('\nend\nrun', 1)[0])
    state = next(n for n in pit.body if isinstance(n, ast.FunctionDef) and n.name == 'state')
    state.name = 'startup_pit_state'
    for node in ast.walk(state):
        if isinstance(node, ast.Name) and node.id == 'original_state':
            node.id = 'startup_cpu_state'
    drawing = ast.parse((repo / 'tools/oracle/vga_callbacks.gdb.inc').read_text())
    fields = next(n for n in drawing.body if isinstance(n, ast.Assign)
                  and any(isinstance(t, ast.Name) and t.id == 'DRAW_FIELDS' for t in n.targets))
    draw = next(n for n in drawing.body if isinstance(n, ast.FunctionDef) and n.name == 'draw_state')
    definitions = ast.unparse(ast.Module(body=[state, fields, draw], type_ignores=[]))
    helper = (repo / 'tools/oracle/startup_vga_frame.gdb.inc').read_text()
    assert text.count('\nend\nrun') == 1
    text = text.replace('\nend\nrun', '\nstartup_cpu_state=state\n' + definitions
                        + '\n' + helper + '\nend\nrun')
    ast.parse(text.split('python\n', 1)[1].rsplit('\nend\nrun', 1)[0])
    return text


def verify(repo, root):
    result = verify_task(repo, root, True)
    folder = root / 'source'
    rows = [json.loads(s) for s in (folder / 'vga-events.jsonl').read_text().splitlines()]
    lines = [json.loads(s) for s in (folder / 'vga-lines.jsonl').read_text().splitlines()]
    assert [q['kind'] for q in rows] == list(KINDS)
    assert len(lines) == 100
    initial = result['events'][0]
    for q in [initial, *rows]:
        validate_context(q['memory_context'], folder, repo)
        assert (folder / q['memory_file']).stat().st_size == 16777216
        for key in RENDER_PARTS:
            part = q[key]
            data = (folder / part['file']).read_bytes()
            import hashlib
            assert len(data) == part['bytes'] and hashlib.sha256(data).hexdigest() == part['sha256']
    for round_number, before, after in ((1, rows[0], rows[1]), (2, rows[2], rows[4])):
        group = [q for q in lines if q['round'] == round_number]
        assert len(group) == before['arguments']['lines'] == 50
        a, b = before['renderer_device'], after['renderer_device']
        assert a['scale.inLine'] + 50 == b['scale.inLine']
        assert a['scale.outLine'] + 50 == b['scale.outLine']
        assert a['cache_cursor'] + 50 * a['scale.cachePitch'] == b['cache_cursor']
        cache = (folder / before['renderer_cache']['file']).read_bytes()
        for index, line in enumerate(group):
            data = bytes.fromhex(line['data_hex'])
            cursor = a['cache_cursor'] + index * a['scale.cachePitch']
            assert data == cache[cursor:cursor + len(data)]
        changed = bytearray((folder / before['renderer_changed']['file']).read_bytes())
        aspect = (folder / before['renderer_aspect']['file']).read_bytes()
        first = struct.unpack_from('<H', changed)[0]
        struct.pack_into('<H', changed, 0, first + sum(aspect[a['scale.inLine']:b['scale.inLine']]))
        assert bytes(changed) == (folder / after['renderer_changed']['file']).read_bytes()
        assert before['CPU_Cycles'] == after['CPU_Cycles'] == 0
        assert before['CPU_CycleLeft'] == after['CPU_CycleLeft']
        assert before['PIC_event_service'] == after['PIC_event_service']
    present = rows[-2]
    renderer = present['renderer_device']
    assert present['arguments']['abort'] == 0
    assert renderer['scale.inLine'] == renderer['src.height'] == 200
    assert renderer['updating'] == 1 and rows[-1]['renderer_device']['updating'] == 0
    assert rows[-1]['renderer_device']['frameskip.index'] == (renderer['frameskip.index'] + 1) & 15
    assert rows[-1]['renderer_device']['scale.hasOutWrite'] == renderer['CaptureState'] == 0
    cache = (folder / present['renderer_cache']['file']).read_bytes()
    palette = (folder / present['renderer_palette']['file']).read_bytes()
    width, height, pitch = (renderer[k] for k in ('src.width', 'src.height', 'scale.cachePitch'))
    pixels = b''.join(cache[y * pitch:y * pitch + width] for y in range(height))
    colors = b''.join(palette[i * 4:i * 4 + 3] for i in range(256))
    phase = struct.unpack('<f', struct.pack('<f', (present['CPU_CycleMax'] - present['CPU_CycleLeft']
                                                - present['CPU_Cycles']) / present['CPU_CycleMax']))[0]
    time_us = int((present['PIC_Ticks'] + phase) * 1000 + .5)
    matches = []
    with (folder / 'sequence.frames').open('rb') as f:
        assert exact(f, 9) == b'FISTSEQ1F'
        index = 0
        while True:
            tag = exact(f, 1)
            if tag == b'E':
                break
            assert tag == b'F'
            at, w, h = struct.unpack('<QII', exact(f, 16))
            data = exact(f, 768 + w * h)
            if at == time_us:
                assert (w, h) == (width, height) and data == colors + pixels
                matches.append(index)
            index += 1
    assert len(matches) == 1
    result.update(scope='Original startup task-gate observations plus both first VGA draws and '
                  'the first complete indexed frame, all palette entries and presentation time. '
                  'Full CPU/RAM/device/calendar/PF/CR2 and reached renderer effects are retained. '
                  'Complete600ms39frame/27518PCM output is unchanged. No port runtime adoption '
                  'or complete original sequence acceptance.', vga_events=rows, vga_lines=lines,
                  presented_frame_index=matches[0], presented_frame_time_us=time_us,
                  complete_original_acceptance=False)
    (root / 'proof.json').write_text(json.dumps(result, indent=2) + '\n')
    print('PASS original first complete startup frame', matches[0], time_us, 'us/100 lines', flush=True)
    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--verify-only', action='store_true')
    args = parser.parse_args()
    repo, root = args.repo.resolve(strict=True), args.output.resolve()
    assert root.is_relative_to(Path('/tmp'))
    if args.verify_only:
        verify(repo, root)
    else:
        tree = repo / 'third_party/dosbox-build/dosbox-0.74-3'
        capture(repo, root, make_observer=observer, check_capture=verify,
                additional_inputs=(*source_inputs(repo), Path(__file__),
                    repo / 'tools/oracle/capture_cpu_pit_writes.py',
                    repo / 'tools/oracle/vga_callbacks.gdb.inc',
                    repo / 'tools/oracle/startup_vga_frame.gdb.inc',
                    *[tree / p for p in ('src/hardware/timer.cpp', 'src/hardware/vga_draw.cpp',
                        'src/hardware/vga_misc.cpp', 'include/vga.h', 'include/render.h',
                        'src/gui/render.cpp', 'src/gui/render_scalers.h', 'src/gui/render_scalers.cpp',
                        'src/gui/fist_sequence_capture.h', 'src/cpu/paging.cpp')]),
                source_wall_seconds=300)
