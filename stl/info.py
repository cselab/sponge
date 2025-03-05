#!/usr/bin/env python3
import os
import sys
import numpy as np
import xml.etree.ElementTree as ET
import re

path = sys.argv[1]
dirname = os.path.dirname(path)
root = ET.parse(path)
time = float(root.find("Domain/Grid/Time").get("Value"))
xyz_path = root.find("Domain/Grid/Geometry/DataItem").text
xyz_path = re.sub("^[\n\t ]*", "", xyz_path)
xyz_path = re.sub("[\n\t ]*$", "", xyz_path)
xyz_path = os.path.join(dirname, xyz_path)

nhex = root.find("Domain/Grid/Topology").get("Dimensions")
nhex = int(nhex)
hexa = np.fromfile(xyz_path, np.dtype("float32"))
hexa = np.reshape(hexa, (nhex, 8, 3))

attr_path, = (x for x in root.findall("Domain/Grid/Attribute")
              if x.get("Name") == "u")
attr_dims, attr_path = attr_path.findall("DataItem/DataItem")
attr_path = re.sub("^[\n\t ]*", "", attr_path.text)
attr_path = re.sub("[\n\t ]*$", "", attr_path)
attr_path = os.path.join(dirname, attr_path)
attr = np.fromfile(attr_path, np.dtype("float32"))
attr = np.reshape(attr, (nhex, -1))
ilo, jlo, istride, jstride, icount, jcount, *rest = map(
    int, attr_dims.text.split())
u = attr[:, jlo:jlo + jcount]
print(np.min(u, 0))
print(np.max(u, 0))
