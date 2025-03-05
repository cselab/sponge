import math
import numpy as np
import os
import re
import sys
import xml.etree.ElementTree as ET
import multiprocessing
import copy


def transform(Data):
    t = 100 / 47, -10 / 47, 0
    r = 0, 0, -90
    s = 2 / 47, 2 / 47, 2 / 47
    p = math.radians(r[2])
    T = [[math.cos(p), -math.sin(p)], [math.sin(p), math.cos(p)]]
    # rotate
    Data["x"], Data["y"] = zip(*[(T[0][0] * x + T[0][1] * y,
                                  T[1][0] * x + T[1][1] * y)
                                 for x, y in zip(Data["x"], Data["y"])])
    # scale and translate
    Data["x"] = [s[0] * r + t[0] for r in Data["x"]]
    Data["y"] = [s[1] * r + t[1] for r in Data["y"]]
    Data["z"] = [s[2] * r + t[2] for r in Data["z"]]

    Data["Vx"], Data["Vy"] = Data["Vy"], Data["Vx"]


def process0(path):
    return process(Data, path)


def process(Data, path):
    Data = copy.deepcopy(Data)
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

    suffix = os.path.basename(path).split(".")[-2:]
    output_path = os.path.join(dirname, ".".join(["project", *suffix]))
    sys.stderr.write("project.py: %ld: %s\n" % (os.getpid(), output_path))
    for i, (x, y) in enumerate(zip(Data["x"], Data["y"])):
        if i % 100 == 0:
            sys.stderr.write("project.py: %ld: warning: %ld/%ld\n" %
                             (os.getpid(), i, len(Data["x"])))
        for h, u0 in zip(hexa, u):
            xl, yl, zl = h[0]
            xh, yh, zh = h[6]
            if xl <= x <= xh and yl <= y <= yh:
                Data["Vx"][i], Data["Vy"][i], Data["Vz"][i] = u0
                break
        else:
            sys.stderr.write("project: warning: no cell for %g, %g\n" % (x, y))
            Data["Vx"][i], Data["Vy"][i], Data["Vz"][i] = 0, 0, 0
    with open(output_path, "w") as f:
        f.write("""\
<Xdmf
    Version="2">
  <Domain>
    <Grid>
      <Topology
          TopologyType="3DSMesh"
          Dimensions="%ld %ld %ld"/>
      <Geometry
          GeometryType="XYZ">
        <DataItem
            Dimensions="%ld 3">
""" % (nz, ny, nx, nx * ny * nz))
        for x, y, z in zip(Data["x"], Data["y"], Data["z"]):
            f.write("""\
              %.16e %.16e %.16e
""" % (x, y, z))
        f.write("""\
        </DataItem>
      </Geometry>
""")
        f.write("""\
      <Attribute
          Name="u"
          AttributeType="Vector">
        <DataItem
            Dimensions="1 %ld %ld 3">
""" % (ny, nx))
        for x, y, z in zip(Data["Vx"], Data["Vy"], Data["Vz"]):
            f.write("""\
            %.16e %.16e %.16e
""" % (x, y, z))
        f.write("""\
        </DataItem>
      </Attribute>
    </Grid>
  </Domain>
</Xdmf>
""")


sys.argv.pop(0)
path = sys.argv.pop(0)
with open(path, "r") as f:
    title = re.sub('TITLE[ \t]*=[\ t]*"', '', f.readline())
    title = re.sub('"[ \t]*\n', '', title)
    variables = re.sub('VARIABLES[ \t]=[ \t]*', '', f.readline())
    variables = re.sub('[ \t]*\n', '', variables)
    variables = re.split(",[ \t]*", variables)
    variables = [re.sub('(^")|("$)', "", x) for x in variables]
    zone = re.sub('ZONE[ \t]*', '', f.readline())
    zone = re.sub('[ \t]*\n', '', zone)
    zone = re.split(",[ \t]*", zone)
    zone = dict(x.split("=") for x in zone)
    nx = int(zone["I"])
    ny = int(zone["J"])
    nz = int(zone["K"])
    data = [[] for x in variables]
    for line in f:
        for d, x in zip(data, line.split()):
            x = float(x)
            d.append(x)
Data = dict(zip(variables, data))
transform(Data)

with multiprocessing.Pool() as p:
    p.map(process0, sys.argv)
