#include "grid/octree.h"
#include "embed.h"
#include "distance.h"
#include "navier-stokes/centered.h"
static const double d = 0.40;
static double Reynolds = 10000;
static int maxlevel = 4;

u.n[left] = dirichlet(1);
p[left] = neumann(0);
pf[left] = neumann(0);
u.n[right] = neumann(0);
p[right] = dirichlet(0);
pf[right] = dirichlet(0);
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
  L0 = 1.0;
  N = 64;
  origin(-L0/2, -L0/2, -L0/2);
  mu = muv;
  run();
}
event properties(i++) { foreach_face() muv.x[] = fm.x[] * d / Reynolds; }

event init(t = 0) {
  vertex scalar phi[];
  FILE * fp = fopen ("geom/scaled.stl", "r");
  coord * p = input_stl (fp);
  coord min, max;
  bounding_box (p, &min, &max);
  fprintf(stderr, "distance: min: %g %g %g\n", min.x, min.y, min.z);
  fprintf(stderr, "distance: max: %g %g %g\n", max.x, max.y, max.z);
  fclose(fp)
  
  
  solid(cs, fs, difference(difference(x * x + y * y - sq(d/2), z - 0.4), -0.4 - z));
  foreach ()
    u.x[] = cs[] ? 1. : 0.;
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
