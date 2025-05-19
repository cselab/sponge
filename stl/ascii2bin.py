import meshio
import sys

path = sys.argv[1]
mesh = meshio.read(path)
mesh.write(sys.argv[2], binary=True)
