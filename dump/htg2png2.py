import sys
import paraview
import re
from paraview.simple import *

Verbose = 1
stl_path = "trans.stl"
htg_path = sys.argv[1]
png_path = re.sub("\.htg$", "", htg_path) + ".png"
render = CreateView("RenderView")
render.ViewSize = [800, 800]
render.AxesGrid = "GridAxes3DActor"
render.OrientationAxesVisibility = 0
render.StereoType = "Crystal Eyes"
render.CameraPosition = [0, -5, 1]
render.CameraFocalPoint = [0, 0, 1]
render.CameraViewUp = [-1, 0, 0]
render.CameraFocalDisk = 1
render.CameraParallelScale = 3
render.CameraParallelProjection = 1
render.BackEnd = "OSPRay raycaster"
render.UseColorPaletteForBackground = 0
render.Background = [1, 1, 1]
layout = CreateLayout()
layout.AssignView(0, render)
stl = STLReader(FileNames=[stl_path])
htg = HyperTreeGridReader(FileNames=[htg_path])
if Verbose:
    sys.stderr.write("htg2png.py: open STL: %s\n" % stl_path)
    sys.stderr.write("htg2png.py: open HTG: %s\n" % htg_path)
hyperTreeGridToDualGrid1 = HyperTreeGridToDualGrid(Input=htg)
if Verbose:
    sys.stderr.write("htg2png.py: HyperTreeGridToDualGrid\n")
stl = Show(stl, render, "GeometryRepresentation")
stl.Representation = "Surface"
htg = Show(hyperTreeGridToDualGrid1, render, "UnstructuredGridRepresentation")
omegaLUT = GetColorTransferFunction("omega")
omegaLUT.RGBPoints = [
    -1.0,
    0.1,
    0,
    0,
    0.0,
    0.1,
    0.1,
    0.1,
    1.0,
    0,
    0,
    0.1,
]
omegaPWF = GetOpacityTransferFunction("omega")
omegaPWF.Points = [
    -10.0,
    0.4,
    0.5,
    0.0,
    -5,
    0.0,
    0.5,
    0.0,
    5,
    0.0,
    0.5,
    0.0,
    10.0,
    0.4,
    0.5,
    0.0,
]
htg.Representation = "Volume"
htg.ColorArrayName = ["POINTS", "omega"]
htg.LookupTable = omegaLUT
htg.ScalarOpacityFunction = omegaPWF
SaveScreenshot(png_path)
