# state file generated using paraview version 5.11.1
import paraview

paraview.compatibility.major = 5
paraview.compatibility.minor = 11

#### import the simple module from the paraview
from paraview.simple import *
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

# ----------------------------------------------------------------
# setup views used in the visualization
# ----------------------------------------------------------------

# get the material library
materialLibrary1 = GetMaterialLibrary()

# Create a new 'Render View'
renderView1 = CreateView('RenderView')
renderView1.ViewSize = [1920, 1080]
renderView1.AxesGrid = 'GridAxes3DActor'
renderView1.OrientationAxesVisibility = 0
renderView1.CenterOfRotation = [
    504.8041639328003, 31.59588623046875, 506.63003063201904
]
renderView1.KeyLightWarmth = 0.5
renderView1.FillLightWarmth = 0.5
renderView1.StereoType = 'Crystal Eyes'
renderView1.CameraPosition = [
    -2074.958443060363, 2167.0582426527603, 1707.2474342700255
]
renderView1.CameraFocalPoint = [
    765.5794662595805, 508.8841664915863, 465.80266374908916
]
renderView1.CameraViewUp = [
    0.3472719068449284, 0.8653781398795282, -0.3612795285301056
]
renderView1.CameraFocalDisk = 1.0
renderView1.CameraParallelScale = 350.8066009897884
renderView1.CameraParallelProjection = 1
renderView1.UseColorPaletteForBackground = 0
renderView1.Background = [1.0, 1.0, 1.0]
renderView1.EnableRayTracing = 1
renderView1.BackEnd = 'OSPRay raycaster'
renderView1.Shadows = 1
renderView1.SamplesPerPixel = 10
renderView1.OSPRayMaterialLibrary = materialLibrary1

SetActiveView(None)

# ----------------------------------------------------------------
# setup view layouts
# ----------------------------------------------------------------

# create new layout object 'Layout #1'
layout1 = CreateLayout(name='Layout #1')
layout1.AssignView(0, renderView1)
layout1.SetSize(1920, 1080)

# ----------------------------------------------------------------
# restore active view
SetActiveView(renderView1)
# ----------------------------------------------------------------

# ----------------------------------------------------------------
# setup the data processing pipelines
# ----------------------------------------------------------------

# create a new 'Cylinder'
cylinder1 = Cylinder(registrationName='Cylinder1')
cylinder1.Resolution = 512
cylinder1.Height = 1124.0
cylinder1.Radius = 10.24

# create a new 'XDMF Reader'
meshxdmf2 = XDMFReader(
    registrationName='mesh.xdmf2',
    FileNames=['/home/lisergey/cudaAmrIsoSurfaceExtraction/mesh.xdmf2'])
meshxdmf2.PointArrayStatus = ['Attribute_3']
meshxdmf2.GridStatus = ['Grid_2']

# create a new 'Extract Surface'
extractSurface1 = ExtractSurface(registrationName='ExtractSurface1',
                                 Input=meshxdmf2)

# create a new 'Generate Surface Normals'
generateSurfaceNormals1 = GenerateSurfaceNormals(
    registrationName='GenerateSurfaceNormals1', Input=extractSurface1)

# create a new 'Transform'
transform1 = Transform(registrationName='Transform1', Input=cylinder1)
transform1.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
transform1.Transform.Translate = [409.6, 512.0, 512.0]
transform1.Transform.Rotate = [90.0, 0.0, 0.0]

# ----------------------------------------------------------------
# setup the visualization in view 'renderView1'
# ----------------------------------------------------------------

# show data from generateSurfaceNormals1
generateSurfaceNormals1Display = Show(generateSurfaceNormals1, renderView1,
                                      'GeometryRepresentation')

# get 2D transfer function for 'Attribute_3'
attribute_3TF2D = GetTransferFunction2D('Attribute_3')
attribute_3TF2D.ScalarRangeInitialized = 1
attribute_3TF2D.Range = [-1.0, 1.0, 0.0, 1.0]

# get color transfer function/color map for 'Attribute_3'
attribute_3LUT = GetColorTransferFunction('Attribute_3')
attribute_3LUT.AutomaticRescaleRangeMode = 'Never'
attribute_3LUT.TransferFunction2D = attribute_3TF2D
attribute_3LUT.RGBPoints = [
    -1.0, 0.176471, 0.0, 0.294118, -0.87451, 0.272434, 0.095963, 0.444214,
    -0.74902, 0.373395, 0.228912, 0.56932, -0.623529, 0.481661, 0.415917,
    0.657901, -0.498039, 0.601922, 0.562937, 0.750481, -0.372549, 0.718493,
    0.695886, 0.836986, -0.24705900000000003, 0.811995, 0.811534, 0.898501,
    -0.12156900000000004, 0.894733, 0.8995, 0.940023, 0.00392156999999993,
    0.969166, 0.966859, 0.963629, 0.12941200000000008, 0.98639, 0.910265,
    0.803691, 0.25490199999999996, 0.995002, 0.835371, 0.624375,
    0.38039200000000006, 0.992541, 0.736947, 0.420146, 0.5058820000000004,
    0.931949, 0.609458, 0.224221, 0.631373, 0.85075, 0.483968, 0.069819,
    0.7568630000000001, 0.740023, 0.380623, 0.035371, 0.8823530000000002,
    0.617993, 0.29827, 0.026759, 1.0, 0.498039, 0.231373, 0.031373
]
attribute_3LUT.ColorSpace = 'Lab'
attribute_3LUT.ScalarRangeInitialized = 1.0

