#!/usr/bin/env python3
"""Compare complete FISTSEQ1 frame and PCM streams; report the first difference."""
import argparse
from itertools import zip_longest
from pathlib import Path
import struct

from sequence_format import exact, validate


def records(path, kind):
    with Path(path).open('rb') as stream:
        exact(stream, 9)
        while (tag := exact(stream, 1)) != b'E':
            if tag != kind.encode():
                raise ValueError('unexpected record type')
            time = struct.unpack('<Q', exact(stream, 8))[0]
            if kind == 'F':
                width, height = struct.unpack('<II', exact(stream, 8))
                yield time, (width, height), exact(stream, 768), exact(stream, width * height)
            else:
                position, rate, frames = struct.unpack('<QII', exact(stream, 16))
                yield time, (position, rate, frames), exact(stream, frames * 4)


def first_byte(left, right):
    return next((i for i, (a, b) in enumerate(zip(left, right)) if a != b), None)


def compare(left, right, kind):
    suffix = '.frames' if kind == 'F' else '.pcm'
    a, b = str(left) + suffix, str(right) + suffix
    validate(a, kind)
    validate(b, kind)
    for index, (ra, rb) in enumerate(zip_longest(records(a, kind), records(b, kind))):
        if ra is None or rb is None:
            event = ra if ra is not None else rb
            return event[0], f'{kind} event {index} missing from {"left" if ra is None else "right"}'
        if ra[0] != rb[0]:
            return min(ra[0], rb[0]), f'{kind} event {index} time: {ra[0]} != {rb[0]} us'
        if ra[1] != rb[1]:
            return ra[0], f'{kind} event {index} layout: {ra[1]} != {rb[1]}'
        for field, left_bytes, right_bytes in zip(('palette', 'pixels') if kind == 'F' else ('PCM',), ra[2:], rb[2:]):
            if left_bytes != right_bytes:
                offset = first_byte(left_bytes, right_bytes)
                return ra[0], f'{kind} event {index} {field} byte {offset}: {left_bytes[offset]} != {right_bytes[offset]}'
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('left')
    parser.add_argument('right')
    parser.add_argument('--frames-only', action='store_true', help='diagnose frames before PCM capture exists')
    args = parser.parse_args()
    try:
        differences = [compare(args.left, args.right, kind)
                       for kind in (('F',) if args.frames_only else ('F', 'A'))]
    except (OSError, ValueError) as error:
        print(f'FAIL: {error}')
        return 1
    if found := [difference for difference in differences if difference]:
        print(f'FAIL: {min(found)[1]}')
        return 1
    print('PASS: complete frame' + ('' if args.frames_only else ' and PCM') + ' streams match')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
