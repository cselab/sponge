from paraview.simple import *
import json
import math
import numpy as np
import os
import re
import sys

if len(sys.argv) < 4:
    sys.stderr.write(
        "usage: pvpython pvmulti.py [-z ZOOM] [-w WIDTH] [-H HEIGHT] [-r SAMPLES] [-a]\n"
        "                           [-c PERCENTILE] [-m MAX] [-A AZ,EL]\n"
        "                           center.stl TAG:OPACITY[:MODE],... FILE.xdmf2 [FILE.xdmf2 ...]\n"
        "       TAG is a mesh written by iso.py: omega1, q5, l21, ...\n"
        "       MODE c colour by omega_z (default), g grey, w white, b pale blue,\n"
        "            l lavender, s steel blue, p pink, or R/G/B in 0..1\n"
        "       -r SAMPLES  OSPRay path tracer, SAMPLES per pixel, denoised\n"
        "       -g THICKNESS   render the shells as thin glass tinted by their colour\n"
        "       -k F,B,H,AZ,EL light kit: key to fill, back and head ratios and\n"
        "                      key direction in degrees\n"
        "       -l SCALE[,ENV[,KEY]]  light scale, environment lighting 0/1, key\n"
        "                      light intensity\n"
        "       -a          screen space ambient occlusion (rasterizer only)\n"
        "       -c PERCENTILE  colour range percentile of |omega_z| (default 99)\n"
        "       -m MAX      fixed colour range instead of the percentile\n"
        "       -w WIDTH -H HEIGHT  image size, default WIDTH 1920 and 16:9\n"
        "       -A AZ,EL    camera azimuth and elevation in degrees, default 0,20\n"
        "       writes FILE.multi.png\n"
        "example: pvmulti.py -r 32 center.stl q1:0.3:b,q5:1.0:p h.[0-9]*.xdmf2\n")
    sys.exit(1)
args = sys.argv[1:]
zoom = 1.0
width = 1920
rays = 0
ssao = 0
pct = 99.0
glass = 0.0
cmax = 0.0
height = 0
az = None
el = None
kit = None
light = None
materials = {}
while args[0].startswith("-"):
    if args[0] == "-z":
        zoom = float(args[1])
        args = args[2:]
    elif args[0] == "-w":
        width = int(args[1])
        args = args[2:]
    elif args[0] == "-r":
        rays = int(args[1])
        args = args[2:]
    elif args[0] == "-c":
        pct = float(args[1])
        args = args[2:]
    elif args[0] == "-g":
        glass = float(args[1])
        args = args[2:]
    elif args[0] == "-k":
        kit = [float(x) for x in args[1].split(",")]
        args = args[2:]
    elif args[0] == "-l":
        light = [float(x) for x in args[1].split(",")]
        args = args[2:]
    elif args[0] == "-H":
        height = int(args[1])
        args = args[2:]
    elif args[0] == "-A":
        az, el = [float(x) for x in args[1].split(",")]
        args = args[2:]
    elif args[0] == "-m":
        cmax = float(args[1])
        args = args[2:]
    elif args[0] == "-a":
        ssao = 1
        args = args[1:]
    else:
        sys.exit("pvmulti.py: unknown option %s" % args[0])
stl = args[0]
surf = [s.split(":") for s in args[1].split(",")]
paths = [p for p in args[2:]
         if not re.search(r"\.(omega|l2|q)[0-9.e+-]+\.xdmf2$", p)]
colors = {"g": [0.72, 0.75, 0.80], "w": [0.95, 0.96, 1.0], "b": [0.72, 0.78, 0.96],
          "l": [0.68, 0.66, 0.92], "s": [0.50, 0.56, 0.76], "p": [0.86, 0.42, 0.50]}

m = 0.0

for path in paths:
    base = os.path.splitext(path)[0]
    for spec in surf:
        name = "%s.%s" % (base, spec[0])
        if len(spec) < 3 or spec[2] == "c":
            a = np.fromfile(name + ".attr.raw", np.float32)
            if a.size:
                m = max(m, float(np.percentile(np.abs(a), pct)))
if cmax:
    m = cmax
if m == 0:
    m = 1.0

