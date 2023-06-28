import numpy as np
from ansys.mapdl.core import launch_mapdl
mapdl = launch_mapdl()
R = 12.6051E-3
H = 60E-3
NV = 40
NC = 37
D_SHELIX = 0.97804E-3
AA = 0.19453E-3
BB = 0.19376E-3
LOOP_NO = 4
PITCH = H / LOOP_NO
PI = np.pi
DELTA_TH = 2 * PI / NV
DELTA_H = H / (NC - 1)
TH_SHELIX = D_SHELIX / R
NOBUCK = 10
mapdl.prep7()
mapdl.et(1, 'PLANE82')
mapdl.clocal(11, 1, "", "", "", "", "", "", 2)
mapdl.csys(11)
RR = 1.2269E-3
COUNT = 1
for TH in range(0, 181, 15):
    mapdl.k(COUNT, RR, TH, 0)
    COUNT = COUNT + 1
mapdl.flst(3, 13, 3)
SPLINE_COUNT = 1
for I in range(1, 14):
    mapdl.fitem(3, SPLINE_COUNT)
    SPLINE_COUNT = SPLINE_COUNT + 1
mapdl.bsplin("", 'P51X')
mapdl.csys(0)
mapdl.l(1, 13)
mapdl.lesize("All", "", "", 40, "", 1, "", "", 1)
mapdl.al(1, 2)
mapdl.amesh("All")
mapdl.esel("All")
mapdl.eplot("All")
mapdl.secwrite("big_helix")
mapdl.aclear("All")
mapdl.adele("All", "", "", 1)
mapdl.et(1, 'BEAM188')
mapdl.mptemp("", "", "", "", "", "", "")
mapdl.mptemp(1, 0)
mapdl.mpdata("EX", 1, "", 91900000000)
mapdl.mpdata("PRXY", 1, "", 0.17)
mapdl.sectype(1, "BEAM", "RECT", "", 0)
mapdl.secoffset("CENT")
mapdl.secdata(AA, BB, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
mapdl.sectype(2, "BEAM", "MESH")
mapdl.secoffset("ORIG", "", "", "")
mapdl.secread("big_helix", "SECT", "MESH")
COUNT = 1
TH = 0
for II in range(1, NV + 1):
    mapdl.k(COUNT, R * np.cos(TH), R * np.sin(TH), 0)
    TH = TH + DELTA_TH
    COUNT = COUNT + 1
TH = 0
for II in range(1, NV + 1):
    mapdl.k(COUNT, R * np.cos(TH), R * np.sin(TH), H)
    TH = TH + DELTA_TH
    COUNT = COUNT + 1
COUNT_LINE = 1
for II in range(1, NV + 1):
    mapdl.l(COUNT_LINE, COUNT_LINE + NV)
    COUNT_LINE = COUNT_LINE + 1
HH = 0
for II in range(1, NC + 1):
    mapdl.k(COUNT, 0, 0, HH)
    mapdl.circle(COUNT, R)
    COUNT = COUNT + 5
    COUNT_LINE = COUNT_LINE + 4
    HH = HH + DELTA_H
mapdl.lplot("All")
TH0 = TH_SHELIX / 2
TH1 = -TH_SHELIX / 2
HH = 0
for JJ in range(1, int(NV / 2) + 1):
    SPLINE_INIT = COUNT
    HH = 0
    for II in range(1, NC + 1):
        mapdl.k(COUNT, R * np.cos(TH0), R * np.sin(TH0), HH)
        COUNT = COUNT + 1
        TH0 = TH0 + DELTA_TH
        HH = HH + DELTA_H
    mapdl.flst(3, NC, 3)
    for II in range(1, NC + 1):
        mapdl.fitem(3, SPLINE_INIT)
        SPLINE_INIT = SPLINE_INIT + 1
    mapdl.bsplin("", 'P51X')

    COUNT_LINE = COUNT_LINE + 1
    SPLINE_INIT = COUNT
    HH = 0
    for II in range(1, NC + 1):
        mapdl.k(COUNT, R * np.cos(TH1), R * np.sin(TH1), HH)
        COUNT = COUNT + 1
        TH1 = TH1 + DELTA_TH
        HH = HH + DELTA_H
    mapdl.flst(3, NC, 3)

    for II in range(1, NC + 1):
        mapdl.fitem(3, SPLINE_INIT)
        SPLINE_INIT = SPLINE_INIT + 1
    mapdl.bsplin("", 'P51X')

    COUNT_LINE = COUNT_LINE + 1
    TH0 = TH_SHELIX / 2 + (2 * DELTA_TH) * JJ
    TH1 = -TH_SHELIX / 2 + (2 * DELTA_TH) * JJ

mapdl.lplot("All")
TH0 = TH_SHELIX / 2
TH1 = -TH_SHELIX / 2
HH = 0
for JJ in range(1, int(NV / 2) + 1):
    SPLINE_INIT = COUNT
    HH = 0
    for II in range(1, NC + 1):
        mapdl.k(COUNT, R * np.cos(TH0), R * np.sin(TH0), HH)
        COUNT = COUNT + 1
        TH0 = TH0 - DELTA_TH
        HH = HH + DELTA_H
    mapdl.flst(3, NC, 3)
    for II in range(1, NC + 1):
        mapdl.fitem(3, SPLINE_INIT)
        SPLINE_INIT = SPLINE_INIT + 1
    mapdl.bsplin("", 'P51X')

    COUNT_LINE = COUNT_LINE + 1
    SPLINE_INIT = COUNT
    HH = 0
    for II in range(1, NC + 1):
        mapdl.k(COUNT, R * np.cos(TH1), R * np.sin(TH1), HH)
        COUNT = COUNT + 1
        TH1 = TH1 - DELTA_TH
        HH = HH + DELTA_H
    mapdl.flst(3, NC, 3)
    for II in range(1, NC + 1):
        mapdl.fitem(3, SPLINE_INIT)
        SPLINE_INIT = SPLINE_INIT + 1
    mapdl.bsplin("", 'P51X')

    COUNT_LINE = COUNT_LINE + 1
    TH0 = TH_SHELIX / 2 + (2 * DELTA_TH) * JJ
    TH1 = -TH_SHELIX / 2 + (2 * DELTA_TH) * JJ

mapdl.lplot("All")
NUM_KP = mapdl.get('NUM_KP', "KP", 0, "NUM", "MAXD")
COUNT = NUM_KP + 1
HH = 0
TH0 = 0
DELTA_H_BIGHELIX = PITCH / NV
SPLINE_INIT = COUNT
for II in range(1, (NV) * LOOP_NO + 2):
    mapdl.k(COUNT, R * np.cos(TH0), R * np.sin(TH0), HH)
    TH0 = TH0 - DELTA_TH
    HH = HH + DELTA_H_BIGHELIX
    COUNT = COUNT + 1
mapdl.flst(3, (NV) * LOOP_NO + 1, 3)
for II in range(1, (NV) * LOOP_NO + 2):
    mapdl.fitem(3, SPLINE_INIT)
    SPLINE_INIT = SPLINE_INIT + 1
mapdl.bsplin("", 'P51X')
mapdl.lplot("All")
mapdl.lsel("All")
mapdl.btol("1e-10")
mapdl.lovlap("All")
print("Beams merged")
mapdl.lsel("All")
mapdl.lmesh("All")
mapdl.eplot(show_edges=True, smooth_shading=True, show_node_numbering=False)
mapdl.exit()
