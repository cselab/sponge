from paraview.simple import *

stl_reader = STLReader(FileNames=["ver.stl"])
render_view = CreateRenderView()
stl_display = Show(stl_reader, render_view)
render_view.ViewSize = 1200, 800
render_view.CameraPosition = 54, 41, 142
render_view.CameraFocalPoint = -4, -7, 56
render_view.OrientationAxesVisibility = 0
SaveScreenshot("ver.png", render_view)

'''
for i in `seq 0 255`; do i=`printf %08d $i`; (cd $i; PYTHONPATH=. python3 ../gen.py && pvbatch ../vis.py && scp ver.png eth:homepage/sponge/ver.$i.png); done
'''
