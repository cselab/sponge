import sys
import paraview
import re
from paraview.simple import *

Verbose = 1
stl_path = "trans.stl"
htg_path = sys.argv[1]
png_path = re.sub('\.htg$', '', htg_path) + ".png"
paraview.simple._DisableFirstRenderCameraReset()
materialLibrary1 = GetMaterialLibrary()
renderView1 = CreateView('RenderView')
renderView1.ViewSize = [2400, 800]
renderView1.AxesGrid = 'GridAxes3DActor'
renderView1.OrientationAxesVisibility = 0
renderView1.StereoType = 'Crystal Eyes'
renderView1.CameraPosition = [0, -1.5, 1.0]
renderView1.CameraFocalPoint = [0, 0, 1.0]
renderView1.CameraViewUp = [-1.0, 0.0, 0.0]
renderView1.CameraFocalDisk = 1.0
renderView1.CameraParallelScale = 0.5
renderView1.CameraParallelProjection = 1
renderView1.BackEnd = 'OSPRay raycaster'
renderView1.OSPRayMaterialLibrary = materialLibrary1
SetActiveView(None)
layout1 = CreateLayout()
layout1.AssignView(0, renderView1)
layout1.SetSize(1170, 989)
SetActiveView(renderView1)
stl = STLReader(FileNames=[stl_path])
htg = HyperTreeGridReader(FileNames=[htg_path])
if Verbose:
    sys.stderr.write("htg2png.py: open STL: %s\n" % stl_path)
    sys.stderr.write("htg2png.py: open HTG: %s\n" % htg_path)
hyperTreeGridToDualGrid1 = HyperTreeGridToDualGrid(Input=htg)
if Verbose:
    sys.stderr.write("htg2png.py: HyperTreeGridToDualGrid\n")
contour1 = Contour(Input=hyperTreeGridToDualGrid1)
if Verbose:
    sys.stderr.write("htg2png.py: Contour\n")
contour1.ContourBy = ['POINTS', 'omega']
contour1.Isosurfaces = [10.0, -10.0]
contour1.PointMergeMethod = 'Uniform Binning'
if Verbose:
    sys.stderr.write("htg2png.py: contour1Display\n")
contour1Display = Show(contour1, renderView1, 'GeometryRepresentation')
omegaTF2D = GetTransferFunction2D('omega')
omegaLUT = GetColorTransferFunction('omega')
omegaLUT.TransferFunction2D = omegaTF2D
omegaLUT.RGBPoints = [
    -10.0, 0.0, 0.0, 0.5625, -7.77778, 0.0, 0.0, 1.0, -2.69841, 0.0, 1.0, 1.0,
    -0.15873000000000026, 0.5, 1.0, 0.5, 2.3809499999999986, 1.0, 1.0, 0.0,
    7.460319999999999, 1.0, 0.0, 0.0, 10.0, 0.5, 0.0, 0.0
]
omegaLUT.ColorSpace = 'RGB'
omegaLUT.ScalarRangeInitialized = 1.0
contour1Display.Representation = 'Surface'
contour1Display.ColorArrayName = ['POINTS', 'omega']
contour1Display.LookupTable = omegaLUT
contour1Display.SelectTCoordArray = 'None'
contour1Display.SelectNormalArray = 'Normals'
contour1Display.SelectTangentArray = 'None'
contour1Display.OSPRayScaleArray = 'omega'
contour1Display.OSPRayScaleFunction = 'PiecewiseFunction'
contour1Display.SelectOrientationVectors = 'None'
contour1Display.ScaleFactor = 0.0802186906337738
contour1Display.SelectScaleArray = 'omega'
contour1Display.GlyphType = 'Arrow'
contour1Display.GlyphTableIndexArray = 'omega'
contour1Display.GaussianRadius = 0.00401093453168869
contour1Display.SetScaleArray = ['POINTS', 'omega']
contour1Display.ScaleTransferFunction = 'PiecewiseFunction'
contour1Display.OpacityArray = ['POINTS', 'omega']
contour1Display.OpacityTransferFunction = 'PiecewiseFunction'
contour1Display.DataAxesGrid = 'GridAxesRepresentation'
contour1Display.PolarAxes = 'PolarAxesRepresentation'
contour1Display.SelectInputVectors = ['POINTS', 'Normals']
contour1Display.WriteLog = ''
contour1Display.ScaleTransferFunction.Points = [
    -10.0, 0.0, 0.5, 0.0, 10.0, 1.0, 0.5, 0.0
]
contour1Display.OpacityTransferFunction.Points = [
    -10.0, 0.0, 0.5, 0.0, 10.0, 1.0, 0.5, 0.0
]
if Verbose:
    sys.stderr.write("htg2png.py: Show\n")
stlDisplay = Show(stl, renderView1, 'GeometryRepresentation')
stlDisplay.Representation = 'Surface'
stlDisplay.ColorArrayName = [None, '']
stlDisplay.SelectTCoordArray = 'None'
stlDisplay.SelectNormalArray = 'None'
stlDisplay.SelectTangentArray = 'None'
stlDisplay.OSPRayScaleFunction = 'PiecewiseFunction'
stlDisplay.SelectOrientationVectors = 'None'
stlDisplay.ScaleFactor = 0.0800000011920929
stlDisplay.SelectScaleArray = 'None'
stlDisplay.GlyphType = 'Arrow'
stlDisplay.GlyphTableIndexArray = 'None'
stlDisplay.GaussianRadius = 0.004000000059604645
stlDisplay.SetScaleArray = [None, '']
stlDisplay.ScaleTransferFunction = 'PiecewiseFunction'
stlDisplay.OpacityArray = [None, '']
stlDisplay.OpacityTransferFunction = 'PiecewiseFunction'
stlDisplay.DataAxesGrid = 'GridAxesRepresentation'
stlDisplay.PolarAxes = 'PolarAxesRepresentation'
stlDisplay.SelectInputVectors = [None, '']
stlDisplay.WriteLog = ''
omegaPWF = GetOpacityTransferFunction('omega')
omegaPWF.Points = [-10.0, 0.0, 0.5, 0.0, 10.0, 1.0, 0.5, 0.0]
omegaPWF.ScalarRangeInitialized = 1
SetActiveSource(None)
SaveScreenshot(png_path, ImageResolution=(2400, 800))
