#include "grid/octree.h"
#include "navier-stokes/centered.h"
#include "two-phase.h"
#include "navier-stokes/conserving.h"
#include "distance.h"
#include "reduced.h"
#include "view.h"
#include "lambda2.h"
#include "maxruntime.h"

void fraction_from_stl (scalar f, FILE * fp, double eps, int maxlevel)
{
  coord * p = input_stl (fp);
  coord min, max;
  bounding_box (p, &min, &max);
  double maxl = -HUGE;
  foreach_dimension()
    if (max.x - min.x > maxl)
      maxl = max.x - min.x;
  scalar d[];
  distance (d, p);
  while (adapt_wavelet ({d}, (double[]){eps*maxl}, maxlevel, 5).nf);
  vertex scalar phi[];
  foreach_vertex()
    phi[] = (d[] + d[-1] + d[0,-1] + d[-1,-1] +
	     d[0,0,-1] + d[-1,0,-1] + d[0,-1,-1] + d[-1,-1,-1])/8.;
  fractions (phi, f);
}

int LEVEL = 9;
double FROUDE = 0.4;
scalar tangaroa[], f0[];
int main (int argc, char * argv[])
{
  maxruntime (&argc, argv);
  if (argc > 1)
    LEVEL = atoi(argv[1]);
  if (argc > 2)
    FROUDE = atof(argv[2]);
    
  init_grid (32);
  rho1 = 1.; // water
  rho2 = 1./815.; // air
  size (5.);
  origin (-L0/2.,-L0/3.,-L0/2.);
  for (scalar s in {tangaroa,f0})
    s.refine = s.prolongation = fraction_refine;
  G.z = - 1./sq(FROUDE);
  run();
}

u.n[bottom] = dirichlet(1);
p[bottom]   = neumann(0.);
pf[bottom]  = neumann(0.);
f[bottom]   = f0[];
u.n[top]  = neumann(0.);
p[top]    = dirichlet(0.);
pf[top]   = dirichlet(0.);
tangaroa[back] = 0;
f[back] = 1;
uf.n[left] = 0.;
uf.n[right] = 0.;
	      
event init (t = 0) {
  if (!restore (file = "restart")) {
    FILE * fp;
    if ((fp = fopen ("tangaroa.stl", "r")) == NULL) {
      fprintf(stderr, "tangaroa: fail to open stl file\n");
      exit(1);
    }
    fraction_from_stl (tangaroa, fp, 5e-4, LEVEL);
    fclose (fp);
    fraction (f0, - z);
    foreach() {
      f[] = f0[];
      u.y[] = 1.;
    }
  }

  display ("view (quat = {0.542,0.150,0.209,0.799},"
	   "      fov = 30, near = 0.01, far = 1000,"
	   "      tx = -0.0074, ty = -0.0138, tz = -0.3211);"
	   "draw_vof (c = 'f', color = 'z', min = -0.1, max = 0.1, "
	   "          linear = true, fc = {0.45,0.78,0.92});"
	   "draw_vof (c = 'tangaroa');", true);
}

event velocity (i++) {
  foreach()
    foreach_dimension()
      u.x[] = (1. - tangaroa[])*u.x[];
}
	      
event movie (t += 0.01; t <= 10)
{
  view (fov = 5.86528,
	quat = {0.515965,0.140691,0.245247,0.808605},
	tx = -0.07438, ty = -0.0612925,
	width = 1024, height = 768);

  clear();
  draw_vof ("tangaroa");
  scalar Z[];
  Z[back] = dirichlet (z);
  foreach()
    Z[] = z;
  draw_vof ("f", color = "Z", min = -0.1, max = 0.1, linear = true);
  save ("movie.mp4");

  draw_vof ("tangaroa", fc = {0.5,0.5,0.5});
  draw_vof ("f", color = "Z", min = -0.1, max = 0.1, linear = true);
  scalar l2[];
  lambda2 (u, l2);
  isosurface ("l2", -100);
  save ("l2.mp4");
}

event snapshot (i += 100)
{
  scalar l2[];
  lambda2 (u, l2);
  dump (file = "restart");
}

event adapt (i++) {
  double uemax = 0.1;
  adapt_wavelet ({f,tangaroa,u},
		 (double[]){0.01,0.01,uemax,uemax,uemax}, LEVEL, 5);
}
