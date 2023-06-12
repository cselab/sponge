import meshio
mesh = meshio.read("ridges.stl")
x, y, z = zip(*mesh.points)

xlo = min(x)
xhi = max(x)
ylo = min(y)
yhi = max(y)
zlo = min(z)
zhi = max(z)

xc = (xlo + xhi) / 2
yc = (ylo + yhi) / 2
zc = (zlo + zhi) / 2

zd = zhi - zlo
s = 0.8 / zd

x = [ (x - xc) * s for x in x]
y = [ (y - yc) * s for y in y]
z = [ (z - zc) * s for z in z]

diameter = xhi - xlo
mesh.points[:, 0] = x
mesh.points[:, 1] = y
mesh.points[:, 2] = z
mesh.write("out.stl", binary=True)
print(diameter, diameter * s, 40 * s)

