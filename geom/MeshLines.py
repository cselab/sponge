# -*- coding: utf-8 -*-
"""
Created on Thu Jul 27 15:40:10 2023

@author: tanjikede
"""
import numpy as np
import matplotlib.pyplot as plt
from scipy.spatial import cKDTree

class MeshLines():
    def __init__(self, elements, nodes, ellipses_elements=[], ellipses_nodes=[],R_E=0,b=0,n=0,BB=0,AA=0,existBelix=True,NumBelix=0,NumBelix2=0):
        ## ELEMENTS: LIST of 2 Points lines, from 1 to 8800 ##
        self.elements = elements
        ## NODES: LIST OF 3 
        self.nodes = nodes
        ## NODES: LIST of 3 point coords, from 1 to 8800 ##
        self.nodes_class = []
        #NODES: LIST of P1,P2,P3,P4,delX,delY,delZ
        self.cubics = np.zeros((elements.shape[0],15))
        self.recB=BB/2
        self.recH=AA/2
        self.NumBelix=NumBelix
        self.NumBelix2=NumBelix2
        # tolerance for merging
        self.tol =0.19/3 #0.19/3
        #### ellipses
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
        # loop over elements and return nodes -> 4 amount of 3 coords, 12 data points
        self.iterate_nodes()
        self.classify_element()
        self.merge_nearby_points(self.cubics) # this changes self.cubics
        if self.existBelix:
            self.iterate_nodes_ellipse()
            self.classify_element_ellipse()
            #self.merge_nearby_points_ellipse(self.ellipses_cubics)
        return self.nodes_class,self.cubics,self.ellipses_cubics
    
    def classify_element(self):
        for i in range(0,self.elements.shape[0]):
            node1_index = int(self.elements[i,0] - 1)
            node2_index = int(self.elements[i,1] - 1)
            node1 = self.nodes[node1_index,:]
            node2 = self.nodes[node2_index,:]
            P1,P2,P3,P4 = self.nodes_class[node1_index].add_face(node2 - node1)
            #### CONVENTION: CUBES COMPRIESED OF P1,P2,P3,P4, del = N2-N1
            self.cubics[i,0:3] = P1
            self.cubics[i,3:6] = P2
            self.cubics[i,6:9] = P3
            self.cubics[i,9:12] = P4
            self.cubics[i,12:15] = node2-node1
    def iterate_nodes(self,):
        for i in range(self.nodes.shape[0]):
            self.nodes_class.append(Node(i,self.nodes[i,0],self.nodes[i,1],self.nodes[i,2],self.recB,self.recH))
    
    
    def merge_nearby_points(self,cubic_list):
        cubics_last3 = cubic_list[:,12:15]
        cubics_first12 = cubic_list[:,0:12]
        dx = cubics_last3[:,0]
        dy = cubics_last3[:,1]
        dz = cubics_last3[:,2]
        a = cubics_first12[:,0:3]
        b = cubics_first12[:,3:6]
        c = cubics_first12[:,6:9]
        d = cubics_first12[:,9:12]
        x = np.stack([a[:,0] + dx, a[:,1] + dy, a[:,2] + dz],axis=1)
        y = np.stack((b[:,0] + dx, b[:,1] + dy, b[:,2] + dz),axis=1)
        z = np.stack((c[:,0] + dx, c[:,1] + dy, c[:,2] + dz),axis=1)
        w = np.stack((d[:,0] + dx, d[:,1] + dy, d[:,2] + dz),axis=1)
        test=np.concatenate((a,b,c,d,x,y,z,w),axis=1)
        test = test.reshape((test.shape[0]*8,3))

        #
        # Create a cKDTree and add the coordinates
        
        kdtree = cKDTree(test)

        # Find pairs of points within the tolerance value
        query_pairs = kdtree.query_ball_point(test,self.tol)

        # Merge points within the tolerance value
        for i in query_pairs:
            if len(i) > 1:
                test[i] = np.mean(test[i], axis=0) 
        
        test = test.reshape((cubics_first12.shape[0],24))
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
               
                #### CONVENTION: CUBES COMPRIESED OF P1,P2,P3,P4, del = N2-N1
                c = projected_points[:,0:3]
                b = projected_points[:,3:6]
                a = projected_points[:,6:9]
                d = projected_points[:,9:12]
                
                projected_points2 = self.ellipses_nodes_class[node2_index].add_face(node3 - node2)
                z = projected_points2[:,0:3]
                y = projected_points2[:,3:6]
                x = projected_points2[:,6:9]
                w = projected_points2[:,9:12]
                
                #delt = node2-node1
                # dx = delt[0]
                # dy = delt[1]
                # dz = delt[2]
                # x = np.stack([a[:,0] + dx, a[:,1] + dy, a[:,2] + dz],axis=1)
                # y = np.stack((b[:,0] + dx, b[:,1] + dy, b[:,2] + dz),axis=1)
                # z = np.stack((c[:,0] + dx, c[:,1] + dy, c[:,2] + dz),axis=1)
                # w = np.stack((d[:,0] + dx, d[:,1] + dy, d[:,2] + dz),axis=1)
                test=np.concatenate((a,b,c,d,x,y,z,w),axis=1)
                #print(test.shape)
                if i == 0:
                    self.ellipses_cubics = test
                else:
                    self.ellipses_cubics = np.vstack((self.ellipses_cubics,test))
            #self.ellipses_cubics = np.array(self.ellipses_cubics)

        
        
    def iterate_nodes_ellipse(self):
        for i in range(self.ellipses_nodes.shape[0]):
            self.ellipses_nodes_class.append(Node_ellipse(i,self.ellipses_nodes[i,0],self.ellipses_nodes[i,1],self.ellipses_nodes[i,2], self.a, self.b, self.n))
    
    
    def merge_nearby_points_ellipse(self,cubic_list):
        test = cubic_list
        test = test.reshape((test.shape[0]*8,3))

        #
        # Create a cKDTree and add the coordinates
        
        kdtree = cKDTree(test)

        # Find pairs of points within the tolerance value
        query_pairs = kdtree.query_ball_point(test,self.tol)

        # Merge points within the tolerance value
        for i in query_pairs:
            if len(i) > 1:
                test[i] = np.mean(test[i], axis=0) 
        
        test = test.reshape((int(test.shape[0]/8),24))
        self.ellipses_cubics = test
        return test
    
