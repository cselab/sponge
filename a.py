# state file generated using paraview version 5.11.1
import paraview
Sponge = False
paraview.compatibility.major = 5
paraview.compatibility.minor = 11
from paraview.simple import *
paraview.simple._DisableFirstRenderCameraReset()
materialLibrary1 = GetMaterialLibrary()
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
layout1 = CreateLayout(name='Layout #1')
layout1.AssignView(0, renderView1)
layout1.SetSize(1108, 805)
SetActiveView(renderView1)
if Sponge:
    output_pvd = PVDReader(registrationName='output_..pvd', FileName='/u/s/sponge/output_..pvd')
else:
    output_pvd = PVDReader(registrationName='output_..pvd', FileName='/u/s/sponge2/output_..pvd')
field = Slice(registrationName='field', Input=output_pvd)
field.SliceType = 'Plane'
field.HyperTreeGridSlicer = 'Plane'
field.SliceOffsetValues = [0.0]
field.SliceType.Origin = [0.0, 0.0, 1.5]
transposestl = STLReader(registrationName='transpose.stl', FileNames=['/u/transpose.stl'])
cylinderply = PLYReader(registrationName='cylinder.ply', FileNames=['/u/cylinder.ply'])
cylinderplyDisplay = Show(cylinderply, renderView1, 'GeometryRepresentation')
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
cylinderplyDisplay.OSPRayScaleFunction.Points = [-124.4246826171875, 0.0, 0.5, 0.0, 57.66411209106445, 0.02717391401529312, 0.5, 0.0, 150.0029754638672, 1.0, 0.5, 0.0]
cylinderplyDisplay.ScaleTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 0.6635220224576285, 0.02717391401529312, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0]
cylinderplyDisplay.OpacityTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 0.6635220224576285, 0.02717391401529312, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0]
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
    spongeDisplay.OSPRayScaleFunction.Points = [-124.4246826171875, 0.0, 0.5, 0.0, 57.66411209106445, 0.02717391401529312, 0.5, 0.0, 150.0029754638672, 1.0, 0.5, 0.0]
    spongeDisplay.ScaleTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 0.6635220224576285, 0.02717391401529312, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0]
    spongeDisplay.OpacityTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 0.6635220224576285, 0.02717391401529312, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0]
fieldDisplay = Show(field, renderView1, 'GeometryRepresentation')
omegaTF2D = GetTransferFunction2D('omega')
omegaTF2D.ScalarRangeInitialized = 1
omegaTF2D.Range = [-50.0, 50.0, 0.0, 1.0]
omegaLUT = GetColorTransferFunction('omega')
omegaLUT.AutomaticRescaleRangeMode = 'Never'
omegaLUT.TransferFunction2D = omegaTF2D
omegaLUT.RGBPoints = [-50.0, 0.5, 0.0, 0.0, -37.3016, 1.0, 0.0, 0.0, -11.904749999999993, 1.0, 1.0, 0.0, 0.7936499999999995, 0.5, 1.0, 0.5, 13.492049999999999, 0.0, 1.0, 1.0, 38.88890000000001, 0.0, 0.0, 1.0, 50.0, 0.0, 0.0, 0.5625]
omegaLUT.ColorSpace = 'RGB'
omegaLUT.ScalarRangeInitialized = 1.0
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
fieldDisplay.OSPRayScaleFunction.Points = [-124.4246826171875, 0.0, 0.5, 0.0, 57.66411209106445, 0.02717391401529312, 0.5, 0.0, 150.0029754638672, 1.0, 0.5, 0.0]
fieldDisplay.ScaleTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 0.6635220224576285, 0.02717391401529312, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0]
fieldDisplay.OpacityTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 0.6635220224576285, 0.02717391401529312, 0.5, 0.0, 1.0, 1.0, 0.5, 0.0]
omegaPWF = GetOpacityTransferFunction('omega')
omegaPWF.Points = [-50.0, 0.0, 0.5, 0.0, 50.0, 1.0, 0.5, 0.0]
omegaPWF.ScalarRangeInitialized = 1
SetActiveSource(field)
for i, time in enumerate(GetTimeKeeper().TimestepValues):
    GetAnimationScene().AnimationTime = time
    path = "a.%09d.png" % i
    sys.stderr.write("%s\n" % path)
    SaveScreenshot(path, ImageResolution=(1200, 800))
