import matplotlib.patches
import matplotlib.pyplot as plt
import multiprocessing
import numpy as np
import os
import re
import sys
import xml.etree.ElementTree as ET


def do(path):
    dtype = np.dtype("float32")
    path = re.sub("\.xdmf2$", "", path)
    path = re.sub("\.attr\.raw$", "", path)
    path = re.sub("\.xyz\.raw$", "", path)

    xdmf_path = path + ".xdmf2"
    xyz_path = path + ".xyz.raw"
    attr_path = path + ".attr.raw"
    png_path = path + ".png"
    root = ET.parse(xdmf_path)
    time = float(root.find("Domain/Grid/Time").get("Value"))
    xyz = np.memmap(xyz_path, dtype)
    ncell = xyz.size // (3 * 8)
    assert ncell * 3 * 8 == xyz.size
    attr = np.memmap(attr_path, dtype)
    attr = attr.reshape((ncell, -1))
    omega = attr[:, 10]
    patches = []
    colors = []
    colors = np.ndarray(ncell)
    for i in range(ncell):
        j = 8 * 3 * i
        x = xyz[j]
        y = xyz[j + 1]
        k = j + 3 * 7
        lx = xyz[k] - x
        ly = xyz[k + 1] - y
        cx = x + lx / 2
        cy = y + ly / 2
        #if cx**2 + cy**2 > 1 / 2**2:
        patches.append(matplotlib.patches.Rectangle((x, y), lx, ly))
        colors[i] = omega[i]

    L = 8.0
    plt.text(-L / 10,
             4 * L / 10,
             "%s: %8.1f" % (text, time),
             color="blue",
             fontsize="xx-large")
    plt.axis((-L / 10, 9 * L / 10, -L / 2, L / 2))
    plt.axis("scaled")
    plt.axis("off")
    p = matplotlib.collections.PatchCollection(patches, cmap=matplotlib.cm.jet)
    p.set_array(colors)
    p.set_clim(-5, 5)
    plt.gca().add_collection(p)
    # plt.colorbar(p)
    plt.tight_layout()
    plt.savefig(png_path, dpi=400, bbox_inches='tight', pad_inches=0)
    plt.close()
    sys.stderr.write("post.sh: %ld: %ld %s\n" % (os.getpid(), ncell, png_path))


text = sys.argv[1]
with multiprocessing.Pool() as pool:
    pool.map(do, sys.argv[2:])
