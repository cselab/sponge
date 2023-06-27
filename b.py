import os
import paraview
import sys
Sponge = False
while True:
    sys.argv.pop(0)
    if len(sys.argv) and len(sys.argv[0]) > 1 and sys.argv[0][0] == '-':
        if sys.argv[0][1] == 's':
            Sponge = True
            stl_path = os.path.join(os.getenv("HOME"), ".local", "transpose.stl")
        else:
            sys.stderr.write("%s: unknown option '%s'\n" % (me, sys.argv[0]))
            sys.exit(2)
    else:
        break
cylinder_path = os.path.join(os.getenv("HOME"), ".local", "cylinder.ply")
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
renderView1.CameraPosition = [-3.0112501042143287, 1.8090864291908322, -1.170887119205709]
renderView1.CameraFocalPoint = [0.16209246453373574, 0.01769547174407124, 1.3050537403394105]
renderView1.CameraViewUp = [0.30031780674912945, 0.9131120743592498, 0.27574545256221444]
renderView1.CameraFocalDisk = 1.0
renderView1.CameraParallelScale = 4.330127018922194
renderView1.BackEnd = 'OSPRay raycaster'
renderView1.OSPRayMaterialLibrary = materialLibrary1
SetActiveView(None)
layout1 = CreateLayout(name='Layout #1')
layout1.AssignView(0, renderView1)
layout1.SetSize(1108, 805)
SetActiveView(renderView1)
cylinderply = PLYReader(registrationName='cylinder.ply', FileNames=[cylinder_path])
if Sponge:
    transposestl = STLReader(registrationName='transpose.stl', FileNames=[stl_path])
    transposestlDisplay = Show(transposestl, renderView1, 'GeometryRepresentation')
    transposestlDisplay.Representation = 'Surface'
    transposestlDisplay.ColorArrayName = [None, '']
    transposestlDisplay.SelectTCoordArray = 'None'
    transposestlDisplay.SelectNormalArray = 'None'
    transposestlDisplay.SelectTangentArray = 'None'
    transposestlDisplay.OSPRayScaleFunction = 'PiecewiseFunction'
    transposestlDisplay.SelectOrientationVectors = 'None'
    transposestlDisplay.ScaleFactor = 0.07999999821186066
    transposestlDisplay.SelectScaleArray = 'None'
    transposestlDisplay.GlyphType = 'Arrow'
    transposestlDisplay.GlyphTableIndexArray = 'None'
    transposestlDisplay.GaussianRadius = 0.003999999910593033
    transposestlDisplay.SetScaleArray = [None, '']
    transposestlDisplay.ScaleTransferFunction = 'PiecewiseFunction'
    transposestlDisplay.OpacityArray = [None, '']
    transposestlDisplay.OpacityTransferFunction = 'PiecewiseFunction'
    transposestlDisplay.DataAxesGrid = 'GridAxesRepresentation'
    transposestlDisplay.PolarAxes = 'PolarAxesRepresentation'
    transposestlDisplay.SelectInputVectors = [None, '']
    transposestlDisplay.WriteLog = ''
    output_pvd = PVDReader(registrationName='output_..pvd', FileName='/u/s/sponge/output_..pvd')
else:
    output_pvd = PVDReader(registrationName='output_..pvd', FileName='/u/s/sponge2/output_..pvd')
