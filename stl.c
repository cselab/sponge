#include "grid/octree.h"
#include "navier-stokes/centered.h"
#include "fractions.h"
#include "two-phase.h"
#include "distance.h"
#include "embed.h"
#include "view.h"
#include <mpi.h>
#include <stdint.h>
#include <stdbool.h>
#include "output_htg.h"

static const double diameter = 0.3733333285861546;
static double Reynolds = 400;
static int maxlevel = 7;
static char *stl_path;
static int period;
static long level;
vertex scalar phi[];
scalar omega[];

u.n[left] = dirichlet(1);
p[left] = neumann(0);
pf[left]   = neumann(0.);

u.n[right] = neumann(0);
p[right] = dirichlet(0);
pf[right]  = dirichlet(0.);

u.n[embed] = dirichlet(0.);
u.t[embed] = dirichlet(0.);
u.r[embed] = dirichlet(0.);
face vector muv[];

int main(int argc, char **argv) {
  int LevelFlag;
  int PeriodFlag;
  char *end;
  LevelFlag = 0;
  PeriodFlag = 0;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      exit(1);
    case 'l':
      argv++;
      if (*argv == NULL) {
	fprintf(stderr, "distance: -l needs an argument\n");
	exit(1);
      }
      level = strtol(*argv, &end, 10);
      if (*end != '\0' || level <= 0) {
	fprintf(stderr, "distance: '%s' is not a positive integer\n", *argv);
	exit(1);
      }
      LevelFlag = 1;
      break;
    case 'p':
      argv++;
      if (*argv == NULL) {
	fprintf(stderr, "cylinder: -p needs an argument\n");
	exit(1);
      }
      period = strtol(*argv, &end, 10);
      if (*end != '\0' || period <= 0) {
	fprintf(stderr, "cylinder: '%s' is not a positive integer\n", *argv);
	exit(1);
      }
      PeriodFlag = 1;
      break;
    default:
      fprintf(stderr, "stl: error: unrecognized command-line option '%s'\n", *argv);
      exit(1);
    }
  if (LevelFlag == 0) {
    fprintf(stderr, "distance: error: -l must be set\n");
    exit(1);
  }
  if (!PeriodFlag) {
    fprintf(stderr, "distance: error: -p must be set\n");
    exit(1);
  }
  if (*argv == NULL) {
    fprintf(stderr, "stl: error: not input file\n");
    exit(1);
  }
  stl_path = *argv;
  size(1.0);
  init_grid (1 << level);
  origin(-L0/2, -L0/2, -L0/2);
  mu = muv;
  run();
}
event properties(i++) {
  foreach_face()
    muv.x[] = fm.x[] * diameter / Reynolds;
}

event init(t = 0) {
  coord min, max;
  coord * p;
  FILE * fp;
  scalar d[];
  if (!restore (file = "restart")) {
    if ((fp = fopen (stl_path, "r")) == NULL) {
      fprintf(stderr, "stl: error: fail to open '%s'\n", stl_path);
      exit(1);
    }
    p = input_stl (fp);
    if (fclose(fp) != 0) {
      fprintf(stderr, "stl: fail to close '%s'\n", stl_path);
      exit(1);
    }
    bounding_box(p, &min, &max);
    fprintf(stderr, "stl: min: %g %g %g\n", min.x, min.y, min.z);
    fprintf(stderr, "stl: max: %g %g %g\n", max.x, max.y, max.z);
    distance (d, p);
    foreach_vertex() {
      double p0, s0;
      /* s0 = (d[] + d[-1] + d[0,-1] + d[-1,-1] +
	 d[0,0,-1] + d[-1,0,-1] + d[0,-1,-1] + d[-1,-1,-1])/8.;
	 p0 = min(-s0, sq(x) + sq(y) - sq(diameter / 2)); */
      p0 = sq(x) + sq(y) - sq(diameter / 2);
      p0 = max(p0, z - 0.4);
      p0 = max(p0, - 0.4 - z);
      phi[] = p0;
    }
    fractions (phi, cs, fs);
    fractions_cleanup (cs, fs);
  }
  
  view (fov = 20, width = 640, height = 480, bg = {1,1,1});
  draw_vof ("cs", "fs");
  draw_vof ("cs", "fs", edges = true, lw = 0.5);
  save ("vof.png");
}

/*
event velocity (i++) {
  foreach()
    foreach_dimension()
      u.x[] = stl[]*u.x[];
}
*/

event dump(i ++; t <= 100) {
  static long iframe = 0;
  char hdg[FILENAME_MAX];
  char path[]=".";
  if (iframe % period == 0) {
    fields_stats();
    vorticity(u, omega);
    sprintf(hdg, "h.%09ld", iframe);
    output_htg({p, cs, phi, omega}, {u}, path, hdg, iframe, t);
  }
  iframe++;
}

/*
event adapt (i++) {
  astats s = adapt_wavelet ({cs,u}, (double[]){1e-2, 0.02, 0.02, 0.02},
			    maxlevel, 4);
  fprintf (stderr, "# refined %d cells, coarsened %d cells\n", s.nf, s.nc);
  fields_stats();
}
*/
