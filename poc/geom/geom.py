#!/usr/bin/env python3

import numpy as np

R = 12.6051E-3
H = 60E-3
NV = 38
NC = 33
D_SHELIX = 1.155E-3
AA = 0.198E-3
BB = 0.198E-3
LOOP_NO = 4
PITCH = H / LOOP_NO
PI = 3.1415927
DELTA_TH = 2 * PI / NV
DELTA_H = H / (NC - 1)
TH_SHELIX = D_SHELIX / R
NOBUCK = 1

vertices = []
count = 1
th = 0

for _ in range(NV):
    x = R * np.cos(th)
    y = R * np.sin(th)
    z = 0
    vertices.append([x, y, z])
    th += DELTA_TH
    count += 1

th = 0
for _ in range(NV):
    x = R * np.cos(th)
    y = R * np.sin(th)
    z = H
    vertices.append([x, y, z])
    th += DELTA_TH
    count += 1

lines = []
count_line = 1
for _ in range(NV):
    line = [count_line, count_line + NV]
    lines.append(line)
    count_line += 1

hh = 0
for _ in range(NC):
    x = 0
    y = 0
    z = hh
    vertices.append([x, y, z])
    circle = [count]
    vertices.append(circle)
    for i in range(NV):
        start_vertex = count + i * 2
        end_vertex = count + ((i + 1) % NV) * 2
        lines.append([start_vertex, end_vertex])
    count += 2 * NV
    count_line += 4
    hh += DELTA_H

# Write the OBJ file
with open('output.obj', 'w') as file:
    for vertex in vertices:
        file.write(f'v {" ".join(str(value) for value in vertex)}\n')

    for line in lines:
        file.write(f'l {" ".join(str(value) for value in line)}\n')
