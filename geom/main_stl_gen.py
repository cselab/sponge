#%% IMPORTS

import MeshLines as ml
import Utils
import Geom as GC
import numpy as np

def compute(geom,H,R,NV,NC,LOOP_NO,R_E,b,n,BB,AA,NumBelix,NumBelix2,D_SHELIX):
    nodes, elements, bnodes, belements = geom.main(H,R,NV,NC,LOOP_NO,NumBelix,NumBelix2,D_SHELIX)
    if NumBelix + NumBelix2 == 0:
        existBelix = False
    else:
        existBelix = True
    meshed = ml.MeshLines(elements, nodes, belements, bnodes, R_E,2*R_E,n, BB,AA,existBelix,NumBelix,NumBelix2)
    node_class_list,cubic_list, bcubics= meshed.main()
    filenameOut = "ver.stl"
    if existBelix:
        cubics_all = np.vstack((cubic_list,bcubics))
    else:
        cubics_all = cubic_list
    Utils.convert_to_stl_binary(cubics_all, filenameOut)
    return node_class_list,cubic_list, bcubics

#%% Construct geometry test vertical
# all units in mm
geom = GC.GeomParam()
H = 100
R = 11
D_SHELIX = 1.3 #for geom.py
n=20
# can change
NV = 30 # even will produce little crossed holes 20-50
NC = 50 # 30-70
AA = 0.25 #2*AA=recB height? 0.075-0.25
BB = 0.5 # wall thickness 2*BB=recB   2* 0.18-0.4
LOOP_NO = 6 #1-8
R_E = 1.2      #2*a=R_E
NumBelix = 2 #CW
NumBelix2 = 1 #CCW
#
node_class_list,cubic_list, bcubics = compute(geom,H,R,NV,NC,LOOP_NO,R_E,2*R_E,n,BB,AA,NumBelix,NumBelix2,D_SHELIX)
#%%

#%%
#%% test node
# nodeTest = ml.Node(1,0.0126051,0,0)
# #nodeTest.add_face([0.3,0.4,0])
# nodeTest.add_face([0,0,-0.000555556])
# nodeTest.NodePlotter()
#%% test ellipsoid
# import matplotlib.pyplot as plt

# rect_coords = ml.ellipsoid_cs(1, 2, 30)
# rect_coords = rect_coords.reshape(rect_coords.shape[0] * 4, 3)
# v = np.array([1,2,3])
# v = v/np.linalg.norm(v)
# R = ml.rotation_matrix_from_vectors(v)
# projected_points = np.dot(rect_coords, R.T) + np.array([1,2,3])
# fig = plt.figure()
# ax = fig.add_subplot(111, projection='3d')

# # Original points in XY plane
# ax.scatter(rect_coords[:, 0], rect_coords[:, 1], rect_coords[:, 2], c='b', marker='o', label='Original Points')

# # Projected points
# ax.scatter(projected_points[:, 0], projected_points[:, 1], projected_points[:, 2], c='r', marker='x', label='Projected Points')

# # Vector v
# ax.quiver(1, 2, 3, v[0], v[1], v[2], color='g', label='Vector v')

# ax.set_xlabel('X')
# ax.set_ylabel('Y')
# ax.set_zlabel('Z')
# ax.set_title('Projection onto Local Y-axis')
# ax.legend()

# plt.show()
