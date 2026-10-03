"""Original resident code with its actual nested-MZ WORD relocations applied."""
import json
from pathlib import Path
import struct


def load(repo):
    asset = (Path(repo) / 'armoredfist/FIST.RUN').read_bytes()
    anchor = json.loads((Path(repo) / 'tools/oracle/sb_irq_frame_case.json').read_text())[
        'resident_entry_region']
    bias = anchor['asset_file_offset'] - anchor['ip']
    headers = []
    for offset in range(len(asset) - 28):
        if asset[offset:offset+2] != b'MZ':
            continue
        words = struct.unpack_from('<14H', asset, offset)
        if offset + words[4]*16 + words[11]*16 == bias:
            headers.append((offset, words))
    assert len(headers) == 1, 'Resident IRQ anchor must identify one MZ header'
    header, words = headers[0]
    image_offset = header + words[4]*16
    entries = [struct.unpack_from('<HH', asset, header + words[12] + index*4)
               for index in range(words[3])]
    offsets = [offset + segment*16 for offset, segment in entries]
    assert all(offset + 2 <= len(asset) - image_offset for offset in offsets)
    return dict(asset=asset, file_bias=bias, header=header, initial_cs=words[11],
                image_offset=image_offset, relocations=offsets)


def relocate(model, code_base):
    assert code_base & 15 == 0
    delta = (code_base >> 4) - model['initial_cs']
    image = bytearray(model['asset'][model['image_offset']:])
    for offset in model['relocations']:
        value = struct.unpack_from('<H', image, offset)[0]
        struct.pack_into('<H', image, offset, (value + delta) & 65535)
    return image[model['initial_cs']*16:]
