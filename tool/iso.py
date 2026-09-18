import os
import re
import sys
import numpy as np
import amriso

usage = ("usage: iso.py [-Z ZLIM] LEVEL[,LEVEL...] FILE.xdmf2 [FILE.xdmf2 ...]\n"
         "       LEVEL is an |omega| value or lVALUE for lambda2 = -VALUE\n"
         "       FILE.xdmf2 is a full-field output of cylinder -F\n"
         "       -Z ZCUT drops cells with |z| > ZCUT (e.g. the boundary\n"
         "       layers on the end walls, ZCUT below the sponge half-height)\n"
         "       writes FILE.omegaLEVEL.* or FILE.l2VALUE.* triangle meshes\n"
         "       colored by omega.z\n")
args = sys.argv[1:]
zlim = None
if len(args) > 1 and args[0] == "-Z":
    zlim = float(args[1])
    args = args[2:]
if len(args) < 2:
    sys.stderr.write(usage)
    sys.exit(1)
levels = args[0].split(",")
# attributes per cell in cylinder -F output:
# p l2 u.x u.y u.z omega.x omega.y omega.z
nattr = 8
for path in args[1:]:
    base = os.path.splitext(path)[0]
    if re.search(r"\.(omega|l2)[0-9.e+-]+$", base):
        continue  # a mesh written by an earlier iso.py call
    xyz = np.fromfile(base + ".xyz.raw", np.float32).reshape(-1, 8, 3)
    attr = np.fromfile(base + ".attr.raw", np.float32).reshape(-1, nattr)
    assert len(attr) == len(xyz)
    if zlim is not None:
        zc = (xyz[:, :, 2].min(1) + xyz[:, :, 2].max(1)) / 2
        keep = np.abs(zc) < zlim
        xyz = np.ascontiguousarray(xyz[keep])
        attr = attr[keep]
    l2 = np.ascontiguousarray(attr[:, 1])
    wz = attr[:, 7].astype(np.float64)
    omega = np.sqrt((attr[:, 5:8].astype(np.float64)**2).sum(1))
    del attr
    # cells inside the solid have no metric and get huge or infinite vorticity
    bad = ~np.isfinite(omega) | (omega > 1e6)
    omega[bad] = 0
    wz[bad] = 0
    omega = np.ascontiguousarray(omega, np.float32)
    wz = np.ascontiguousarray(wz, np.float32)
    for spec in levels:
        if spec.startswith("l"):
            lv = float(spec[1:])
            x, t, a = amriso.extract3d(xyz, l2, wz, -lv)
            name = "%s.l2%g" % (base, lv)
        else:
            lv = float(spec)
            x, t, a = amriso.extract3d(xyz, omega, wz, lv)
            name = "%s.omega%g" % (base, lv)
        amriso.dump3d(name, x, t, a)
        sys.stderr.write("iso.py: %s ntri=%d\n" % (name, len(t)))
