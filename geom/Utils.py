# -*- coding: utf-8 -*-
"""
Created on Mon Aug  7 12:17:14 2023

@author: tanjikede
"""

def convert_to_stl(cubic_list, filenameOut):
    def tri(out, a, b, c):
            out.write("""\
    facet normal 0 0 0
        outer loop
            vertex %g %g %g
            vertex %g %g %g
            vertex %g %g %g
        endloop
    endfacet
    """ % (*a, *b, *c))
    
    def sq(out, a, b, c, d):
        tri(out, c, b, d)
        tri(out, d, b, a)
    
    with open(filenameOut, "w") as out:
        j = 0
        out.write("solid elements\n")
        count = 0
        for i in range(cubic_list.shape[0]):
    
            a = cubic_list[i,0:3]
            b = cubic_list[i,3:6]
            c = cubic_list[i,6:9]
            d = cubic_list[i,9:12]
    
            w = cubic_list[i,12:15]
            x = cubic_list[i,15:18]
            y = cubic_list[i,18:21]
            z = cubic_list[i,21:24]
            

            #
            tri(out,w,z,a)
            tri(out,a,z,d)
            tri(out,x,w,b)
            tri(out,b,w,a)
            tri(out,y,x,c)
            tri(out,c,x,b)
            tri(out,z,y,d)
            tri(out,d,y,c)
            tri(out,x,y,w)
            tri(out,w,y,z)
            tri(out,c,b,d)
            tri(out,d,b,a)
            
            # tri(out,x,w,a)
            # tri(out,a,w,d)
            # tri(out,y,x,b)
            # tri(out,b,x,a)
            # tri(out,z,y,c)
            # tri(out,c,y,b)
            # tri(out,w,z,d)
            # tri(out,d,z,c)
            # tri(out,y,z,x)
            # tri(out,x,z,w)
            # tri(out,c,b,d)
            # tri(out,d,b,a)
            
            #
            j += 8
            
            count+=1
    
        out.write("endsolid elements\n")
import struct
def convert_to_stl_binary(cubic_list, filenameOut):
        
    def tri(out, a, b, c):
        out.write(struct.pack('12f', 0, 0, 0, *a, *b, *c))
        out.write(bytes(2))
        return 1
    def sq(out, a, b, c, d):
        return tri(out, a, b, c) + tri(out, a, c, d)
    with open(filenameOut, "wb") as out:
        j = 0
        out.write(bytes(80  + 4))
        nt = 0
        for i in range(cubic_list.shape[0]):
            
            a = cubic_list[i,0:3]
            b = cubic_list[i,3:6]
            c = cubic_list[i,6:9]
            d = cubic_list[i,9:12]
    
            w = cubic_list[i,12:15]
            x = cubic_list[i,15:18]
            y = cubic_list[i,18:21]
            z = cubic_list[i,21:24]
            
            nt += tri(out,w,z,a)
            nt += tri(out,a,z,d)
            nt += tri(out,x,w,b)
            nt += tri(out,b,w,a)
            nt += tri(out,y,x,c)
            nt += tri(out,c,x,b)
            nt += tri(out,z,y,d)
            nt += tri(out,d,y,c)
            nt += tri(out,x,y,w)
            nt += tri(out,w,y,z)
            nt += tri(out,c,b,d)
            nt += tri(out,d,b,a)
            # nt += sq(out, a, b, c, d)
            # nt += sq(out, x, y, z, w)
            # nt += sq(out, a, b, y, x)
            # nt += sq(out, c, d, w, z)
            # nt += sq(out, a, d, w, x)
            # nt += sq(out, b, c, z, y)
            j += 8
        out.seek(80)
        out.write(struct.pack('I', nt))