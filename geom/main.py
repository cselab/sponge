import Geom as GC
import MeshLines as ml
import numpy as np
import Utils

H = 100
R = 11
D_SHELIX = 1.3
n=20

NV = 30
NC = 50
AA = 0.25
BB = 0.5
LOOP_NO = 6
R_E = 1.2
NumBelix = 1
NumBelix2 = 1
filenameOut = "ver.stl"
geom = GC.GeomParam()
nodes, elements, bnodes, belements = geom.main(H, R, NV, NC, LOOP_NO, NumBelix,
                                               NumBelix2, D_SHELIX)
if NumBelix + NumBelix2 == 0:
    existBelix = False
else:
    existBelix = True
meshed = ml.MeshLines(elements, nodes, belements, bnodes, R_E, 2*R_E, n, BB, AA,
                      existBelix,NumBelix,NumBelix2)
node_class_list, cubic_list, bcubics = meshed.main()
if existBelix:
    cubics_all = np.vstack((cubic_list, bcubics))
else:
    cubics_all = cubic_list
Utils.convert_to_stl_binary(cubics_all, filenameOut)
