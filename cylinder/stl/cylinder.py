#!/usr/bin/env python3
import struct
import math
import sys


def circle(z, orient):
    i0 = len(ver)
    ver.append((0, 0, z))
    for i in range(n):
        p = 2 * math.pi * i / n
        x = R * math.cos(p)
        y = R * math.sin(p)
        ver.append((x, y, z))
    for i in range(n):
        j = i + 1
        if j == n: j = 0
        a, b, c = i0, i + 1 + i0, j + 1 + i0
        if orient:
            tri.append((b, a, c))
        else:
            tri.append((a, b, c))


def write(path, ver, tri):
    with open(path, "wb") as out:
        out.write(bytes(80 + 4))
        nt = 0
        for i, j, k in tri:
            out.write(struct.pack('12f', 0, 0, 0, *ver[i], *ver[j], *ver[k]))
            out.write(bytes(2))
            nt += 1
        out.seek(80)
        out.write(struct.pack('<I', nt))


L = 5
R = 0.5
n = 30
ver = []
tri = []
circle(-L / 2, True)
circle(L / 2, False)
for i in range(n):
    a = i + 1
    b = 1 if i == n - 1 else i + 2
    c = a + (n + 1)
    d = b + (n + 1)
    tri.append((a, b, c))
    tri.append((c, b, d))
write("center.stl", ver, tri)
