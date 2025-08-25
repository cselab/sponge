import numpy as np
import scipy.interpolate
import scipy.spatial
import struct
import config


class GeomParam():

    def __init__(self):
        self.elements = []
        self.nodes = []
        self.belements = []
        self.bnodes = []
        self.R = 0
        self.H = 0
        self.NV = 0
        self.DELTA_TH = 0
        self.NC = 0
        self.DELTA_H = self.H / (self.NC - 1)
        self.D_SHELIX = 0
        self.TH_SHELIX = 0
        self.LOOP_NO = 0
        self.PITCH = 0
        self.COUNT = 1
        return

    def main(self, H, R, NV, NC, LOOP_NO, NumBhelix, NumBhelix2, D_SHELIX):
        self.ChangeParams(H, R, NV, NC, LOOP_NO, NumBhelix, NumBhelix2,
                          D_SHELIX)
        self.VerticalBeams()
        self.Circumferential()
        self.SmallHelix()
        self.SmallHelix2()
        self.BigHelix()
        self.BigHelix2()
        return np.array(self.nodes), np.array(self.elements), np.array(
            self.bnodes), np.array(self.belements)

    def ChangeParams(self, H, R, NV, NC, LOOP_NO, NumBhelix, NumBhelix2,
                     D_SHELIX):
        self.elements = []
        self.nodes = []
        self.belements = []
        self.bnodes = []
        self.COUNT_LINE = 1
        self.bCOUNT_LINE = 1

        self.R = R
        self.H = H
        self.NV = NV
        self.DELTA_TH = 2 * np.pi / self.NV
        self.NC = NC
        self.DELTA_H = self.H / (self.NC - 1)
        self.D_SHELIX = D_SHELIX
        self.TH_SHELIX = self.D_SHELIX / self.R
        self.LOOP_NO = LOOP_NO
        self.PITCH = self.H / self.LOOP_NO
        self.NumBhelix = NumBhelix
        self.NumBhelix2 = NumBhelix2
        return

    def VerticalBeams(self):
        NV = self.NV
        H = self.H
        DELTA_TH = self.DELTA_TH
        R = self.R
        COUNT = self.COUNT
        TH = 0
        for II in range(1, NV + 1):
            self.nodes.append([R * np.cos(TH), R * np.sin(TH), 0])
            TH = TH + DELTA_TH
            COUNT = COUNT + 1

        TH = 0
        for II in range(1, NV + 1):
            self.nodes.append([R * np.cos(TH), R * np.sin(TH), H])
            TH = TH + DELTA_TH
            COUNT = COUNT + 1
        COUNT_LINE = 1
        for II in range(1, NV + 1):
            self.elements.append([COUNT_LINE, COUNT_LINE + NV])
            COUNT_LINE = COUNT_LINE + 1
        COUNT_LINE = COUNT_LINE + NV
        self.COUNT_LINE = COUNT_LINE

    def Circumferential(self):
        NC = self.NC
        R = self.R
        COUNT_LINE = self.COUNT_LINE
        DELTA_H = self.DELTA_H
        HH = 0
        for II in range(1, NC + 1):
            for theta in range(360):
                self.nodes.append([
                    R * np.cos(np.radians(theta)),
                    R * np.sin(np.radians(theta)), HH
                ])
                if theta != 359:
                    self.elements.append([COUNT_LINE, COUNT_LINE + 1])
                else:
                    self.elements.append([COUNT_LINE, COUNT_LINE - 359])
                COUNT_LINE += 1
            HH = HH + DELTA_H
        self.COUNT_LINE = COUNT_LINE

    def SmallHelix(self):

        def bspline(points):
            points = np.array(points)
            tck, u = scipy.interpolate.splprep(points.T, s=0)
            u_new = np.linspace(u.min(), u.max(), points.shape[0] * 5)
            x_new, y_new, z_new = scipy.interpolate.splev(u_new, tck)
            spline_nodes = np.column_stack((x_new, y_new, z_new))
            return spline_nodes

        COUNT_LINE = self.COUNT_LINE
        TH_SHELIX = self.TH_SHELIX
        NV = self.NV
        NC = self.NC
        R = self.R
        DELTA_TH = self.DELTA_TH
        DELTA_H = self.DELTA_H
        TH0 = TH_SHELIX / 2
        TH1 = -TH_SHELIX / 2

        for JJ in range(1, int(NV / 2) + 2):
            unsplined_nodes = []
            HH = 0
            for II in range(1, NC + 1):
                unsplined_nodes.append([R * np.cos(TH0), R * np.sin(TH0), HH])
                TH0 = TH0 + DELTA_TH
                HH = HH + DELTA_H

            splined_nodes = bspline(unsplined_nodes)
            self.elements.extend([[i + COUNT_LINE, i + 1 + COUNT_LINE]
                                  for i in range(len(splined_nodes) - 1)])
            COUNT_LINE += len(splined_nodes)
            self.nodes.extend(splined_nodes.tolist())

            unsplined_nodes = []
            HH = 0
            for II in range(1, NC + 1):
                unsplined_nodes.append([R * np.cos(TH1), R * np.sin(TH1), HH])
                TH1 = TH1 + DELTA_TH
                HH = HH + DELTA_H

            splined_nodes = bspline(unsplined_nodes)
            self.elements.extend([[i + COUNT_LINE, i + 1 + COUNT_LINE]
                                  for i in range(len(splined_nodes) - 1)])
            COUNT_LINE += len(splined_nodes)
            self.nodes.extend(splined_nodes.tolist())
            TH0 = TH_SHELIX / 2 + (2 * DELTA_TH) * JJ
            TH1 = -TH_SHELIX / 2 + (2 * DELTA_TH) * JJ

        self.COUNT_LINE = COUNT_LINE

    def SmallHelix2(self):
        COUNT_LINE = self.COUNT_LINE
        TH_SHELIX = self.TH_SHELIX
        NV = self.NV
        NC = self.NC
        R = self.R
        DELTA_TH = self.DELTA_TH
        DELTA_H = self.DELTA_H
        TH0 = TH_SHELIX / 2
        TH1 = -TH_SHELIX / 2

        for JJ in range(1, int(NV / 2) + 2):
            unsplined_nodes = []
            HH = 0
            for II in range(1, NC + 1):
                unsplined_nodes.append([R * np.cos(TH0), R * np.sin(TH0), HH])
                TH0 = TH0 - DELTA_TH
                HH = HH + DELTA_H

            splined_nodes = bspline(unsplined_nodes)
            self.elements.extend([[i + COUNT_LINE, i + 1 + COUNT_LINE]
                                  for i in range(len(splined_nodes) - 1)])
            COUNT_LINE += len(splined_nodes)
            self.nodes.extend(splined_nodes.tolist())

            unsplined_nodes = []
            HH = 0
            for II in range(1, NC + 1):
                unsplined_nodes.append([R * np.cos(TH1), R * np.sin(TH1), HH])
                TH1 = TH1 - DELTA_TH
                HH = HH + DELTA_H

            splined_nodes = bspline(unsplined_nodes)
            self.elements.extend([[i + COUNT_LINE, i + 1 + COUNT_LINE]
                                  for i in range(len(splined_nodes) - 1)])
            COUNT_LINE += len(splined_nodes)
            self.nodes.extend(splined_nodes.tolist())
            TH0 = TH_SHELIX / 2 + (2 * DELTA_TH) * JJ
            TH1 = -TH_SHELIX / 2 + (2 * DELTA_TH) * JJ

        self.COUNT_LINE = COUNT_LINE

    def BigHelix(self):
        NV = self.NV
        LOOP_NO = self.LOOP_NO
        PITCH = self.PITCH
        R = self.R
        DELTA_TH = self.DELTA_TH
        bCOUNT_LINE = self.bCOUNT_LINE
        NumBhelix = self.NumBhelix
        if NumBhelix == 0:
            return
        thetainit = (2 * np.pi) / (NumBhelix)
        for i in range(NumBhelix):
            HH = 0
            TH0 = thetainit * i
            DELTA_H_BIGHELIX = PITCH / NV
            unsplined_nodes = []
            for II in range(1, (NV) * LOOP_NO + 2):
                unsplined_nodes.append([R * np.cos(TH0), R * np.sin(TH0), HH])
                TH0 = TH0 - DELTA_TH
                HH = HH + DELTA_H_BIGHELIX
            splined_nodes = bspline(unsplined_nodes)
            self.belements.extend([[i + bCOUNT_LINE, i + 1 + bCOUNT_LINE]
                                   for i in range(len(splined_nodes) - 1)])
            bCOUNT_LINE += len(splined_nodes)
            self.bnodes.extend(splined_nodes.tolist())
        self.bCOUNT_LINE = bCOUNT_LINE

    def BigHelix2(self):
        NV = self.NV
        LOOP_NO = self.LOOP_NO
        PITCH = self.PITCH
        R = self.R
        DELTA_TH = self.DELTA_TH
        bCOUNT_LINE = self.bCOUNT_LINE
        NumBhelix = self.NumBhelix2
        if NumBhelix == 0:
            return
        thetainit = (2 * np.pi) / (NumBhelix)
        for i in range(NumBhelix):
            HH = 0
            TH0 = thetainit * i
            DELTA_H_BIGHELIX = PITCH / NV
            unsplined_nodes = []
            for II in range(1, (NV) * LOOP_NO + 2):
                unsplined_nodes.append([R * np.cos(TH0), R * np.sin(TH0), HH])
                TH0 = TH0 + DELTA_TH
                HH = HH + DELTA_H_BIGHELIX
            splined_nodes = bspline(unsplined_nodes)
            self.belements.extend([[i + bCOUNT_LINE, i + 1 + bCOUNT_LINE]
                                   for i in range(len(splined_nodes) - 1)])
            bCOUNT_LINE += len(splined_nodes)
            self.bnodes.extend(splined_nodes.tolist())
        self.bCOUNT_LINE = bCOUNT_LINE


