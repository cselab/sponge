import collections
import os
import struct
import sys

shift = ((0, 0, 0), (0, 0, 1), (0, 1, 0), (0, 1, 1), (1, 0, 0), (1, 0, 1),
         (1, 1, 0), (1, 1, 1))


def ellipse(x, y, z):
    xc = 1 / 2
    yc = 1 / 2
    zc = 1 / 2
    rx = 1 / 4
    ry = 1 / 5
    rz = 1 / 6
    return (x - xc)**2 / rx**2 + (y - yc)**2 / ry**2 + (z - zc)**2 / rz**2 - 1


def cylinder(x, y, z):
    diameter = 10
    return x**2 + y**2 - (diameter / 2)**2


def refinep(x, y, z, delta):
    seen = None
    for dx, dy, dz in shift:
        u = x + delta * (dx - 1 / 2)
        v = y + delta * (dy - 1 / 2)
        w = z + delta * (dz - 1 / 2)
        sign = indicator(u, v, w) > 0
        if sign < 0:
            print(sign)
        if seen != None and sign != seen:
            return True
        else:
            seen = sign
    return False


def traverse(level):
    values = [0] * nfields
    x = X0 + L0 / 2
    y = Y0 + L0 / 2
    z = Z0 + L0 / 2
    for i in range(1, level + 1):
        Delta = L0 * (1 / (1 << i))
        x += Delta * (shift[index[i]][0] - 1 / 2)
        y += Delta * (shift[index[i]][1] - 1 / 2)
        z += Delta * (shift[index[i]][2] - 1 / 2)
    Delta = L0 * (1.0 / (1 << level))
    values[1] = indicator(x, y, z)
    leaf = level >= minlevel and (level == maxlevel
                                  or not refinep(x, y, z, Delta))
    fmt = "%dd" % nfields
    dump.write(struct.pack("I", 2 if leaf else 0))
    pos = dump.tell()
    dump.write(struct.pack(fmt, *values))
    cell_size = 1
    if not leaf:
        for index[level + 1] in range(8):
            cell_size += traverse(level + 1)
    curr = dump.tell()
    dump.seek(pos, os.SEEK_SET)
    dump.write(struct.pack("d", cell_size))
    dump.seek(curr, os.SEEK_SET)
    return cell_size


minlevel = 1
maxlevel = 6
fields = "size", "cs"
nfields = len(fields)
t = 0
i = 0
depth = 7  # TODO:
npe = 1
version = 170901
n = 0, 0, 0
index = collections.defaultdict(int)

# indicator, X0, Y0, Z0, L0 = ellipse, 0, 0, 0, 1
indicator, X0, Y0, Z0, L0 = cylinder, -20, -25, -25, 50

with open("gen.dump", "wb") as dump:
    dump.write(struct.pack("dl4i3d", t, nfields, i, depth, npe, version, *n))
    for field in fields:
        fmt = "I%ds" % len(field)
        dump.write(struct.pack(fmt, len(field), str.encode(field)))
    dump.write(struct.pack("4d", X0, Y0, Z0, L0))
    sys.stderr.write("cells: %ld\n" % traverse(0))