# trace defaults for the display properties.
generateSurfaceNormals1Display.Representation = 'Surface'
generateSurfaceNormals1Display.ColorArrayName = ['POINTS', 'Attribute_3']
generateSurfaceNormals1Display.LookupTable = attribute_3LUT
generateSurfaceNormals1Display.SelectTCoordArray = 'None'
generateSurfaceNormals1Display.SelectNormalArray = 'Normals'
generateSurfaceNormals1Display.SelectTangentArray = 'None'
generateSurfaceNormals1Display.OSPRayScaleArray = 'Attribute_3'
generateSurfaceNormals1Display.OSPRayScaleFunction = 'PiecewiseFunction'
generateSurfaceNormals1Display.SelectOrientationVectors = 'None'
generateSurfaceNormals1Display.ScaleFactor = 102.30000610351563
generateSurfaceNormals1Display.SelectScaleArray = 'Attribute_3'
generateSurfaceNormals1Display.GlyphType = 'Arrow'
generateSurfaceNormals1Display.GlyphTableIndexArray = 'Attribute_3'
generateSurfaceNormals1Display.GaussianRadius = 5.115000305175782
generateSurfaceNormals1Display.SetScaleArray = ['POINTS', 'Attribute_3']
generateSurfaceNormals1Display.ScaleTransferFunction = 'PiecewiseFunction'
generateSurfaceNormals1Display.OpacityArray = ['POINTS', 'Attribute_3']
generateSurfaceNormals1Display.OpacityTransferFunction = 'PiecewiseFunction'
generateSurfaceNormals1Display.DataAxesGrid = 'GridAxesRepresentation'
generateSurfaceNormals1Display.PolarAxes = 'PolarAxesRepresentation'
generateSurfaceNormals1Display.SelectInputVectors = ['POINTS', 'Normals']
generateSurfaceNormals1Display.WriteLog = ''

# init the 'PiecewiseFunction' selected for 'ScaleTransferFunction'
generateSurfaceNormals1Display.ScaleTransferFunction.Points = [
    -117.15668487548828, 0.0, 0.5, 0.0, 117.43035888671875, 1.0, 0.5, 0.0
]

# init the 'PiecewiseFunction' selected for 'OpacityTransferFunction'
generateSurfaceNormals1Display.OpacityTransferFunction.Points = [
    -117.15668487548828, 0.0, 0.5, 0.0, 117.43035888671875, 1.0, 0.5, 0.0
]

# show data from transform1
transform1Display = Show(transform1, renderView1, 'GeometryRepresentation')

# trace defaults for the display properties.
transform1Display.Representation = 'Surface'
transform1Display.AmbientColor = [1.0, 0.0, 0.0]
transform1Display.ColorArrayName = [None, '']
transform1Display.DiffuseColor = [1.0, 0.0, 0.0]
transform1Display.SelectTCoordArray = 'TCoords'
transform1Display.SelectNormalArray = 'Normals'
transform1Display.SelectTangentArray = 'None'
transform1Display.OSPRayScaleArray = 'Normals'
transform1Display.OSPRayScaleFunction = 'PiecewiseFunction'
transform1Display.SelectOrientationVectors = 'None'
transform1Display.ScaleFactor = 102.4
transform1Display.SelectScaleArray = 'None'
transform1Display.GlyphType = 'Arrow'
transform1Display.GlyphTableIndexArray = 'None'
transform1Display.GaussianRadius = 5.12
transform1Display.SetScaleArray = ['POINTS', 'Normals']
transform1Display.ScaleTransferFunction = 'PiecewiseFunction'
transform1Display.OpacityArray = ['POINTS', 'Normals']
transform1Display.OpacityTransferFunction = 'PiecewiseFunction'
transform1Display.DataAxesGrid = 'GridAxesRepresentation'
transform1Display.PolarAxes = 'PolarAxesRepresentation'
transform1Display.SelectInputVectors = ['POINTS', 'Normals']
transform1Display.WriteLog = ''

# init the 'PiecewiseFunction' selected for 'ScaleTransferFunction'
transform1Display.ScaleTransferFunction.Points = [
    -1.0, 0.0, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0
]

# init the 'PiecewiseFunction' selected for 'OpacityTransferFunction'
transform1Display.OpacityTransferFunction.Points = [
    -1.0, 0.0, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0
]

# ----------------------------------------------------------------
# setup color maps and opacity mapes used in the visualization
# note: the Get..() functions create a new object, if needed
# ----------------------------------------------------------------

# get opacity transfer function/opacity map for 'Attribute_3'
attribute_3PWF = GetOpacityTransferFunction('Attribute_3')
attribute_3PWF.Points = [-1.0, 0.0, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0]
attribute_3PWF.ScalarRangeInitialized = 1

# ----------------------------------------------------------------
# restore active source
SetActiveSource(generateSurfaceNormals1)
# ----------------------------------------------------------------

if __name__ == '__main__':
    # generate extracts
    SaveExtracts(ExtractsOutputDirectory='extracts')