view = GetActiveViewOrCreate("RenderView")
view.ViewSize = [width, height if height else width * 9 // 16]
view.Background = [1, 1, 1]
view.OrientationAxesVisibility = 0
view.UseColorPaletteForBackground = 0
if ssao and not rays:
    for k, v in [("UseAmbientOcclusion", 1), ("UseSSAODefaultPresets", 1),
                 ("AmbientSamples", 8), ("Shadows", 0)]:
        if hasattr(view, k):
            setattr(view, k, v)
body = STLReader(FileNames=[stl])
bodyn = SurfaceNormals(Input=body)
bodyn.FeatureAngle = 80
bd = Show(bodyn, view)
bd.ColorArrayName = ["POINTS", ""]
bd.DiffuseColor = [0.28, 0.27, 0.32]
bd.Specular = 0.0 if (ssao or rays) else 0.3
bodyn.UpdatePipeline()
b = bodyn.GetDataInformation().GetBounds()
lo = np.array(b[0::2], float)
hi = np.array(b[1::2], float)
H = height if height else width * 9 // 16
hi[0] = max(hi[0], lo[0] + (hi[2] - lo[2]) * width / H)
c = (lo + hi) / 2
L = float((hi - lo).max())

lut = GetColorTransferFunction("u")
lut.ApplyPreset("Cool to Warm", True)
lut.AutomaticRescaleRangeMode = "Never"
lut.RescaleTransferFunction(-m, m)
for path in paths:
    base = os.path.splitext(path)[0]
    keep = []
    for spec in surf:
        tag = spec[0]
        op = float(spec[1])
        mode = spec[2] if len(spec) > 2 else "c"
        r = XDMFReader(FileNames=["%s.%s.xdmf2" % (base, tag)])
        r.UpdatePipeline()
        e = ExtractSurface(Input=r)
        sm = Smooth(Input=e)
        sm.NumberofIterations = 15
        n = SurfaceNormals(Input=sm)
        n.FeatureAngle = 80
        d = Show(n, view)
        if mode == "c":
            ColorBy(d, ("POINTS", "u"))
            d.LookupTable = lut
        else:
            d.ColorArrayName = ["POINTS", ""]
            d.DiffuseColor = [float(x) for x in mode.split("/")] if "/" in mode else colors[mode]
        d.Opacity = op
        d.Specular = 0.0 if (ssao or rays) else 0.2
        if glass and rays and mode != "c":
            name = "glass_" + tag
            materials[name] = {
                "type": "ThinGlass",
                "doubles": {"attenuationColor": list(d.DiffuseColor),
                            "attenuationDistance": [1.0],
                            "thickness": [glass],
                            "eta": [1.5]}}
            d.OSPRayMaterial = name
            d.Opacity = 1.0
        keep += [d, n, sm, e, r]
    Render(view)
    view.CameraParallelProjection = 1
    view.ResetCamera(lo[0], hi[0], lo[1], hi[1], lo[2], hi[2])
    scale = 0.6 * (hi[2] - lo[2]) / zoom
    fx = lo[0] - 0.1 * scale + scale * width / H
    view.CameraFocalPoint = [fx, c[1], c[2]]
    if az is None:
        view.CameraPosition = [fx, c[1] - 1.2 * L, c[2] + 0.35 * L]
    else:
        a = math.radians(az)
        e = math.radians(el)
        view.CameraPosition = [fx + 1.6 * L * math.cos(e) * math.sin(a),
                               c[1] - 1.6 * L * math.cos(e) * math.cos(a),
                               c[2] + 1.6 * L * math.sin(e)]
    view.CameraViewUp = [0, 0, 1]
    view.GetActiveCamera().SetParallelScale(scale)
    if rays:
        if materials:
            mpath = base + ".materials.json"
            json.dump({"family": "OSPRay", "version": "0.0",
                       "materials": materials}, open(mpath, "w"))
            GetMaterialLibrary().LoadMaterials = mpath
        view.EnableRayTracing = 1
        view.BackEnd = "OSPRay pathtracer"
        view.SamplesPerPixel = rays
        view.Denoise = 1
        view.UseEnvironmentLighting = int(light[1]) if light and len(light) > 1 else 1
        view.EnvironmentalBG = [1, 1, 1]
        view.BackgroundColorMode = "Single Color"
        if light:
            view.LightScale = light[0]
            if len(light) > 2:
                view.KeyLightIntensity = light[2]
        if kit:
            view.FillLightKFRatio = kit[0]
            view.BackLightKBRatio = kit[1]
            view.HeadLightKHRatio = kit[2]
            view.KeyLightAzimuth = kit[3]
            view.KeyLightElevation = kit[4]
    Render(view)
    SaveScreenshot(base + ".multi.png", view)
    for p in keep:
        Delete(p)