def bspline(points):
    points = np.array(points)
    tck, u = scipy.interpolate.splprep(points.T, s=0)
    u_new = np.linspace(u.min(), u.max(), points.shape[0] * 5)
    x_new, y_new, z_new = scipy.interpolate.splev(u_new, tck)
    spline_nodes = np.column_stack((x_new, y_new, z_new))
    return spline_nodes


class MeshLines():

    def __init__(self,
                 elements,
                 nodes,
                 ellipses_elements=[],
                 ellipses_nodes=[],
                 R_E=0,
                 b=0,
                 n=0,
                 BB=0,
                 AA=0,
                 existBelix=True,
                 NumBelix=0,
                 NumBelix2=0):
        self.elements = elements
        self.nodes = nodes
        self.nodes_class = []
        self.cubics = np.zeros((elements.shape[0],15))
        self.recB=BB/2
        self.recH=AA/2
        self.NumBelix=NumBelix
        self.NumBelix2=NumBelix2
        self.tol =0.19/3 #0.19/3
        self.ellipses_nodes = ellipses_nodes #TODO
        self.ellipses_elements = ellipses_elements
        self.a = R_E/2
        self.b = R_E
        self.n = n
        self.ellipses_nodes_class = []
        self.ellipses_cubics = []
        self.existBelix = existBelix
        return

    def main(self):
        self.iterate_nodes()
        self.classify_element()
        self.merge_nearby_points(self.cubics)
        if self.existBelix:
            self.iterate_nodes_ellipse()
            self.classify_element_ellipse()
        return self.nodes_class, self.cubics, self.ellipses_cubics

    def classify_element(self):
        for i in range(0, self.elements.shape[0]):
            node1_index = int(self.elements[i, 0] - 1)
            node2_index = int(self.elements[i, 1] - 1)
            node1 = self.nodes[node1_index, :]
            node2 = self.nodes[node2_index, :]
            P1, P2, P3, P4 = self.nodes_class[node1_index].add_face(node2 -
                                                                    node1)
            self.cubics[i, 0:3] = P1
            self.cubics[i, 3:6] = P2
            self.cubics[i, 6:9] = P3
            self.cubics[i, 9:12] = P4
            self.cubics[i, 12:15] = node2 - node1

    def iterate_nodes(self, ):
        for i in range(self.nodes.shape[0]):
            self.nodes_class.append(
                Node(i, self.nodes[i, 0], self.nodes[i, 1], self.nodes[i, 2],
                     self.recB, self.recH))

    def merge_nearby_points(self, cubic_list):
        cubics_last3 = cubic_list[:, 12:15]
        cubics_first12 = cubic_list[:, 0:12]
        dx = cubics_last3[:, 0]
        dy = cubics_last3[:, 1]
        dz = cubics_last3[:, 2]
        a = cubics_first12[:, 0:3]
        b = cubics_first12[:, 3:6]
        c = cubics_first12[:, 6:9]
        d = cubics_first12[:, 9:12]
        x = np.stack([a[:, 0] + dx, a[:, 1] + dy, a[:, 2] + dz], axis=1)
        y = np.stack((b[:, 0] + dx, b[:, 1] + dy, b[:, 2] + dz), axis=1)
        z = np.stack((c[:, 0] + dx, c[:, 1] + dy, c[:, 2] + dz), axis=1)
        w = np.stack((d[:, 0] + dx, d[:, 1] + dy, d[:, 2] + dz), axis=1)
        test = np.concatenate((a, b, c, d, x, y, z, w), axis=1)
        test = test.reshape((test.shape[0] * 8, 3))
        kdtree = scipy.spatial.cKDTree(test)
        query_pairs = kdtree.query_ball_point(test, self.tol)
        for i in query_pairs:
            if len(i) > 1:
                test[i] = np.mean(test[i], axis=0)

        test = test.reshape((cubics_first12.shape[0], 24))
        self.cubics = test
        return test

    def classify_element_ellipse(self):
        numHelix=int(self.NumBelix+self.NumBelix2)
        ellipses_per_helix=self.ellipses_elements.shape[0]//(numHelix)
        for j in range(0,numHelix):
            for i in range(int(self.ellipses_elements.shape[0]/numHelix*j),int(self.ellipses_elements.shape[0]/numHelix*(j+1)-4)):
    
                node1_index = int(self.ellipses_elements[i,0] - 1)
                node2_index = int(self.ellipses_elements[i,1] - 1)
                node3_index = int(self.ellipses_elements[i+1,1] - 1) #for 2nd point projection
                
                node1 = self.ellipses_nodes[node1_index,:]
                node2 = self.ellipses_nodes[node2_index,:]
                node3 = self.ellipses_nodes[node3_index,:]
                projected_points = self.ellipses_nodes_class[node1_index].add_face(node2 - node1)
               
                c = projected_points[:,0:3]
                b = projected_points[:,3:6]
                a = projected_points[:,6:9]
                d = projected_points[:,9:12]
                projected_points2 = self.ellipses_nodes_class[node2_index].add_face(node3 - node2)
                z = projected_points2[:,0:3]
                y = projected_points2[:,3:6]
                x = projected_points2[:,6:9]
                w = projected_points2[:,9:12]
                test=np.concatenate((a,b,c,d,x,y,z,w),axis=1)
                #print(test.shape)
                if i == 0:
                    self.ellipses_cubics = test
                else:
                    self.ellipses_cubics = np.vstack((self.ellipses_cubics,test))

    def iterate_nodes_ellipse(self):
        for i in range(self.ellipses_nodes.shape[0]):
            self.ellipses_nodes_class.append(
                Node_ellipse(i, self.ellipses_nodes[i, 0],
                             self.ellipses_nodes[i, 1], self.ellipses_nodes[i,
                                                                            2],
                             self.a, self.b, self.n))

    def merge_nearby_points_ellipse(self, cubic_list):
        test = cubic_list
        test = test.reshape((test.shape[0] * 8, 3))
        kdtree = scipy.spatial.cKDTree(test)
        query_pairs = kdtree.query_ball_point(test, self.tol)
        for i in query_pairs:
            if len(i) > 1:
                test[i] = np.mean(test[i], axis=0)

        test = test.reshape((int(test.shape[0] / 8), 24))
        self.ellipses_cubics = test
        return test


