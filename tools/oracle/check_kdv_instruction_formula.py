#!/usr/bin/env python3
"""Verify the 7135 decoder instruction formula against DOSBox IP histograms."""
import argparse
from pathlib import Path
import re
import subprocess


def stores(image):
    listing = subprocess.check_output([
        'objdump', '-D', '-b', 'binary', '-mi386', '-M', 'intel',
        '--start-address=0x7135', '--stop-address=0x746b', str(image),
    ], text=True)
    groups = {'two_bit': [], 'one_bit': [], 'raw': []}
    for line in listing.splitlines():
        match = re.match(r'\s*([0-9a-f]+):\s+(?:[0-9a-f]{2} )+\s+mov\s+BYTE PTR', line)
        if not match:
            continue
        ip = int(match[1], 16)
        if 0x716d <= ip < 0x7278:
            groups['two_bit'].append(ip)
        elif 0x7278 <= ip < 0x7383:
            groups['one_bit'].append(ip)
        elif 0x73ad <= ip < 0x7452:
            groups['raw'].append(ip)
    if any(len(group) != 16 for group in groups.values()):
        raise ValueError('decoder store layout differs from the measured image')
    return groups


def calls(path):
    result = []
    counts = None
    for line in Path(path).read_text().splitlines():
        match = re.fullmatch(r'call (\d+) end ([\d.]+) total (\d+) other (\d+)', line)
        if match:
            counts = {}
            result.append((int(match[1]), int(match[3]), int(match[4]), counts))
            continue
        match = re.fullmatch(r'([0-9a-f]{4}) (\d+)', line)
        if match and counts is not None:
            counts[int(match[1], 16)] = int(match[2])
    return result


def predicted(ip, groups):
    at = lambda address: ip.get(address, 0)
    two_bit = at(0x716d)
    one_bit = at(0x7278)
    solid = at(0x7391)
    raw = at(0x73ad)
    skip = at(0x7383) - at(0x738c)
    writes = {name: sum(at(address) for address in addresses)
              for name, addresses in groups.items()}
    if min(two_bit, one_bit, solid, raw, skip) < 0:
        raise ValueError('negative decoder branch count')
    if two_bit + one_bit + solid + raw + skip != at(0x7150):
        raise ValueError('decoder branches do not cover every cell')
    fixed = 7 + 7 * at(0x714a) + 2 * at(0x7155)
    return (fixed + 82 * two_bit + 97 * one_bit + 25 * solid + 64 * raw + 13 * skip
            + 2 * writes['two_bit'] + writes['one_bit'] + writes['raw'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('profile')
    parser.add_argument('--image', default='re_out/fist_image.bin')
    args = parser.parse_args()
    try:
        groups = stores(args.image)
        profiles = calls(args.profile)
        if not profiles:
            raise ValueError('no completed decoder calls')
        mixed = 0
        for index, total, other, ip in profiles:
            actual = sum(count for address, count in ip.items() if 0x7135 <= address <= 0x746a)
            expected = predicted(ip, groups)
            if actual != expected:
                raise ValueError(f'call {index}: decoder {actual}, formula {expected}')
            if total != actual:
                mixed += 1
                if total - actual < other:
                    raise ValueError(f'call {index}: invalid non-decoder count')
        print(f'PASS: {len(profiles)} decoder calls; {mixed} include interleaved non-decoder work')
    except (OSError, subprocess.CalledProcessError, ValueError) as error:
        parser.exit(1, f'FAIL: {error}\n')


if __name__ == '__main__':
    main()
