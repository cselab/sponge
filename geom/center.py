#!/usr/bin/env python3

import meshio
import sys

Transpose = False
while True:
    sys.argv.pop(0)
    if len(sys.argv) and len(sys.argv[0]) > 1 and sys.argv[0][0] == '-':
        if sys.argv[0][1] == 'h':
            sys.stderr.write("center.py [-t] file.stl\n")
            sys.exit(1)
        if sys.argv[0][1] == 't':
            Transpose = True
        else:
            sys.stderr.write("center.py: unknown option '%s'\n" % sys.argv[0])
            sys.exit(1)
    else:
        break
if len(sys.argv) == 0:
    sys.stderr.write("center.py: need an input file\n")
    sys.exit(1)
try:
    mesh = meshio.read(sys.argv[0])
except meshio._exceptions.ReadError:
    sys.stderr.write("center.py: fail to read mesh '%s'\n" % sys.argv[0])
    sys.exit(1)
if Transpose:
    x, z, y = zip(*mesh.points)
else:
    x, y, z = zip(*mesh.points)

xlo = min(x)
xhi = max(x)
ylo = min(y)
yhi = max(y)
zlo = min(z)
zhi = max(z)

xc = (xlo + xhi) / 2
yc = (ylo + yhi) / 2
zc = (zlo + zhi) / 2

L = max(xhi - xlo, yhi - ylo, zhi - zlo)
s = 0.95 / L
print(xhi - xlo, yhi - ylo, zhi - zlo, s)

x = [(x - xhi) * s + 0.5 for x in x]
y = [(y - yc) * s for y in y]
z = [(z - zc) * s for z in z]

mesh.points[:, 0] = x
mesh.points[:, 1] = y
mesh.points[:, 2] = z
mesh.write("center.stl", binary=True)

mesh.points[:, 2] = x
mesh.points[:, 1] = y
mesh.points[:, 0] = z
mesh.write("trans.stl", binary=True)
