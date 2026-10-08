"""Independent original op-64 bank selection, queue and consumed return model.

Reference evidence only. This describes request admission and header/queue state;
it does not decode, mix, schedule or play PCM.
"""
import hashlib
import struct

BANK_PINS = {
    'DSOUNDS.BIN': (134240, 'eb83b59727f23ab6abc0427d9428ab860fcb03523205b922a93583a3a2d6f615', 15),
    'WVSOUNDS.BIN': (224470, 'fe0fef48d315e390585d6d332ebc1bde2b80e70565413deadb71b719a5dd3ff3', 43),
    'EVSOUNDS.BIN': (206678, 'e03cc8da590a0ab9c3b3beecc9b71035c59b6d94a41fb22637cea054dd8061b5', 43),
}
SILENCE = 0x156b
CHANNELS = (0, 1, 2, 128, 129, 130)
MIXER = ((0x2689, 0x268b), (0x26b3, 0x26b6), (0x26df, 0x26e2))


def bank_records(name, data):
    size, digest, count = BANK_PINS[name]
    if (len(data), hashlib.sha256(data).hexdigest()) != (size, digest):
        raise ValueError('Original sound bank does not match its pin: ' + name)
    records = []
    cursor = 0
    while cursor + 2 < len(data):
        paragraphs, frames, rate = struct.unpack_from('<3H', data, cursor)
        end = cursor + 2 + paragraphs * 16
        if paragraphs == 0 or end > len(data):
            raise ValueError('Original sound record does not fit its bank')
        records.append({'offset': cursor, 'body': cursor + 2, 'paragraphs': paragraphs,
                        'frames': frames, 'rate': rate})
        cursor = end
    if len(records) != count or data[cursor:] != (bytes(2) if name == 'DSOUNDS.BIN' else b''):
        raise ValueError('Original bank record count or tail differs')
    # Some original frame counts exceed paragraph payload space by 1..3 bytes.
    # Queue admission reads only the header. PCM boundary behavior is unproved.
    return tuple(records)


def request(before, registers, banks):
    """Predict every kernel byte and general register before original execution.

    banks maps original mapped base addresses to (unchanged bytes, records).
    No caller-provided sample address stands in for original bank selection.
    """
    data = bytearray(before)
    result = dict(registers)
    packet = result['eax'] & 65535
    channel, sample = packet >> 8, packet & 255
    effect = {'branch': 'disabled', 'channel': None, 'pointer': None,
              'bank': None, 'record': None, 'started': False}
    if data[0x2293] == 0:
        return data, result, effect
    if channel not in CHANNELS:
        raise ValueError('Outside the authored direct/queued channel contract')
    queued = bool(channel & 128)
    result['ebx'] = (result['ebx'] & 0xffffff00) | channel
    if sample == 255:
        pointer = SILENCE
        frames, rate = struct.unpack_from('<HH', data, pointer)
        result['eax'], result['ecx'] = pointer, 0
        result['edx'] &= 0xffffff00
        effect['branch'] = 'silence'
    else:
        voice = bool(sample & 128)
        if voice:
            sample &= 127
            result['eax'] &= 0xffffff7f
        base = struct.unpack_from('<I', data, 0x85b4 if voice else 0x85b0)[0]
        result['esi'] = base
        effect.update(bank='voice' if voice else 'effects', record=sample)
        if base == 0:
            effect['branch'] = 'bank_absent'
            return data, result, effect
        bank, records = banks[base]
        if sample >= len(records):
            raise ValueError('Outside the authored original sound record domain')
        record = records[sample]
        pointer = base + record['body']
        frames, rate = struct.unpack_from('<HH', bank, record['body'])
        result['eax'], result['esi'] = pointer, pointer
        effect['branch'] = 'queued' if queued else 'direct'
    channel &= 127
    result['ebx'] = channel
    result['edx'] &= 255
    if result['ecx'] == 0:
        result['ecx'] = rate
    attenuation = result['edx']
    volume = data[0x1e8b + attenuation]
    if queued:
        struct.pack_into('<I', data, 0x15fb + channel * 4, pointer)
        struct.pack_into('<I', data, 0x1607 + channel * 4, result['ecx'])
        data[0x1616 + channel] = attenuation
        data[0x1613 + channel] = volume
    started = not queued or struct.unpack_from('<I', data, 0x15d7 + channel * 4)[0] == SILENCE
    if started:
        struct.pack_into('<I', data, 0x15d7 + channel * 4, pointer)
        struct.pack_into('<I', data, 0x15e3 + channel * 4, frames)
        struct.pack_into('<I', data, 0x15cb + channel * 4, result['ecx'])
        struct.pack_into('<I', data, 0x15ef + channel * 4, 0x40000)
        attenuation_byte, volume_byte = MIXER[channel]
        data[attenuation_byte], data[volume_byte] = attenuation, volume
    result['eax'] = pointer
    effect.update(channel=channel, pointer=pointer, started=started)
    return data, result, effect
