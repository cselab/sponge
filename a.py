# state file generated using paraview version 5.11.1
import paraview
Sponge = True
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
renderView1.ViewSize = [1108, 805]
renderView1.AxesGrid = 'GridAxes3DActor'
renderView1.OrientationAxesVisibility = 0
renderView1.CenterOfRotation = [0.0, 0.0, 1.5]
renderView1.StereoType = 'Crystal Eyes'
renderView1.CameraPosition = [13.74740168055299, -0.01363277000699272, 0.9512810072185296]
renderView1.CameraFocalPoint = [0.0, -0.01363277000699272, 0.9512810072185296]
renderView1.CameraViewUp = [0.0, -1.0, -6.661338147750939e-16]
renderView1.CameraFocalDisk = 1.0
renderView1.CameraParallelScale = 0.9369561382102826
renderView1.CameraParallelProjection = 1
renderView1.BackEnd = 'OSPRay raycaster'
renderView1.OSPRayMaterialLibrary = materialLibrary1

SetActiveView(None)

# ----------------------------------------------------------------
# setup view layouts
# ----------------------------------------------------------------

# create new layout object 'Layout #1'
layout1 = CreateLayout(name='Layout #1')
layout1.AssignView(0, renderView1)
layout1.SetSize(1108, 805)

# ----------------------------------------------------------------
# restore active view
SetActiveView(renderView1)
# ----------------------------------------------------------------

# ----------------------------------------------------------------
# setup the data processing pipelines
# ----------------------------------------------------------------

# create a new 'PVD Reader'
if Sponge:
    output_pvd = PVDReader(registrationName='output_..pvd', FileName='/u/s/sponge2/output_..pvd')
else:
    output_pvd = PVDReader(registrationName='output_..pvd', FileName='/u/s/sponge/output_..pvd')

# create a new 'Slice'
field = Slice(registrationName='field', Input=output_pvd)
field.SliceType = 'Plane'
field.HyperTreeGridSlicer = 'Plane'
field.SliceOffsetValues = [0.0]

# init the 'Plane' selected for 'SliceType'
field.SliceType.Origin = [0.0, 0.0, 1.5]

# create a new 'STL Reader'
transposestl = STLReader(registrationName='transpose.stl', FileNames=['/u/transpose.stl'])
# create a new 'PLY Reader'
cylinderply = PLYReader(registrationName='cylinder.ply', FileNames=['/u/cylinder.ply'])

# ----------------------------------------------------------------
# setup the visualization in view 'renderView1'
# ----------------------------------------------------------------

# show data from cylinderply
cylinderplyDisplay = Show(cylinderply, renderView1, 'GeometryRepresentation')

# trace defaults for the display properties.
cylinderplyDisplay.Representation = 'Surface'
cylinderplyDisplay.ColorArrayName = ['POINTS', '']
cylinderplyDisplay.SelectTCoordArray = 'None'
cylinderplyDisplay.SelectNormalArray = 'None'
cylinderplyDisplay.SelectTangentArray = 'None'
cylinderplyDisplay.OSPRayScaleFunction = 'PiecewiseFunction'
cylinderplyDisplay.SelectOrientationVectors = 'None'
cylinderplyDisplay.ScaleFactor = 0.0800000011920929
cylinderplyDisplay.SelectScaleArray = 'None'
cylinderplyDisplay.GlyphType = 'Arrow'
cylinderplyDisplay.GlyphTableIndexArray = 'None'
cylinderplyDisplay.GaussianRadius = 0.004000000059604645
cylinderplyDisplay.SetScaleArray = ['POINTS', '']
cylinderplyDisplay.ScaleTransferFunction = 'PiecewiseFunction'
cylinderplyDisplay.OpacityArray = ['POINTS', '']
cylinderplyDisplay.OpacityTransferFunction = 'PiecewiseFunction'
cylinderplyDisplay.DataAxesGrid = 'GridAxesRepresentation'
cylinderplyDisplay.PolarAxes = 'PolarAxesRepresentation'
cylinderplyDisplay.SelectInputVectors = ['POINTS', '']
cylinderplyDisplay.WriteLog = ''
# init the 'PiecewiseFunction' selected for 'OSPRayScaleFunction'
cylinderplyDisplay.OSPRayScaleFunction.Points = [-124.4246826171875, 0.0, 0.5, 0.0, 57.66411209106445, 0.02717391401529312, 0.5, 0.0, 150.0029754638672, 1.0, 0.5, 0.0]

# init the 'PiecewiseFunction' selected for 'ScaleTransferFunction'
cylinderplyDisplay.ScaleTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 0.6635220224576285, 0.02717391401529312, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0]

# init the 'PiecewiseFunction' selected for 'OpacityTransferFunction'
cylinderplyDisplay.OpacityTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 0.6635220224576285, 0.02717391401529312, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0]

