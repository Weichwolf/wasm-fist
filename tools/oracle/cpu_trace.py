#!/usr/bin/env python3
"""Attribute a complete instruction interval from a bounded original CPU trace."""
import argparse
from collections import Counter
import json
from pathlib import Path


def location(value):
    cs, ip = value.split(':')
    return int(cs, 16), int(ip, 16)


def records(path):
    with Path(path).open() as stream:
        magic, first, last = stream.readline().split()
        first, last = int(first), int(last)
        if magic != 'FISTCPU1' or not 0 <= first < last <= 0xffffffff:
            raise ValueError('invalid CPU trace window')
        count = 0
        previous = None
        frequency = None
        for line in stream:
            fields = line.split()
            if fields and fields[0] == 'E':
                if len(fields) != 3 or int(fields[1]) != count or not count or int(fields[2]) < last or stream.read():
                    raise ValueError('invalid CPU trace completion')
                return
            if len(fields) != 22 or fields[0] != 'I':
                raise ValueError('invalid CPU instruction record')
            tick, maximum, left, remaining = map(int, fields[1:5])
            cs, ip, linear = (int(value, 16) for value in fields[5:8])
            registers = tuple(int(value, 16) for value in fields[8:16])
            segments = tuple(location(value) for value in fields[16:])
            if not first <= tick < last or maximum <= 0 or left < 0 or remaining < 0 or left + remaining > maximum:
                raise ValueError('invalid CPU clock state')
            if frequency is not None and maximum != frequency:
                raise ValueError('CPU trace requires fixed cycles')
            frequency = maximum
            cycle = tick * maximum + maximum - left - remaining
            if previous is not None and cycle < previous:
                raise ValueError('non-monotonic CPU instruction time')
            if (cs != segments[1][0] or linear != (segments[1][1] + ip) & 0xffffffff or
                    any(not 0 <= value <= 0xffffffff for value in (ip, linear, *registers)) or
                    any(not 0 <= selector <= 0xffff or not 0 <= base <= 0xffffffff for selector, base in segments)):
                raise ValueError('invalid CPU register state')
            previous = cycle
            count += 1
            yield cycle, (cs, ip), registers, segments
        raise ValueError('incomplete CPU trace')


def interval(path, start, stop):
    if start == stop:
        raise ValueError('instruction interval needs distinct boundaries')
    begin = previous = end = None
    count = 0
    locations = Counter()
    gaps = []
    for record in records(path):
        cycle, address, registers, segments = record
        if begin is None:
            if address != start:
                continue
            begin = record
        if end is not None:
            continue
        if previous is not None:
            cost = cycle - previous[0]
            if cost != 1:
                gaps.append(dict(cs_ip='%04x:%08x' % previous[1], cycles=cost,
                                 eax=previous[2][0], ecx=previous[2][1]))
        if address == stop:
            end = record
            continue
        locations['%04x:%08x' % address] += 1
        count += 1
        previous = record
    if begin is None or end is None:
        raise ValueError('missing complete instruction interval')
    cycles = end[0] - begin[0]
    if cycles != count + sum(gap['cycles'] - 1 for gap in gaps):
        raise ValueError('instruction interval does not account for elapsed cycles')
    return dict(instructions=count, cycles=cycles, extra_cycles=cycles-count,
                gaps=gaps, locations=dict(sorted(locations.items())))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('trace')
    parser.add_argument('--start', type=location, required=True, metavar='CS:IP')
    parser.add_argument('--stop', type=location, required=True, metavar='CS:IP')
    args = parser.parse_args()
    try:
        print(json.dumps(interval(args.trace, args.start, args.stop), indent=2))
    except (OSError, ValueError) as error:
        parser.exit(1, f'FAIL: {error}\n')


if __name__ == '__main__':
    main()
