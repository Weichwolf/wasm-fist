"""Independent original PATH/PINF wire observation shared by complete-world tests."""
import struct

PLATOONS = 8
PATH_RECORD = 268
PATH_HEADER = 12
WAYPOINTS = 32
DESCRIPTOR_RECORD = 22


def scenario_order_blocks(data):
    chunks = {}
    offset = 0
    while offset < len(data):
        if len(data) - offset < 6:
            raise ValueError('Incomplete scenario chunk header')
        tag, length = struct.unpack_from('<4sH', data, offset)
        end = offset + 6 + length
        if end > len(data) or tag in chunks:
            raise ValueError('Incomplete or duplicate scenario chunk')
        chunks[tag] = data[offset + 6:end]
        offset = end
    return chunks[b'PATH'], chunks[b'PINF']


def orders_lines(blocks=None):
    if blocks is None:
        return 'orders 0\n'
    paths, descriptors = blocks
    if len(paths) != PLATOONS * PATH_RECORD or len(descriptors) != PLATOONS * DESCRIPTOR_RECORD:
        raise ValueError('Incomplete mission orders')
    lines = ['orders 1']
    for platoon in range(PLATOONS):
        route = paths[platoon * PATH_RECORD:(platoon + 1) * PATH_RECORD]
        if route[0] > WAYPOINTS:
            raise ValueError('Route count exceeds original editor capacity')
        lines.append(route_line(platoon, route).rstrip('\n'))
    for platoon in range(PLATOONS):
        words = struct.unpack_from('<11H', descriptors, platoon * DESCRIPTOR_RECORD)
        lines.append(f'descriptor {platoon} ' + ' '.join(map(str, words)))
    return '\n'.join(lines) + '\n'


def route_line(platoon, route):
    if len(route) != PATH_RECORD:
        raise ValueError('Incomplete owned route observation')
    coordinates = struct.unpack_from('<64i', route, PATH_HEADER)
    return (f'route {platoon} {route[0]} ' + route[1:PATH_HEADER].hex(' ') +
            ' ' + ' '.join(map(str, coordinates)) + '\n')


def constructed_blocks(count=32, salt=0):
    """Declared fixtures with nonzero unused slots/header/unknown words and signed edges."""
    paths = bytearray()
    descriptors = bytearray()
    edges = (-2147483648, 2147483647, -1, 0, 1, -65537, 65536, 123456789)
    words = (0, 1, 3, 4, 5, 32767, 32768, 65534, 65535)
    for platoon in range(PLATOONS):
        paths.append(count)
        paths.extend((salt + platoon * 31 + index * 23) % 256 for index in range(11))
        paths.extend(struct.pack('<64i', *(edges[(index + platoon + salt) % len(edges)]
                                          for index in range(64))))
        descriptors.extend(struct.pack('<11H', *(words[(index + platoon + salt) % len(words)]
                                                for index in range(11))))
    return bytes(paths), bytes(descriptors)