# create a new 'Slice'
if Sponge:
    sponge = Slice(registrationName='sponge', Input=transposestl)
    sponge.SliceType = 'Plane'
    sponge.HyperTreeGridSlicer = 'Plane'
    sponge.SliceOffsetValues = [0.0]
    sponge.HyperTreeGridSlicer.Origin = [1.4901161193847656e-08, 7.450580596923828e-09, 0.0]
    spongeDisplay = Show(sponge, renderView1, 'GeometryRepresentation')
    spongeDisplay.Representation = 'Wireframe'
    spongeDisplay.ColorArrayName = ['POINTS', '']
    spongeDisplay.LineWidth = 3.0
    spongeDisplay.SelectTCoordArray = 'None'
    spongeDisplay.SelectNormalArray = 'None'
    spongeDisplay.SelectTangentArray = 'None'
    spongeDisplay.OSPRayScaleFunction = 'PiecewiseFunction'
    spongeDisplay.SelectOrientationVectors = 'None'
    spongeDisplay.ScaleFactor = 0.04595768451690674
    spongeDisplay.SelectScaleArray = 'None'
    spongeDisplay.GlyphType = 'Arrow'
    spongeDisplay.GlyphTableIndexArray = 'None'
    spongeDisplay.GaussianRadius = 0.002297884225845337
    spongeDisplay.SetScaleArray = ['POINTS', '']
    spongeDisplay.ScaleTransferFunction = 'PiecewiseFunction'
    spongeDisplay.OpacityArray = ['POINTS', '']
    spongeDisplay.OpacityTransferFunction = 'PiecewiseFunction'
    spongeDisplay.DataAxesGrid = 'GridAxesRepresentation'
    spongeDisplay.PolarAxes = 'PolarAxesRepresentation'
    spongeDisplay.SelectInputVectors = ['POINTS', '']
    spongeDisplay.WriteLog = ''

    # init the 'PiecewiseFunction' selected for 'OSPRayScaleFunction'
    spongeDisplay.OSPRayScaleFunction.Points = [-124.4246826171875, 0.0, 0.5, 0.0, 57.66411209106445, 0.02717391401529312, 0.5, 0.0, 150.0029754638672, 1.0, 0.5, 0.0]

    # init the 'PiecewiseFunction' selected for 'ScaleTransferFunction'
    spongeDisplay.ScaleTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 0.6635220224576285, 0.02717391401529312, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0]

    # init the 'PiecewiseFunction' selected for 'OpacityTransferFunction'
    spongeDisplay.OpacityTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 0.6635220224576285, 0.02717391401529312, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0]

# show data from field
fieldDisplay = Show(field, renderView1, 'GeometryRepresentation')

# get 2D transfer function for 'omega'
omegaTF2D = GetTransferFunction2D('omega')
omegaTF2D.ScalarRangeInitialized = 1
omegaTF2D.Range = [-50.0, 50.0, 0.0, 1.0]

# get color transfer function/color map for 'omega'
omegaLUT = GetColorTransferFunction('omega')
omegaLUT.AutomaticRescaleRangeMode = 'Never'
omegaLUT.TransferFunction2D = omegaTF2D
omegaLUT.RGBPoints = [-50.0, 0.5, 0.0, 0.0, -37.3016, 1.0, 0.0, 0.0, -11.904749999999993, 1.0, 1.0, 0.0, 0.7936499999999995, 0.5, 1.0, 0.5, 13.492049999999999, 0.0, 1.0, 1.0, 38.88890000000001, 0.0, 0.0, 1.0, 50.0, 0.0, 0.0, 0.5625]
omegaLUT.ColorSpace = 'RGB'
omegaLUT.ScalarRangeInitialized = 1.0

# trace defaults for the display properties.
fieldDisplay.Representation = 'Surface'
fieldDisplay.ColorArrayName = ['CELLS', 'omega']
fieldDisplay.LookupTable = omegaLUT
fieldDisplay.SelectTCoordArray = 'None'
fieldDisplay.SelectNormalArray = 'None'
fieldDisplay.SelectTangentArray = 'None'
fieldDisplay.OSPRayScaleFunction = 'PiecewiseFunction'
fieldDisplay.SelectOrientationVectors = 'None'
fieldDisplay.ScaleFactor = 0.5
fieldDisplay.SelectScaleArray = 'u'
fieldDisplay.GlyphType = 'Arrow'
fieldDisplay.GlyphTableIndexArray = 'u'
fieldDisplay.GaussianRadius = 0.025
fieldDisplay.SetScaleArray = [None, '']
fieldDisplay.ScaleTransferFunction = 'PiecewiseFunction'
fieldDisplay.OpacityArray = [None, '']
fieldDisplay.OpacityTransferFunction = 'PiecewiseFunction'
fieldDisplay.DataAxesGrid = 'GridAxesRepresentation'
fieldDisplay.PolarAxes = 'PolarAxesRepresentation'
fieldDisplay.SelectInputVectors = [None, '']
fieldDisplay.WriteLog = ''

# init the 'PiecewiseFunction' selected for 'OSPRayScaleFunction'
fieldDisplay.OSPRayScaleFunction.Points = [-124.4246826171875, 0.0, 0.5, 0.0, 57.66411209106445, 0.02717391401529312, 0.5, 0.0, 150.0029754638672, 1.0, 0.5, 0.0]

# init the 'PiecewiseFunction' selected for 'ScaleTransferFunction'
fieldDisplay.ScaleTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 0.6635220224576285, 0.02717391401529312, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0]

# init the 'PiecewiseFunction' selected for 'OpacityTransferFunction'
fieldDisplay.OpacityTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 0.6635220224576285, 0.02717391401529312, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0]

# ----------------------------------------------------------------
# setup color maps and opacity mapes used in the visualization
# note: the Get..() functions create a new object, if needed
# ----------------------------------------------------------------

# get opacity transfer function/opacity map for 'omega'
omegaPWF = GetOpacityTransferFunction('omega')
omegaPWF.Points = [-50.0, 0.0, 0.5, 0.0, 50.0, 1.0, 0.5, 0.0]
omegaPWF.ScalarRangeInitialized = 1

# ----------------------------------------------------------------
# restore active source
SetActiveSource(field)
# ----------------------------------------------------------------

for i, time in enumerate(GetTimeKeeper().TimestepValues):
    GetAnimationScene().AnimationTime = time
    path = "a.%09d.png" % i
    sys.stderr.write("%s\n" % path)
    SaveScreenshot(path)