class Node():

    def __init__(self, node_index, x, y, z, recB, recH):
        self.node_index = node_index
        self.x = x
        self.y = y
        self.z = z
        self.dir1 = [None, None, None]
        self.face1 = np.zeros((4, 3))
        self.recB = recB
        self.recH = recH

    def add_face(self, dir_cur):

        self.dir1 = dir_cur
        P1, P2, P3, P4 = self.add_face_helper(dir_cur, 1)
        return P1, P2, P3, P4

    def add_face_helper(self, dir_cur, face_num):

        recB = self.recB
        recH = self.recH
        vx, vy, vz = dir_cur[0], dir_cur[1], dir_cur[2]
        N = np.array([self.x, self.y, self.z])

        v1 = dir_cur
        v1_norm = np.linalg.norm(v1)
        v1 = v1 / v1_norm

        v2 = np.array([-self.x, -self.y, 0])
        v2_norm = np.linalg.norm(v2)
        v2 = v2 / v2_norm

        v3 = np.cross(v1, v2)
        P1 = N - recB * v2 - recH * v3
        P2 = N + recB * v2 - recH * v3
        P3 = N + recB * v2 + recH * v3
        P4 = N - recB * v2 + recH * v3
        self.face1 = np.array([P1, P2, P3, P4])
        return P1, P2, P3, P4


