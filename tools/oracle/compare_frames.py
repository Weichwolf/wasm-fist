#!/usr/bin/env python3
import argparse
from contextlib import ExitStack
import struct
import sys
from sequence_format import exact, validate


def compare(a, b):
    left = validate(a, 'F')
    right = validate(b, 'F')
    with ExitStack() as stack:
        files = [stack.enter_context(open(path, 'rb')) for path in (a, b)]
        for stream in files:
            exact(stream, 9)
        frame = 0
        while True:
            tags = [exact(stream, 1) for stream in files]
            if tags == [b'E', b'E']:
                return f'equal {frame} frame events'
            if tags != [b'F', b'F']:
                raise ValueError(f'frame {frame}: one stream ends before the other')
            headers = [struct.unpack('<QII', exact(stream, 16)) for stream in files]
            if headers[0] != headers[1]:
                raise ValueError(f'frame {frame}: time/dimensions {headers[0]} != {headers[1]}')
            time_us, width, height = headers[0]
            palettes = [exact(stream, 768) for stream in files]
            if palettes[0] != palettes[1]:
                lane = next(i for i, (a, b) in enumerate(zip(*palettes)) if a != b)
                raise ValueError(f'frame {frame} at {time_us} us: palette entry={lane // 3} '
                                 f'lane={lane % 3}, value {palettes[0][lane]} != {palettes[1][lane]}')
            offset = 0
            remaining = width * height
            while remaining:
                size = min(65536, remaining)
                chunks = [exact(stream, size) for stream in files]
                if chunks[0] == chunks[1]:
                    offset += size
                    remaining -= size
                    continue
                byte = next(i for i, (a, b) in enumerate(zip(*chunks)) if a != b)
                pixel = offset + byte
                left, right = chunks[0][byte], chunks[1][byte]
                raise ValueError(f'frame {frame} at {time_us} us: pixel x={pixel % width} '
                                 f'y={pixel // width}, indices {left} != {right}')
                offset += size
                remaining -= size
            frame += 1


def main():
    parser = argparse.ArgumentParser(description='Compare complete indexed frame sequences.')
    parser.add_argument('a')
    parser.add_argument('b')
    args = parser.parse_args()
    try:
        print(compare(args.a, args.b))
    except (OSError, ValueError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
