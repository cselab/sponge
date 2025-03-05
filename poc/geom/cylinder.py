import math

Transpose = True
n = 100
r0 = r1 = 0.3733333285861546 / 2
z0 = -0.4
z1 = 0.4
vert = [ ]
tri = [ ]
vert.append( (0, 0, z0) )
for i in range(n):
    phi = 2 * math.pi * i / n
    x = r0 * math.cos(phi)
    y = r0 * math.sin(phi)
    vert.append((x, y, z0))
vert.append( (0, 0, z1) )
for i in range(n):
    phi = 2 * math.pi * i / n
    x = r1 * math.cos(phi)
    y = r1 * math.sin(phi)
    vert.append((x, y, z1))
for i in range(n):
    j = i + 1
    if j == n:
        j = 0
    tri.append((i + 1, 0, j + 1))
for i in range(n):
    j = i + 1
    if j == n:
        j = 0
    tri.append((j + 2 + n, n + 1, i + 2 + n))
for i in range(n):
    j = i + 1
    if j == n:
        j = 0
    tri.append((i + 1, j + 1, i + 2 + n))
for i in range(n):
    j = i + 1
    if j == n:
        j = 0
    tri.append((j + 2 + n, i + 2 + n, j + 1))
def off():
    print("OFF")
    print("%d %d 0" % (len(vert), len(tri)))
    for x, y, z in vert:
        print("%.16e %.16e %.16e" % (x, y, z))
    for t in tri:
        print("3 %d %d %d" % t)

def ply():
    print("""\
ply
format ascii 1.0
element vertex %d
property float x
property float y
property float z
element face %d
property list uchar int vertex_index
end_header""" % (len(vert), len(tri)))
    for x, y, z in vert:
        print("%.16e %.16e %.16e" % (x, y, z))
    for t in tri:
        print("3 %d %d %d" % t)

if Transpose:
    x, y, z = zip(*vert)
    vert = list(zip(z, y, x))
ply()