def ellipsoid_cs(a, b, n):

    def semi_ellipse(x, a, b):
        return np.sqrt(1 - (x**2 / a**2)) * b

    x_values = np.linspace(-a, a, n + 1)
    y_left_values = semi_ellipse(x_values[:-1], a, b)
    y_right_values = semi_ellipse(x_values[1:], a, b)

    rectangle_coordinates = np.zeros((n, 12))

    for i in range(n):
        x_left = x_values[i]
        x_right = x_values[i + 1]
        y_left_top = y_left_values[i]
        y_right_top = y_right_values[i]
        y_bottom = 0

        rectangle_coordinates[i] = [
            x_left, y_bottom, 0,
            x_right, y_bottom, 0,
            x_right, y_right_top, 0,
            x_left, y_left_top, 0
        ]

    return rectangle_coordinates.reshape(rectangle_coordinates.shape[0] * 4, 3)


def rotation_matrix_from_vectors(vecX, vecY):
    vecZ = np.cross(vecX, vecY)
    R = np.column_stack((vecX, vecY, vecZ))
    return R


class Node_ellipse():

    def __init__(self, node_index, x, y, z, a, b, n):
        self.node_index = node_index
        self.x = x
        self.y = y
        self.z = z
        self.dir1 = [None, None, None]
        self.face1 = np.zeros((4, 3))
        self.ellipse_origin = ellipsoid_cs(a, b, n)

    def add_face(self, dir_cur):

        self.dir1 = dir_cur

        projected_points = self.add_face_helper(dir_cur, 1)
        return projected_points

    def add_face_helper(self, dir_cur, face_num):
        N = np.array([self.x, self.y, self.z])
        v1 = dir_cur
        v1_norm = np.linalg.norm(v1)
        v1 = v1 / v1_norm

        v2 = np.array([self.x, self.y, 0])
        v2_norm = np.linalg.norm(v2)
        v2 = v2 / v2_norm

        v3 = np.cross(v1, v2)
        rotation_matrix = rotation_matrix_from_vectors(v3, v2)
        projected_points = np.dot(self.ellipse_origin, rotation_matrix.T) + N

        self.face1 = projected_points.reshape(
            int(projected_points.shape[0] / 4), 12)
        return self.face1


