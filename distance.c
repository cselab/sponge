#include "grid/octree.h"
#include "utils.h"
#include "distance.h"
#include "fractions.h"
#include "view.h"

int main(int argc, char **argv)
{
  int Verbose, Refine, LevelFlag;
  long level;
  FILE *file;
  char *end;
  coord *p, min, max;

  Verbose = 0;
  Refine = 0;
  LevelFlag = 0;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(stderr, "distance [-v] [-r] -l INT file.stl\n");
      exit(1);
    case 'v':
      Verbose = 1;
      break;
    case 'r':
      Refine = 1;
      break;
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
    default:
      fprintf(stderr, "distance: unknown option '%s'\n", *argv);
      exit(1);
    }
  if (*argv == NULL) {
    fprintf(stderr, "distance: error: not input file\n");
    exit(1);
  }
  if (LevelFlag == 0) {
    fprintf(stderr, "distance: error: -l must be set\n");
    exit(1);
  }
  if ((file = fopen(*argv, "r")) == NULL) {
    fprintf(stderr, "distance: error: '%s': no suck file\n", *argv);
    exit(1);
  }
  p = input_stl(file);
  if (fclose(file) != 0) {
    fprintf(stderr, "distance: fail to close '%s'\n", *argv);
    exit(1);
  }
  bounding_box (p, &min, &max);
  double maxl = -HUGE;
  foreach_dimension()
    if (max.x - min.x > maxl)
      maxl = max.x - min.x;
  init_grid (1 << level);
  if (Verbose) {
    fprintf(stderr, "distance: min: %g %g %g\n", min.x, min.y, min.z);
    fprintf(stderr, "distance: max: %g %g %g\n", max.x, max.y, max.z);
    fprintf(stderr, "distance: init_grid: %d\n", N);
  }
  size (1.2*maxl);
  origin ((max.x + min.x)/2. - L0/2,
	  (max.y + min.y)/2. - L0/2,
	  (max.z + min.z)/2. - L0/2);
  scalar d[];
  distance (d, p);
  if (Refine)
    while (adapt_wavelet ({d}, (double[]){5e-4*L0}, 10).nf);
  view (fov = 15.65, quat = {-0.52,0.31,0.38,-0.7},
	tx = -0.045, ty = 0.015, width = 640, height = 480, bg = {1,1,1});
  isosurface ("d", 0, color = "level", min = 5, max = 10);
  save ("isosurface.png");
  scalar f[];
  face vector s[];
  solid (f, s, (d[] + d[-1] + d[0,-1] + d[-1,-1] +
		d[0,0,-1] + d[-1,0,-1] + d[0,-1,-1] + d[-1,-1,-1])/8.);
  clear();
  draw_vof ("f", "s");
  draw_vof ("f", "s", edges = true, lw = 0.5);
  save ("vof.png");
}
