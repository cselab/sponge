#include "grid/octree.h"
#include "embed.h"
#include "navier-stokes/centered.h"
#include "view.h"
static const double d = 0.25;
static double Reynolds = 100;
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
    fprintf(stderr, "cylinder: fail to write to '%s'\n", raw);
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
    fprintf(stderr, "cylinder: fail to close '%s'\n", raw);
    return 1;
  }
  if ((fp = fopen(xdmf, "w")) == NULL) {
    fprintf(stderr, "cylinder: fail to write to '%s'\n", xdmf);
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
    fprintf(stderr, "cylinder: fail to close '%s'\n", xdmf);
    return 1;
  }
  return 0;
}

int main(int argc, char **argv) {
  L0 = 1.0;
  origin(-L0/2, -L0/2, -L0/2);
  N = 64;
  mu = muv;
  run();
}
event properties(i++) { foreach_face() muv.x[] = fm.x[] * d / Reynolds; }

event init(t = 0) {
  vertex scalar phi[];
  solid(cs, fs, x * x + y * y - sq(d/2));
}

event logfile(i += 10) { fprintf(stderr, "%d %g %d %d\n", i, t, mgp.i, mgu.i); }

event movies(i += 1; t <= 100) {
  static long iframe = 0;
  char path[FILENAME_MAX], raw[FILENAME_MAX], xdmf[FILENAME_MAX];
  view(fov = 11, theta = 0.05, relative = false);
  isosurface("u.x", 0.5, color = "level");
  sprintf(path, "%09ld.png", iframe);
  save(path);
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

