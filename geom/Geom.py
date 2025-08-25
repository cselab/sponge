# -*- coding: utf-8 -*-
"""
Created on Mon Aug  7 12:28:57 2023

@author: tanjikede
"""
import numpy as np
import matplotlib.pyplot as plt
from scipy.spatial import cKDTree
from scipy.interpolate import splprep, splev

class GeomParam():
    
    def __init__(self):
        #### ELEMENTS AND NODES
        self.elements = []
        self.nodes = [] # index and 3 coords
        #### BIGHELIX ELEMENTS AND NODES
        self.belements = []
        self.bnodes = []
        #### VERTICAL BEAMS
        self.R=0 # base radius cylinder
        self.H=0 # height of vertical beams
        self.NV=0 # number of vertical beams?
        self.DELTA_TH = 0 # angle addition for creating vertical beams
        #### CIRCUMFERENTIAL
        self.NC=0 # number of circumferential circles
        self.DELTA_H = self.H/(self.NC-1) #delta Height for each circumferential circle
        #### SHELIX
        self.D_SHELIX=0
        self.TH_SHELIX = 0
        #### BHELIX
        self.LOOP_NO = 0
        self.PITCH=0
        
        # 
        
        self.COUNT = 1
        # 
        
        # NOBUCK = 10
        return
    
    def main(self,H,R,NV,NC,LOOP_NO,NumBhelix,NumBhelix2,D_SHELIX):
        self.ChangeParams(H,R,NV,NC,LOOP_NO,NumBhelix,NumBhelix2,D_SHELIX)
        self.VerticalBeams()
        self.Circumferential()
        self.SmallHelix()
        self.SmallHelix2()
        self.BigHelix()
        self.BigHelix2()
        return np.array(self.nodes),np.array(self.elements), np.array(self.bnodes), np.array(self.belements)
    
    def ChangeParams(self,H,R,NV,NC,LOOP_NO, NumBhelix,NumBhelix2,D_SHELIX):
        # erase the attributes and rebuild elements and nodes list
        #### ELEMENTS AND NODES
        self.elements = []
        self.nodes = [] # index and 3 coords
        self.belements = []
        self.bnodes = []
        self.COUNT_LINE = 1
        self.bCOUNT_LINE = 1
        
        #### VERTICAL BEAMS
        self.R=R#12.6051E-3 # base radius cylinder
        self.H=H #60E-3 # height of vertical beams
        self.NV=NV#40 # number of vertical beams?
        self.DELTA_TH = 2*np.pi/self.NV # angle addition for creating vertical beams
        #### CIRCUMFERENTIAL BEAMS
        self.NC=NC#37
        self.DELTA_H = self.H/(self.NC-1)
        #### SHELIX
        self.D_SHELIX=D_SHELIX # spacing between two small helixs
        self.TH_SHELIX = self.D_SHELIX/self.R
        #### BHELIX
        self.LOOP_NO = LOOP_NO
        self.PITCH=self.H/self.LOOP_NO
        self.NumBhelix = NumBhelix
        self.NumBhelix2 = NumBhelix2
        return
    
    def VerticalBeams(self):
        NV = self.NV
        H = self.H
        DELTA_TH = self.DELTA_TH
        R = self.R
        COUNT = self.COUNT
  
        TH=0
        for II in range(1,NV+1): 
             #key points created at locations at height 0 #mapdl.k(COUNT,R*np.cos(TH),R*np.sin(TH),0)
            self.nodes.append([R*np.cos(TH),R*np.sin(TH),0])
            TH = TH + DELTA_TH 
            COUNT = COUNT + 1
            
        TH=0
        for II in range(1,NV+1):
            #mapdl.k(COUNT,R*np.cos(TH),R*np.sin(TH),H) 
            self.nodes.append([R*np.cos(TH),R*np.sin(TH),H])
            TH = TH + DELTA_TH
            COUNT = COUNT + 1
        #assert COUNT == NV*2
        
        COUNT_LINE=1
        for II in range(1,NV+1):
            #mapdl.l(COUNT_LINE,COUNT_LINE+NV)
            self.elements.append([COUNT_LINE, COUNT_LINE+NV])
            COUNT_LINE = COUNT_LINE + 1
        COUNT_LINE = COUNT_LINE + NV
        self.COUNT_LINE = COUNT_LINE

    def Circumferential(self):
        NC = self.NC
        R = self.R
        COUNT_LINE = self.COUNT_LINE
        DELTA_H = self.DELTA_H
        HH=0
        for II in range(1,NC+1):
            #mapdl.k(COUNT,0,0,HH)
            for theta in range(360):
                self.nodes.append([R*np.cos(np.radians(theta)), R*np.sin(np.radians(theta)), HH])
                if theta != 359:
                    self.elements.append([COUNT_LINE, COUNT_LINE + 1])
                else:
                    self.elements.append([COUNT_LINE, COUNT_LINE - 359])
                COUNT_LINE += 1
            HH = HH + DELTA_H
        self.COUNT_LINE = COUNT_LINE
    
    def SmallHelix(self):
        def bspline(points): # helper function; points (n,3)
            # Fit a B-spline curve through the points
            points = np.array(points)
            tck, u = splprep(points.T, s=0)
            # Evaluate the B-spline curve
            u_new = np.linspace(u.min(), u.max(), points.shape[0] * 5) #determines how many data points
            x_new, y_new, z_new = splev(u_new, tck) 
            spline_nodes = np.column_stack((x_new, y_new, z_new))
            return spline_nodes
        # fetch attributes
        COUNT_LINE = self.COUNT_LINE
        TH_SHELIX = self.TH_SHELIX
        NV = self.NV
        NC = self.NC
        R = self.R
        DELTA_TH = self.DELTA_TH
        DELTA_H = self.DELTA_H
        # end fetch
        
        TH0 = TH_SHELIX/2
        TH1 = -TH_SHELIX/2
        
        for JJ in range(1,int(NV/2)+2):
            unsplined_nodes = []
            HH=0
            for II in range(1,NC+1):
                unsplined_nodes.append([R*np.cos(TH0),R*np.sin(TH0),HH])
                TH0 = TH0 + DELTA_TH
                HH = HH + DELTA_H
        
            splined_nodes = bspline(unsplined_nodes)
            # add elements from splined_nodes
            self.elements.extend([[i+COUNT_LINE, i+1+COUNT_LINE] for i in range(len(splined_nodes) - 1)])
            COUNT_LINE += len(splined_nodes)
            self.nodes.extend(splined_nodes.tolist())
            
            
            unsplined_nodes = []
            HH=0
            for II in range(1,NC+1):
                unsplined_nodes.append([R*np.cos(TH1),R*np.sin(TH1),HH])
                TH1 = TH1 + DELTA_TH
                HH = HH + DELTA_H
            
            splined_nodes = bspline(unsplined_nodes)
            self.elements.extend([[i+COUNT_LINE, i+1+COUNT_LINE] for i in range(len(splined_nodes) - 1)])
            COUNT_LINE += len(splined_nodes)
            self.nodes.extend(splined_nodes.tolist())
            TH0 = TH_SHELIX/2+(2*DELTA_TH)*JJ #盘一柱空一柱
            TH1 = -TH_SHELIX/2+(2*DELTA_TH)*JJ
            
        self.COUNT_LINE = COUNT_LINE
        
    def SmallHelix2(self):
        # fetch attributes
        COUNT_LINE = self.COUNT_LINE
        TH_SHELIX = self.TH_SHELIX
        NV = self.NV
        NC = self.NC
        R = self.R
        DELTA_TH = self.DELTA_TH
        DELTA_H = self.DELTA_H
        # end fetch
        
        TH0 = TH_SHELIX/2
        TH1 = -TH_SHELIX/2
        
        for JJ in range(1,int(NV/2)+2):
            unsplined_nodes = []
            HH=0
            for II in range(1,NC+1):
                unsplined_nodes.append([R*np.cos(TH0),R*np.sin(TH0),HH])
                TH0 = TH0 - DELTA_TH
                HH = HH + DELTA_H
        
            splined_nodes = bspline(unsplined_nodes)
            # add elements from splined_nodes
            self.elements.extend([[i+COUNT_LINE, i+1+COUNT_LINE] for i in range(len(splined_nodes) - 1)])
            COUNT_LINE += len(splined_nodes)
            self.nodes.extend(splined_nodes.tolist())
            
            
            unsplined_nodes = []
            HH=0
            for II in range(1,NC+1):
                unsplined_nodes.append([R*np.cos(TH1),R*np.sin(TH1),HH])
                TH1 = TH1 - DELTA_TH
                HH = HH + DELTA_H
            
            splined_nodes = bspline(unsplined_nodes)
            self.elements.extend([[i+COUNT_LINE, i+1+COUNT_LINE] for i in range(len(splined_nodes) - 1)])
            COUNT_LINE += len(splined_nodes)
            self.nodes.extend(splined_nodes.tolist())
            TH0 = TH_SHELIX/2+(2*DELTA_TH)*JJ #盘一柱空一柱
            TH1 = -TH_SHELIX/2+(2*DELTA_TH)*JJ
            
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
        thetainit = (2*np.pi)/(NumBhelix)
        for i in range(NumBhelix):
            HH = 0
            TH0 = thetainit * i 
            DELTA_H_BIGHELIX = PITCH/NV
            unsplined_nodes = []
            for II in range(1,(NV)*LOOP_NO+2):
                unsplined_nodes.append([R*np.cos(TH0),R*np.sin(TH0),HH])
                TH0 = TH0 - DELTA_TH
                HH = HH + DELTA_H_BIGHELIX
            splined_nodes = bspline(unsplined_nodes)
            self.belements.extend([[i+bCOUNT_LINE, i+1+bCOUNT_LINE] for i in range(len(splined_nodes) - 1)])
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
        thetainit = (2*np.pi)/(NumBhelix)
        for i in range(NumBhelix):
            HH = 0
            TH0 = thetainit * i 
            DELTA_H_BIGHELIX = PITCH/NV
            unsplined_nodes = []
            for II in range(1,(NV)*LOOP_NO+2):
                unsplined_nodes.append([R*np.cos(TH0),R*np.sin(TH0),HH])
                TH0 = TH0 + DELTA_TH
                HH = HH + DELTA_H_BIGHELIX
            splined_nodes = bspline(unsplined_nodes)
            self.belements.extend([[i+bCOUNT_LINE, i+1+bCOUNT_LINE] for i in range(len(splined_nodes) - 1)])
            bCOUNT_LINE += len(splined_nodes)
            self.bnodes.extend(splined_nodes.tolist())
        self.bCOUNT_LINE = bCOUNT_LINE
            
def bspline(points): # helper function; points (n,3)
    # Fit a B-spline curve through the points
    points = np.array(points)
    tck, u = splprep(points.T, s=0)
    # Evaluate the B-spline curve
    u_new = np.linspace(u.min(), u.max(), points.shape[0] * 5) #determines how many data points
    x_new, y_new, z_new = splev(u_new, tck) 
    spline_nodes = np.column_stack((x_new, y_new, z_new))
    return spline_nodes
