from paraview.simple import *
import numpy as np
import os
import re
import sys

if len(sys.argv) < 4:
    sys.stderr.write(
        "usage: pvpython pvsurf.py [-r SAMPLES] [-a] [-c PERCENTILE]\n"
        "                          LEVEL[,ZOOM[,WIDTH]] center.stl FILE.xdmf2 [FILE.xdmf2 ...]\n"
        "       -r SAMPLES  OSPRay path tracer, SAMPLES per pixel, denoised\n"
        "       -a          screen space ambient occlusion (rasterizer only)\n"
        "       -c PERCENTILE  colour range percentile of |omega_z| (default 99)\n"
        "       LEVEL is an |omega| value, qVALUE or lVALUE, as given to iso.py\n"
        "       center.stl is the sponge surface as given to stl2dump\n"
        "       writes FILE.omegaLEVEL.png, FILE.qVALUE.png or FILE.l2VALUE.png\n")
    sys.exit(1)
args = sys.argv[1:]
rays = 0
ssao = 0
pct = 99.0
while args and args[0].startswith("-") and not args[0][1:2].isdigit():
    if args[0] == "-r":
        rays = int(args[1])
        args = args[2:]
    elif args[0] == "-c":
        pct = float(args[1])
        args = args[2:]
    elif args[0] == "-a":
        ssao = 1
        args = args[1:]
    else:
        sys.exit("pvsurf.py: unknown option %s" % args[0])
arg = args[0].split(",")
spec = arg[0]
if spec.startswith("l"):
    tag = "l2%g" % float(spec[1:])
elif spec.startswith("q"):
    tag = "q%g" % float(spec[1:])
else:
    tag = "omega%g" % float(spec)
zoom = float(arg[1]) if len(arg) > 1 else 1.0
width = int(arg[2]) if len(arg) > 2 else 1920
stl = args[1]
paths = [p for p in args[2:]
         if not re.search(r"\.(omega|l2|q)[0-9.e+-]+\.xdmf2$", p)]

m = 0.0
lo = np.full(3, np.inf)
hi = np.full(3, -np.inf)
for path in paths:
    base = os.path.splitext(path)[0]
    a = np.fromfile("%s.%s.attr.raw" % (base, tag), np.float32)
    if a.size:
        m = max(m, float(np.percentile(np.abs(a), pct)))
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
if ssao and not rays:
    for k, val in [("UseAmbientOcclusion", 1), ("UseSSAODefaultPresets", 1),
                   ("AmbientSamples", 8), ("Shadows", 0)]:
        if hasattr(view, k):
            setattr(view, k, val)
body = STLReader(FileNames=[stl])
bodyn = SurfaceNormals(Input=body)
bodyn.FeatureAngle = 80
bd = Show(bodyn, view)
bd.ColorArrayName = ["POINTS", ""]
bd.DiffuseColor = [0.28, 0.27, 0.32]
bd.Specular = 0.0 if (ssao or rays) else 0.3
bodyn.UpdatePipeline()
b = bodyn.GetDataInformation().GetBounds()
lo = np.minimum(lo, b[0::2])
hi = np.maximum(hi, b[1::2])
hi[0] = max(hi[0], lo[0] + (hi[2] - lo[2]) * width / (width * 9 // 16))
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
    sd.Specular = 0.0 if (ssao or rays) else 0.2
    sd.Opacity = 0.45
    Render(view)
    view.CameraParallelProjection = 1
    view.ResetCamera(lo[0], hi[0], lo[1], hi[1], lo[2], hi[2])
    scale = 0.6 * (hi[2] - lo[2]) / zoom
    fx = lo[0] - 0.1 * scale + scale * width / (width * 9 // 16)
    view.CameraFocalPoint = [fx, c[1], c[2]]
    view.CameraPosition = [fx, c[1] - 1.2 * L, c[2] + 0.35 * L]
    view.CameraViewUp = [0, 0, 1]
    view.GetActiveCamera().SetParallelScale(scale)
    if rays:
        view.EnableRayTracing = 1
        view.BackEnd = "OSPRay pathtracer"
        view.SamplesPerPixel = rays
        view.Denoise = 1
        view.UseEnvironmentLighting = 1
        view.EnvironmentalBG = [1, 1, 1]
        view.BackgroundColorMode = "Single Color"
    Render(view)
    SaveScreenshot("%s.%s.png" % (base, tag), view)
    Delete(sd)
    for p in (surfn, smooth, surfs, surf):
        Delete(p)