cylinderplyDisplay = Show(cylinderply, renderView1, 'GeometryRepresentation')
cylinderplyDisplay.Representation = 'Surface'
cylinderplyDisplay.ColorArrayName = [None, '']
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
cylinderplyDisplay.SetScaleArray = [None, '']
cylinderplyDisplay.ScaleTransferFunction = 'PiecewiseFunction'
cylinderplyDisplay.OpacityArray = [None, '']
cylinderplyDisplay.OpacityTransferFunction = 'PiecewiseFunction'
cylinderplyDisplay.DataAxesGrid = 'GridAxesRepresentation'
cylinderplyDisplay.PolarAxes = 'PolarAxesRepresentation'
cylinderplyDisplay.SelectInputVectors = [None, '']
cylinderplyDisplay.WriteLog = ''
omegaTF2D = GetTransferFunction2D('omega')
omegaTF2D.ScalarRangeInitialized = 1
omegaTF2D.Range = [-50.0, 50.0, 0.0, 1.0]
omegaLUT = GetColorTransferFunction('omega')
omegaLUT.AutomaticRescaleRangeMode = 'Never'
omegaLUT.TransferFunction2D = omegaTF2D
omegaLUT.RGBPoints = [-50.0, 0.5, 0.0, 0.0, -37.301600000000015, 1.0, 0.0, 0.0, -11.90475, 1.0, 1.0, 0.0, 0.7936499999999995, 0.5, 1.0, 0.5, 13.492049999999999, 0.0, 1.0, 1.0, 38.88890000000001, 0.0, 0.0, 1.0, 50.0, 0.0, 0.0, 0.5625]
omegaLUT.ColorSpace = 'RGB'
omegaLUT.ScalarRangeInitialized = 1.0
for i, time in enumerate(GetTimeKeeper().TimestepValues):
    GetAnimationScene().AnimationTime = time
    hyperTreeGridToDualGrid1 = HyperTreeGridToDualGrid(registrationName='HyperTreeGridToDualGrid1', Input=output_pvd)
    contour1 = Contour(registrationName='Contour1', Input=hyperTreeGridToDualGrid1)
    contour1.ContourBy = ['POINTS', 'omega']
    contour1.Isosurfaces = [-60.0, -50.0, -30.0, -10.0, 10.0, 30.0, 50.0, 60.0]
    contour1.PointMergeMethod = 'Uniform Binning'
    contour1Display = Show(contour1, renderView1, 'GeometryRepresentation')
    contour1Display.Representation = 'Surface'
    contour1Display.ColorArrayName = ['POINTS', 'omega']
    contour1Display.LookupTable = omegaLUT
    contour1Display.Opacity = 0.26
    contour1Display.SelectTCoordArray = 'None'
    contour1Display.SelectNormalArray = 'Normals'
    contour1Display.SelectTangentArray = 'None'
    contour1Display.OSPRayScaleArray = 'omega'
    contour1Display.OSPRayScaleFunction = 'PiecewiseFunction'
    contour1Display.SelectOrientationVectors = 'None'
    contour1Display.ScaleFactor = 0.3098579525947571
    contour1Display.SelectScaleArray = 'omega'
    contour1Display.GlyphType = 'Arrow'
    contour1Display.GlyphTableIndexArray = 'omega'
    contour1Display.GaussianRadius = 0.015492897629737854
    contour1Display.SetScaleArray = ['POINTS', 'omega']
    contour1Display.ScaleTransferFunction = 'PiecewiseFunction'
    contour1Display.OpacityArray = ['POINTS', 'omega']
    contour1Display.OpacityTransferFunction = 'PiecewiseFunction'
    contour1Display.DataAxesGrid = 'GridAxesRepresentation'
    contour1Display.PolarAxes = 'PolarAxesRepresentation'
    contour1Display.SelectInputVectors = ['POINTS', 'Normals']
    contour1Display.WriteLog = ''
    contour1Display.ScaleTransferFunction.Points = [50.0, 0.0, 0.5, 0.0, 50.0078125, 1.0, 0.5, 0.0]
    contour1Display.OpacityTransferFunction.Points = [50.0, 0.0, 0.5, 0.0, 50.0078125, 1.0, 0.5, 0.0]
    omegaPWF = GetOpacityTransferFunction('omega')
    omegaPWF.Points = [-50.0, 0.0, 0.5, 0.0, 50.0, 1.0, 0.5, 0.0]
    omegaPWF.ScalarRangeInitialized = 1
    SetActiveSource(contour1)
    path = "a.%09d.png" % i
    sys.stderr.write("%s\n" % path)
    SaveScreenshot(path, ImageResolution=(1200, 800))
    Delete(contour1Display)
    Delete(contour1)
    Delete(hyperTreeGridToDualGrid1)
    del contour1Display
    del contour1
    del hyperTreeGridToDualGrid1
