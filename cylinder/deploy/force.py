import sys
import numpy as np

nvect = 2
nattr = 2
d = np.memmap(sys.argv[1], dtype=np.dtype("float32"), order="F")
d = np.reshape(d, (-1, nattr + 3 * nvect))

i = 0
Delta = d[:, i]; i += 1
cs = d[:, i]; i += 1
u = d[:, i : i + 3]; i += 3
f = d[:, i : i + 3]; i += 3
coef = (cs - 1) * Delta * Delta * Delta

print(np.min(u, axis=0), np.max(u, axis=0), np.std(u, axis=0))
