#/usr/bin/env python3

import struct
import mmap
import numpy as np
import sys

input_path, output_path, *rest = sys.argv[1:]
with open(input_path, "rb") as f:
    mm = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ)
nt, = struct.unpack("<I", mm[80:84])
a = np.ndarray((nt, 3, 3),
               dtype=np.dtype("<f4"),
               buffer=mm,
               offset=80 + 4 + 12,
               strides=(50, 12, 4))
tag = b"OFF BINARY\n"
h = len(tag) + 3 * 4
size = h + 4 * (3 * 3 * nt + 5 * nt)
with open(output_path, "wb") as f:
    f.seek(size - 1)
    f.write(b'\0')
with open(output_path, "rb+") as f:
    mm = mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_WRITE)
mm[:h] = tag + struct.pack(">iii", 3 * nt, nt, 0)
ver = np.ndarray((nt, 3, 3), dtype=np.dtype(">f4"), buffer=mm, offset=h)
tri = np.ndarray((nt, 5),
                 dtype=np.dtype(">i4"),
                 buffer=mm,
                 offset=h + ver.nbytes)
tri[:, 0].fill(3)
np.copyto(ver, a)
np.copyto(tri[:, 1:4], np.reshape(np.arange(3 * nt), (nt, 3)))
