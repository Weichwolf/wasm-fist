#!/usr/bin/env python3
import argparse
from contextlib import ExitStack
from pathlib import Path
import struct
import sys


def frame(stream):
    def token():
        value = bytearray()
        while True:
            char = stream.read(1)
            if not char:
                raise ValueError('incomplete PPM header')
            if not value and char == b'#':
                stream.readline()
            elif char.isspace():
                if value:
                    return bytes(value)
            else:
                value += char
                if len(value) > 32:
                    raise ValueError('invalid PPM token')

    if token() != b'P6':
        raise ValueError('expected binary PPM')
    width, height, maximum = (int(token()) for _ in range(3))
    if (width, height, maximum) != (320, 200, 255):
        raise ValueError(f'expected 320x200 RGB8, got {width}x{height} max={maximum}')
    offset = stream.tell()
    size = width * height * 3
    if stream.seek(0, 2) != offset + size:
        raise ValueError('PPM payload length does not match dimensions')
    return (width, height, maximum), offset, size, 3


def pcm(stream):
    length = stream.seek(0, 2)
    stream.seek(0)
    header = stream.read(12)
    if len(header) != 12 or header[:4] != b'RIFF' or header[8:] != b'WAVE':
        raise ValueError('expected RIFF/WAVE')
    if struct.unpack_from('<I', header, 4)[0] + 8 != length:
        raise ValueError('RIFF length does not match file')
    fmt = payload = None
    while stream.tell() < length:
        chunk = stream.read(8)
        if len(chunk) != 8:
            raise ValueError('incomplete WAV chunk header')
        name, size = struct.unpack('<4sI', chunk)
        start = stream.tell()
        end = start + size + (size & 1)
        if end > length:
            raise ValueError('incomplete WAV chunk payload')
        if name == b'fmt ':
            if fmt is not None or size < 16:
                raise ValueError('invalid or duplicate WAV format')
            tag, channels, rate, byte_rate, align, bits = struct.unpack('<HHIIHH', stream.read(16))
            if tag != 1 or not channels or not rate or bits not in (8, 16, 24, 32):
                raise ValueError('expected integer PCM with a valid sample format')
            if align != channels * (bits // 8) or byte_rate != rate * align:
                raise ValueError('inconsistent PCM alignment/rate')
            fmt = channels, rate, bits, align
        elif name == b'data':
            if fmt is None or payload is not None or not size or size % fmt[3]:
                raise ValueError('invalid, empty or duplicate PCM payload')
            payload = start, size
        stream.seek(end)
    if fmt is None or payload is None:
        raise ValueError('missing PCM format/data')
    return fmt[:3], *payload, fmt[3]


def compare(kind, paths):
    loader = {'frame': frame, 'pcm': pcm}[kind]
    with ExitStack() as stack:
        streams = [stack.enter_context(Path(path).open('rb')) for path in paths]
        captures = [loader(stream) for stream in streams]
        for stream, (_, offset, _, _) in zip(streams, captures):
            stream.seek(offset)
        fmt, _, size, unit = captures[0]
        if len(streams) == 1:
            return f'valid {kind}: format={fmt}, units={size // unit}'
        other_fmt, _, other_size, _ = captures[1]
        if fmt != other_fmt:
            raise ValueError(f'{kind} format differs: {fmt} != {other_fmt}')
        if size != other_size:
            raise ValueError(f'{kind} length differs: {size // unit} != {other_size // unit} units')
        offset = 0
        while offset < size:
            count = min(65536, size - offset)
            a, b = (stream.read(count) for stream in streams)
            if len(a) != count or len(b) != count:
                raise ValueError('capture truncated during comparison')
            if a != b:
                index = offset + next(i for i, (x, y) in enumerate(zip(a, b)) if x != y)
                location = (f'x={(index // 3) % 320}, y={(index // 3) // 320}, RGB lane={index % 3}'
                            if kind == 'frame' else f'sample frame={index // unit}, byte lane={index % unit}')
                raise ValueError(f'{kind} differs at {location} (payload byte {index})')
            offset += count
        return f'equal {kind}: format={fmt}, units={size // unit}'


def main():
    parser = argparse.ArgumentParser(description='Strict captured RGB-frame or PCM comparison (board:0033).')
    parser.add_argument('kind', choices=('frame', 'pcm'))
    parser.add_argument('a')
    parser.add_argument('b', nargs='?')
    args = parser.parse_args()
    try:
        print(compare(args.kind, [args.a, args.b] if args.b else [args.a]))
    except (OSError, ValueError, struct.error) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
