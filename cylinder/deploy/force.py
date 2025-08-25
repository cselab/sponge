import sys
import numpy as np
import re
import matplotlib.pyplot as plt

nvect = 2
nattr = 2

att_path = sys.argv[1]
xyz_path = re.sub("[.]attr[.]raw$", "", sys.argv[1]) + ".xyz.raw"
att = np.memmap(sys.argv[1], dtype=np.dtype("float32"), order="F")
att = np.reshape(att, (-1, nattr + 3 * nvect))

xyz = np.memmap(xyz_path, dtype=np.dtype("float32"), order="F")
xyz = np.reshape(xyz, (-1, 8, 3))

i = 0
Delta = att[:, i]; i += 1
cs = att[:, i]; i += 1
u = att[:, i : i + 3]; i += 3
f = att[:, i : i + 3]; i += 3
coef = (cs - 1) * Delta * Delta * Delta

# print(np.min(u, axis=0), np.max(u, axis=0), np.std(u, axis=0))
ux = u[:, 0]
# plt.hist(ux * Delta * Delta * Delta, bins=100)
dt = 1.3491601059154479e-03
f = ux * Delta * Delta * Delta / dt
lo, hi = np.quantile(f, [0.1, 0.9])
plt.hist(f, bins=200, range=(lo, hi))
plt.show()
