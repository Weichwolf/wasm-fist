#!/usr/bin/env python3
import argparse
from pathlib import Path
import struct
import sys


def exact(stream, size):
    data = stream.read(size)
    if len(data) != size:
        raise ValueError('incomplete sequence record')
    return data


def validate(path, kind):
    records = samples = 0
    first = last = None
    modes = set()
    rates = set()
    with Path(path).open('rb') as stream:
        if exact(stream, 9) != b'FISTSEQ1' + kind.encode():
            raise ValueError('invalid sequence header')
        while True:
            tag = exact(stream, 1)
            if tag == b'E':
                declared = struct.unpack('<QQ', exact(stream, 16))
                if declared != (records, samples) or stream.read(1):
                    raise ValueError('invalid completion record or trailing data')
                if not records:
                    raise ValueError('empty sequence')
                return dict(records=records, samples=samples, first=first, last=last,
                            modes=sorted(modes), rates=sorted(rates))
            if tag != kind.encode():
                raise ValueError('unexpected record type')
            time_us = struct.unpack('<Q', exact(stream, 8))[0]
            if last is not None and time_us < last:
                raise ValueError('non-monotonic event time')
            first = time_us if first is None else first
            last = time_us
            if kind == 'F':
                width, height = struct.unpack('<II', exact(stream, 8))
                if not (0 < width <= 8192 and 0 < height <= 8192):
                    raise ValueError('invalid frame dimensions')
                exact(stream, 768)
                remaining = width * height
                while remaining:
                    chunk = exact(stream, min(remaining, 65536))
                    remaining -= len(chunk)
                modes.add((width, height, 8))
            else:
                position, rate, frames = struct.unpack('<QII', exact(stream, 16))
                if position != samples or not rate or not frames:
                    raise ValueError('invalid PCM sample position/rate/count')
                remaining = frames * 4
                while remaining:
                    chunk = exact(stream, min(remaining, 65536))
                    remaining -= len(chunk)
                rates.add(rate)
                samples += frames
            records += 1


def extract_frame(path, index, destination):
    count = validate(path, 'F')['records']
    target = index if index >= 0 else count + index
    if not 0 <= target < count:
        raise ValueError('frame index outside capture')
    with Path(path).open('rb') as stream:
        exact(stream, 9)
        for current in range(target + 1):
            if exact(stream, 1) != b'F':
                raise ValueError('unexpected record type')
            time_us, width, height = struct.unpack('<QII', exact(stream, 16))
            palette = exact(stream, 768)
            if current != target:
                stream.seek(width * height, 1)
                continue
            pixels = exact(stream, width * height)
            rgb = b''.join(palette[3 * pixel:3 * pixel + 3] for pixel in pixels)
            Path(destination).write_bytes(f'P6\n{width} {height}\n255\n'.encode() + rgb)
            return time_us, width, height


def main():
    parser = argparse.ArgumentParser(description='Validate complete DOSBox frame/PCM sequence captures.')
    parser.add_argument('prefix')
    parser.add_argument('--frame', type=int)
    parser.add_argument('--ppm')
    args = parser.parse_args()
    if (args.frame is None) != (args.ppm is None):
        parser.error('--frame and --ppm must be used together')
    try:
        for suffix, kind in (('.frames', 'F'), ('.pcm', 'A')):
            result = validate(args.prefix + suffix, kind)
            print(f'{kind}: {result}')
        if args.frame is not None:
            time_us, width, height = extract_frame(args.prefix + '.frames', args.frame, args.ppm)
            print(f'frame={args.frame} time_us={time_us} size={width}x{height} ppm={args.ppm}')
    except (OSError, ValueError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
