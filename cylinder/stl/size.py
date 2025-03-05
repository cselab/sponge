#!/usr/bin/env python3
import numpy as np
import mmap
import sys
import struct
import os
import argparse

def read(path):
    with open(path, "rb") as file:
        mm = mmap.mmap(file.fileno(), length=0, access=mmap.ACCESS_READ)
        nt, = struct.unpack('<i', mm[80:80 + 4])
        return np.ndarray((nt, 3, 3), np.dtype("<f4"), mm, 80 + 4 + 12,
                          (36 + 12 + 2, 12, 4))

parser = argparse.ArgumentParser(description="Print the sizes of the STL file.")
parser.add_argument("input", type=str, help="Path to the input STL file")
args = parser.parse_args()
r = read(args.input)
nt = len(r)
xlo, ylo, zlo = np.min(r, axis=(0, 1))
xhi, yhi, zhi = np.max(r, axis=(0, 1))
sys.stdout.write("%.16e %.16e\n" % (zlo, zhi))
