#include "grid/octree.h"
#include "navier-stokes/centered.h"
#include "fractions.h"
#include "two-phase.h"
#include "distance.h"
#include "view.h"

static const double diameter = 0.3733333285861546;
static double Reynolds = 400;
static int maxlevel = 8;
static char *stl_path;
static int period;

u.n[left] = dirichlet(1);
p[left] = neumann(0);
pf[left] = neumann(0);
u.n[right] = neumann(0);
p[right] = dirichlet(0);
pf[right] = dirichlet(0);
face vector muv[];
scalar stl[];

static int dump_fields(const char *raw, const char *xdmf, double t, double ox,
		double oy, double ex, double ey, long nx) {
  long k, j, ny, nfield;
  double sx, sy, xp, yp, zp;
  float v;
  FILE *fp;
  char *names[] = {"ux", "uy", "uz", "p"};
  zp = 0;
  sx = ex / nx;
  ny = ey / sx;
  sy = ey / ny;
  if ((fp = fopen(raw, "w")) == NULL) {
    fprintf(stderr, "stl: fail to write to '%s'\n", raw);
    exit(1);
  }
  for (k = 0; k < ny; k++) {
    yp = oy + sy * k + sy / 2.;
    for (j = 0; j < nx; j++) {
      xp = ox + sx * j + sx / 2;
#define FIELD(f)				\
      v = interpolate((f), xp, yp, zp);		\
      fwrite(&v, sizeof v, 1, fp);		\

      FIELD(u.x);
      FIELD(u.y);
      FIELD(u.z);
      FIELD(p);
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
  nfield = sizeof names / sizeof *names;
  fprintf(fp, "\
<Xdmf Version=\"2.0\">\n\
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
  for (j = 0; j < nfield; j++)
    fprintf(fp, "\
     <Attribute Name=\"%s\" Center=\"Cell\">\n\
	<DataItem ItemType=\"HyperSlab\" Dimensions=\"%ld %ld\">\n\
	  <DataItem Dimensions=\"3 2\">0 %ld 1 %ld %ld %ld</DataItem>\n\
	  <DataItem Dimensions=\"%ld %ld\" Format=\"Binary\">%s</DataItem>\n\
	</DataItem>\n\
     </Attribute>\n\
",
	    names[j], ny, nx, j, nfield, ny, nx, ny,
	    nfield * nx, raw);
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
  int PeriodFlag;
  long level;
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
  foreach () {
    u.x[] = 0;
    u.y[] = 0;
    u.z[] = 0;
  }
  view (fov = 20, width = 640, height = 480, bg = {1,1,1});
  draw_vof ("stl", "s");
  draw_vof ("stl", "s", edges = true, lw = 0.5);
  save ("stl.png");
}

event velocity (i++) {
  foreach()
    foreach_dimension()
      u.x[] = stl[]*u.x[];
}

event logfile(i += 10) { fprintf(stderr, "%d %g %d %d\n", i, t, mgp.i, mgu.i); }

event dump(i ++; t <= 100) {
  static long iframe = 0;
  scalar omega[];
  char raw[FILENAME_MAX], xdmf[FILENAME_MAX], omega_path[FILENAME_MAX];
  if (iframe % period == 0) {
    sprintf(xdmf, "a.%09ld.xdmf2", iframe);
    sprintf(raw, "%09ld.raw", iframe);
    sprintf(omega_path, "omega.%09ld.png", iframe);
    vorticity(u, omega);
    draw_vof ("stl", "s");
    draw_vof ("stl", "s", edges = true, lw = 0.5);
    isosurface("u.x", 0.5);
    save(omega_path);
    if (dump_fields(raw, xdmf, t, X0, Y0, L0, L0, N) != 0) {
      fprintf(stderr, "stl: error:dump_fields failed\n");
      exit(1);
    }
  }
  iframe++;
}

event adapt (i++) {
  double uemax = 0.1;
  astats s = adapt_wavelet ({stl, u},
			    (double[]){0.01,0.01,uemax,uemax,uemax}, maxlevel, 5);
  fprintf(stderr, "stl: %g refined %d cells, coarsened %d cells\n",
	   t, s.nf, s.nc);
}
