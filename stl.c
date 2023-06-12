#include "grid/octree.h"
#include "navier-stokes/centered.h"
#include "fractions.h"
#include "two-phase.h"
#include "distance.h"
#include "view.h"

static const double diameter = 0.3733333285861546;
static double Reynolds = 10000;
static int maxlevel = 4;
static char *stl_path;

scalar stl[];
u.n[left] = dirichlet(1);
p[left] = neumann(0);
pf[left] = neumann(0);
u.n[right] = neumann(0);
p[right] = dirichlet(0);
pf[right] = dirichlet(0);
//stl[back] = 0;
face vector muv[];

static int dump_fields(const char *raw, const char *xdmf, double t, double ox,
		double oy, double ex, double ey, long nx) {
  long k, j, ny;
  double sx, sy, xp, yp, zp;
  float v;
  FILE *fp;
  char *names[] = {"ux", "uy", "p"};
  zp = 0;
  sx = ex / nx;
  ny = ey / sx;
  sy = ey / ny;
  if ((fp = fopen(raw, "w")) == NULL) {
    fprintf(stderr, "3: fail to write to '%s'\n", raw);
    exit(1);
  }
  for (k = 0; k < ny; k++) {
    yp = oy + sy * k + sy / 2.;
    for (j = 0; j < nx; j++) {
      xp = ox + sx * j + sx / 2;
      v = interpolate(u.x, xp, yp, zp);
      fwrite(&v, sizeof v, 1, fp);
      v = interpolate(u.y, xp, yp, zp);
      fwrite(&v, sizeof v, 1, fp);
      v = interpolate(p, xp, yp, zp);
      fwrite(&v, sizeof v, 1, fp);
    }
  }
  if (fclose(fp) != 0) {
    fprintf(stderr, "3: fail to close '%s'\n", raw);
    return 1;
  }
  if ((fp = fopen(xdmf, "w")) == NULL) {
    fprintf(stderr, "3: fail to write to '%s'\n", xdmf);
    return 1;
  }
  fprintf(fp, "\
<Xdmf>\n\
 <Domain>\n\
   <Grid>\n\
     <Time Value=\"%.16e\"/>\n\
     <Topology TopologyType=\"2DCORECTMesh\" Dimensions=\"%ld %ld\"/>\n\
     <Geometry GeometryType=\"ORIGIN_DXDY\">\n\
       <DataItem Name=\"Origin\" Dimensions=\"2\">%.16e %.16e</DataItem>\n\
       <DataItem Name=\"Spacing\" Dimensions=\"2\">%.16e %.16e</DataItem>\n\
     </Geometry>\n\
",
	  t, ny + 1, nx + 1, oy, ox, sy, sx);
  for (j = 0; j < sizeof names / sizeof *names; j++)
    fprintf(fp, "\
     <Attribute Name=\"%s\" Center=\"Cell\">\n\
	<DataItem ItemType=\"HyperSlab\" Dimensions=\"%ld %ld\">\n\
	  <DataItem Dimensions=\"3 2\">0 %ld 1 %ld %ld %ld</DataItem>\n\
	  <DataItem Dimensions=\"%ld %ld\" Format=\"Binary\">%s</DataItem>\n\
	</DataItem>\n\
     </Attribute>\n\
",
	    names[j], ny, nx, j, sizeof names / sizeof *names, ny, nx, ny,
	    3 * nx, raw);
  fprintf(fp, "\
   </Grid>\n\
 </Domain>\n\
</Xdmf>\n\
");
  if (fclose(fp) != 0) {
    fprintf(stderr, "3: fail to close '%s'\n", xdmf);
    return 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  int LevelFlag;
  long level;
  char *end;
  LevelFlag = 0;
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
    default:
      fprintf(stderr, "stl: error: unrecognized command-line option '%s'\n", *argv);
      exit(1);
    }
  if (LevelFlag == 0) {
    fprintf(stderr, "distance: error: -l must be set\n");
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
event properties(i++) { foreach_face() muv.x[] = fm.x[] * diameter / Reynolds; }

event init(t = 0) {
  coord min, max;
  coord * p;
  FILE * fp;
  vertex scalar phi[];
  scalar d[];

  if ((fp = fopen (stl_path, "r")) == NULL) {
    fprintf(stderr, "stl: error: fail to open '%s'\n", stl_path);
    exit(1);
  }
  p = input_stl (fp);
  fclose(fp);
  bounding_box (p, &min, &max);
  fprintf(stderr, "stl: min: %g %g %g\n", min.x, min.y, min.z);
  fprintf(stderr, "stl: max: %g %g %g\n", max.x, max.y, max.z);
  distance (d, p);
  foreach_vertex() {
    double p0, s0;
    s0 = (d[] + d[-1] + d[0,-1] + d[-1,-1] +
	  d[0,0,-1] + d[-1,0,-1] + d[0,-1,-1] + d[-1,-1,-1])/8.;
    p0 = min(-s0, sq(x) + sq(y) - sq(diameter / 2));
    p0 = max(p0, z - 0.4);
    p0 = max(p0, - 0.4 - z);
    phi[] = p0;
  }
  fractions (phi, stl);
  foreach ()
    u.x[] = stl[] ? 1. : 0.;

  view (fov = 20, quat = {-0.52,0.31,0.38,-0.7},
	tx = -0.045, ty = 0.015, width = 640, height = 480, bg = {1,1,1});
  draw_vof ("stl", "s");
  draw_vof ("stl", "s", edges = true, lw = 0.5);
  save ("stl.png");
  exit(0);
}

event velocity (i++) {
  foreach()
    foreach_dimension()
      u.x[] = (1. - stl[])*u.x[];
}

event logfile(i += 10) { fprintf(stderr, "%d %g %d %d\n", i, t, mgp.i, mgu.i); }

event movies(i += 1; t <= 100) {
  static long iframe = 0;
  char raw[FILENAME_MAX], xdmf[FILENAME_MAX];
  sprintf(xdmf, "a.%09ld.xdmf2", iframe);
  sprintf(raw, "%09ld.raw", iframe);
  if (dump_fields(raw, xdmf, t, X0, Y0, L0, L0, N) != 0)
      exit(1);
  iframe++;
}

/*
event adapt(i++) {
  adapt_wavelet({cs, u}, (double[]){1e-2, 3e-3, 3e-3}, maxlevel, 4);
}
*/
