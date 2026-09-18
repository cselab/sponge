from paraview.simple import *
import numpy as np
import os
import re
import sys

if len(sys.argv) < 4:
    sys.stderr.write(
        "usage: pvpython pvsurf.py LEVEL[,ZOOM[,WIDTH]] center.stl FILE.xdmf2 [FILE.xdmf2 ...]\n"
        "       LEVEL is an |omega| value or lVALUE (lambda2), as given to iso.py\n"
        "       center.stl is the sponge surface as given to stl2dump\n"
        "       writes FILE.omegaLEVEL.png or FILE.l2VALUE.png\n")
    sys.exit(1)
arg = sys.argv[1].split(",")
spec = arg[0]
tag = "l2%g" % float(spec[1:]) if spec.startswith("l") else "omega%g" % float(spec)
zoom = float(arg[1]) if len(arg) > 1 else 1.0
width = int(arg[2]) if len(arg) > 2 else 1920
stl = sys.argv[2]
paths = [p for p in sys.argv[3:]
         if not re.search(r"\.(omega|l2)[0-9.e+-]+\.xdmf2$", p)]

# color range and camera box over the whole sequence
m = 0.0
lo = np.full(3, np.inf)
hi = np.full(3, -np.inf)
for path in paths:
    base = os.path.splitext(path)[0]
    a = np.fromfile("%s.%s.attr.raw" % (base, tag), np.float32)
    if a.size:
        m = max(m, float(np.abs(a).max()))
    x = np.fromfile("%s.%s.xyz.raw" % (base, tag), np.float32).reshape(-1, 3)
    if x.size:
        lo = np.minimum(lo, x.min(0))
        hi = np.maximum(hi, x.max(0))
if m == 0:
    m = 1.0

view = GetActiveViewOrCreate("RenderView")
view.ViewSize = [width, width * 9 // 16]
view.Background = [1, 1, 1]
view.OrientationAxesVisibility = 0
view.UseColorPaletteForBackground = 0
body = STLReader(FileNames=[stl])
bodyn = SurfaceNormals(Input=body)
bodyn.FeatureAngle = 80
bd = Show(bodyn, view)
bd.ColorArrayName = ["POINTS", ""]
bd.DiffuseColor = [0.2, 0.2, 0.2]
bd.Specular = 0.3
bodyn.UpdatePipeline()
b = bodyn.GetDataInformation().GetBounds()
lo = np.minimum(lo, b[0::2])
hi = np.maximum(hi, b[1::2])
c = (lo + hi) / 2
L = float((hi - lo).max())

lut = GetColorTransferFunction("u")
lut.ApplyPreset("Cool to Warm", True)
lut.AutomaticRescaleRangeMode = "Never"
lut.RescaleTransferFunction(-m, m)
for path in paths:
    base = os.path.splitext(path)[0]
    surf = XDMFReader(FileNames=["%s.%s.xdmf2" % (base, tag)])
    surf.UpdatePipeline()
    surfs = ExtractSurface(Input=surf)
    smooth = Smooth(Input=surfs)
    smooth.NumberofIterations = 20
    surfn = SurfaceNormals(Input=smooth)
    surfn.FeatureAngle = 80
    sd = Show(surfn, view)
    ColorBy(sd, ("POINTS", "u"))
    sd.LookupTable = lut
    sd.Specular = 0.2
    sd.Opacity = 0.45
    Render(view)
    # flow along x, sponge axis along z: look from the -y side, slightly above
    view.CameraFocalPoint = list(c)
    view.CameraViewUp = [0, 0, 1]
    view.CameraPosition = [c[0] + 0.3 * L, c[1] - 1.2 * L, c[2] + 0.5 * L]
    view.ResetCamera(lo[0], hi[0], lo[1], hi[1], lo[2], hi[2])
    view.GetActiveCamera().Dolly(zoom)
    Render(view)
    SaveScreenshot("%s.%s.png" % (base, tag), view)
    Delete(sd)
    for p in (surfn, smooth, surfs, surf):
        Delete(p)