def convert_to_stl_binary(cubic_list, filenameOut):

    def tri(out, a, b, c):
        out.write(struct.pack('12f', 0, 0, 0, *a, *b, *c))
        out.write(bytes(2))
        return 1

    def sq(out, a, b, c, d):
        return tri(out, a, b, c) + tri(out, a, c, d)

    with open(filenameOut, "wb") as out:
        j = 0
        out.write(bytes(80 + 4))
        nt = 0
        for i in range(cubic_list.shape[0]):

            a = cubic_list[i, 0:3]
            b = cubic_list[i, 3:6]
            c = cubic_list[i, 6:9]
            d = cubic_list[i, 9:12]

            w = cubic_list[i, 12:15]
            x = cubic_list[i, 15:18]
            y = cubic_list[i, 18:21]
            z = cubic_list[i, 21:24]

            nt += tri(out, w, z, a)
            nt += tri(out, a, z, d)
            nt += tri(out, x, w, b)
            nt += tri(out, b, w, a)
            nt += tri(out, y, x, c)
            nt += tri(out, c, x, b)
            nt += tri(out, z, y, d)
            nt += tri(out, d, y, c)
            nt += tri(out, x, y, w)
            nt += tri(out, w, y, z)
            nt += tri(out, c, b, d)
            nt += tri(out, d, b, a)
            j += 8
        out.seek(80)
        out.write(struct.pack('I', nt))


H = 100
R = 11
D_SHELIX = 1.3
n = 20

NV = config.NV
NC = config.NC
AA = config.AA
BB = config.BB
LOOP_NO = config.LOOP_NO
R_E = config.R_E 
NumBelix = config.NumBelix
NumBelix2 = config.NumBelix2
geom = GeomParam()
nodes, elements, bnodes, belements = geom.main(H, R, NV, NC, LOOP_NO, NumBelix,
                                               NumBelix2, D_SHELIX)
if NumBelix + NumBelix2 == 0:
    existBelix = False
else:
    existBelix = True
meshed = MeshLines(elements, nodes, belements, bnodes, R_E, 2*R_E, n, BB, AA,
                   existBelix,NumBelix,NumBelix2)
node_class_list, cubic_list, bcubics = meshed.main()
if existBelix:
    cubics_all = np.vstack((cubic_list, bcubics))
else:
    cubics_all = cubic_list
convert_to_stl_binary(cubics_all, "ver.stl")
