import sys
import imageio.v3
import matplotlib.pyplot as plt
import matplotlib
import numpy as np
import math

sys.argv.pop(0)
path, = sys.argv
a = imageio.v3.imread(path)
ni, nj = np.shape(a)
b = np.ndarray((ni, nj, 3), dtype=np.dtype("uint8"))
b[:, :, 0] = 255 * np.logical_not(a)

i0, i1 = 45, 730
j0, j1 = 110, 1939

x0, x1 = 1e-1, 1e7
y0, y1 = 1e2, 1e-1

lx0, lx1 = math.log10(x0), math.log10(x1)
ly0, ly1 = math.log10(y0), math.log10(y1)

b[i1, :, 1] = 255
b[i0, :, 1] = 255
b[:, j0, 2] = 255
b[:, j1, 2] = 255
for (i, j), v in np.ndenumerate(a):
    if not v:
        x = (j - j0) / (j1 - j0) * (lx1 - lx0) + lx0
        y = (i - i0) / (i1 - i0) * (ly1 - ly0) + ly0
        print("%.16e %.16e" % (10**x, 10**y))
matplotlib.image.imsave("o.png", b)
