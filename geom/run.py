#!/usr/bin/env python

import sys
import os
import itertools


def add(l):
    return [[(key, v) for v in val] for (key, val) in l.items()]


l = {
    'NV': (20, 50),
    'NC': (30, 74),
    'AA': (0.15, 0.65),
    'BB': (0.15, 0.65),
    'LOOP_NO': (1, 6),
    'R_E': (0.6, 1.6),
    'NumBelix': (0, 3),
    'NumBelix2': (0, 3),
}
l = itertools.chain(itertools.product(*add(l)))
for i, x in enumerate(l):
    path = "%08d" % int(i)
    os.makedirs(path, exist_ok=True)
    with open(os.path.join(path, "config.py"), "w") as f:
        f.write("""\
NV = %(NV)d
NC = %(NC)d
AA = %(AA).16e
BB = %(BB).16e
LOOP_NO = %(LOOP_NO)d
R_E = %(R_E).16e
NumBelix = %(NumBelix)d
NumBelix2 = %(NumBelix2)d
""" % dict(x))
