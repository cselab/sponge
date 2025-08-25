import matplotlib.patches
import matplotlib.pyplot as plt
import multiprocessing
import numpy as np
import os
import re
import sys


def do(path):
    dtype = np.dtype("float32")
    path = re.sub("\.xdmf2$", "", path)
    path = re.sub("\.attr\.raw$", "", path)
    path = re.sub("\.xyz\.raw$", "", path)

    xyz_path = path + ".xyz.raw"
    attr_path = path + ".attr.raw"
    png_path = path + ".png"
    xyz = np.memmap(xyz_path, dtype)
    ncell = xyz.size // (3 * 8)
    assert ncell * 3 * 8 == xyz.size
    attr = np.memmap(attr_path, dtype)
    attr = attr.reshape((ncell, -1))
    u = attr[:, 2:2 + 3]
    vel = np.sqrt(u[:, 0]**2 + u[:, 1]**2 + u[:, 2]**2)
    patches = []
    colors = []
    for i in range(ncell):
        j = 8 * 3 * i
        x = xyz[j]
        y = xyz[j + 1]
        k = j + 3 * 7
        lx = xyz[k] - x
        ly = xyz[k + 1] - y
        cx = x + lx / 2
        cy = y + ly / 2
        if cx**2 + cy**2 > 1 / 2**2:
            patches.append(matplotlib.patches.Rectangle((x, y), lx, ly))
            colors.append(vel[i])

    L = 2.5
    plt.axis((-L / 2 + L / 10, L / 2 + L / 10, -L / 2, L / 2))
    plt.axis("scaled")
    plt.axis("off")
    p = matplotlib.collections.PatchCollection(patches, cmap=matplotlib.cm.jet)
    p.set_array(colors)
    p.set_clim(0, 2.6)
    plt.gca().add_collection(p)
    # plt.colorbar(p)
    plt.tight_layout()
    plt.savefig(png_path, dpi=75, bbox_inches='tight', pad_inches=0)
    plt.close()
    sys.stderr.write("post.sh: %ld: %ld %s\n" % (os.getpid(), ncell, png_path))


with multiprocessing.Pool(multiprocessing.cpu_count()) as pool:
    pool.map(do, sys.argv[1:])