class Node():
    def __init__(self, node_index, x, y, z, recB, recH):
        self.node_index = node_index
        self.x = x
        self.y = y
        self.z = z
        self.dir1 = [None,None,None]
        self.face1 = np.zeros((4,3))
        self.recB=recB
        self.recH=recH
        
    def add_face(self, dir_cur):
        
        self.dir1 = dir_cur
        P1,P2,P3,P4 = self.add_face_helper(dir_cur,1)
        return P1,P2,P3,P4
    
    def add_face_helper(self, dir_cur, face_num):

        recB = self.recB
        recH = self.recH
        vx,vy,vz = dir_cur[0],dir_cur[1],dir_cur[2]
        N = np.array([self.x,self.y,self.z])

        v1 = dir_cur
        v1_norm = np.linalg.norm(v1)
        v1 = v1/v1_norm
        
        v2 = np.array([-self.x,-self.y,0])
        v2_norm = np.linalg.norm(v2)
        v2 = v2/v2_norm
        
        v3 = np.cross(v1,v2)
        
        #now v2 and v3 are w1 w2
        P1 = N - recB * v2 - recH * v3
        P2 = N + recB * v2 - recH * v3
        P3 = N + recB * v2 + recH * v3
        P4 = N - recB * v2 + recH * v3
        self.face1 = np.array([P1,P2,P3,P4])
        return P1,P2,P3,P4
    

    
    def NodePlotter(self):
        N = np.array([self.x,self.y,self.z])
        P1,P2,P3,P4 = self.face1[0,:],self.face1[1,:],self.face1[2,:],self.face1[3,:]
        vx,vy,vz = self.dir1[0],self.dir1[1],self.dir1[2]
       
        fig = plt.figure()
        ax = fig.add_subplot(111, projection='3d')
        ax.scatter(N[0], N[1], N[2], c='r', marker='o', label='Node N')
        ax.scatter(0, 0, N[2], c='r', marker='x', label='center')
        #ax.quiver(N[0], N[1], N[2], vx, vy, vz, color='b', label='Vector v')
        #ax.quiver(N[0], N[1], N[2], 0, 0, N[2], color='b', label='Vector d')
        ax.scatter(P1[0], P1[1], P1[2], c='g', marker='o', label='P1')
        ax.scatter(P2[0], P2[1], P2[2], c='b', marker='o', label='P2')
        ax.scatter(P3[0], P3[1], P3[2], c='y', marker='o', label='P3')
        ax.scatter(P4[0], P4[1], P4[2], c='m', marker='o', label='P4')
        
        # Set labels and legend
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        ax.set_zlabel('Z')
        ax.legend()
        
        # Show the plot
        plt.show()
        dist_P1_P2 = np.linalg.norm(P2 - P1)
        dist_P2_P4 = np.linalg.norm(P2 - P4)
        if abs(dist_P1_P2-dist_P2_P4) > 1e-4:
            print("error")


def ellipsoid_cs(a, b, n):  # n determines the mesh size
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


def rotation_matrix_from_vectors(vecX,vecY): #TODO: vec1, vec2 分别代表global xy轴，把他map到local xy轴
    vecZ = np.cross(vecX, vecY)
    R = np.column_stack((vecX, vecY,vecZ))
    return R

class Node_ellipse():
    def __init__(self, node_index, x, y, z, a, b, n):
        self.node_index = node_index
        self.x = x
        self.y = y
        self.z = z
        self.dir1 = [None,None,None]
        self.face1 = np.zeros((4,3))
        self.ellipse_origin = ellipsoid_cs(a,b,n)
        
    def add_face(self, dir_cur):
        
        self.dir1 = dir_cur
        
        projected_points = self.add_face_helper(dir_cur,1)
        return projected_points
    
    def add_face_helper(self, dir_cur, face_num):
        N = np.array([self.x,self.y,self.z])
        v1 = dir_cur
        v1_norm = np.linalg.norm(v1)
        v1 = v1/v1_norm
        
        v2 = np.array([self.x,self.y,0])
        v2_norm = np.linalg.norm(v2)
        v2 = v2/v2_norm
        
        v3 = np.cross(v1,v2)

        #v3 = np.dot(rotation_matrix, v3)
        rotation_matrix = rotation_matrix_from_vectors(v3,v2)
        projected_points = np.dot(self.ellipse_origin,rotation_matrix.T) + N

        
        self.face1 = projected_points.reshape(int(projected_points.shape[0]/4),12) #shape (n,12)
        return self.face1
