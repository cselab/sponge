@include <float.h>
@include <math.h>
@include <stdbool.h>
@include <stdint.h>
@include <stdlib.h>
@include <string.h>
#include "grid/octree.h"
#include "fractions.h"
#include "navier-stokes/centered.h"
#include "lambda2.h"
#include "output_xdmf.h"

#if 0
#include "embed.h"
trace static double embed_interpolate3(Point point, scalar s, coord p) {
  int i = sign(p.x), j = sign(p.y), k = sign(p.z);
  if (cs[i, 0, 0] && cs[0, j, 0] && cs[i, j, 0] && cs[0, 0, k] && cs[i, 0, k] &&
      cs[0, j, k] && cs[i, j, k]) {
    double val_0, val_k;
    val_0 =
	(s[0, 0, 0] * (1. - fabs(p.x)) + s[i, 0, 0] * fabs(p.x)) *
	    (1. - fabs(p.y)) +
	(s[0, j, 0] * (1. - fabs(p.x)) + s[i, j, 0] * fabs(p.x)) * fabs(p.y);
    val_k =
	(s[0, 0, k] * (1. - fabs(p.x)) + s[i, 0, k] * fabs(p.x)) *
	    (1. - fabs(p.y)) +
	(s[0, j, k] * (1. - fabs(p.x)) + s[i, j, k] * fabs(p.x)) * fabs(p.y);
    return (val_0 * (1. - fabs(p.z)) + val_k * fabs(p.z));
  } else {
    double val = s[];
    foreach_dimension() {
      int i = sign(p.x);
      if (cs[i])
	val += fabs(p.x) * (s[i] - s[]);
      else if (cs[-i])
	val += fabs(p.x) * (s[] - s[-i]);
    }
    return val;
  }
}

trace static void embed_force3(scalar p, vector u, face vector mu, coord *Fp,
			       coord *Fmu) {
  coord Fps = {0}, Fmus = {0};
  foreach (reduction(+ : Fps) reduction(+ : Fmus)) {
    if (cs[] > 0. && cs[] < 1.) {
      coord n, b;
      double area = embed_geometry(point, &b, &n);
      area *= pow(Delta, dimension - 1);
      double Fn = area * embed_interpolate3(point, p, b);
      foreach_dimension() Fps.x += Fn * n.x;
      if (constant(mu.x) != 0.) {
	double mua = 0., fa = 0.;
	foreach_dimension() {
	  mua += mu.x[] + mu.x[1];
	  fa += fs.x[] + fs.x[1];
	}
	mua /= fa;
	coord dudn = embed_gradient(point, u, b, n);
	foreach_dimension() Fmus.x -=
	    area * mua *
	    (dudn.x * (sq(n.x) + 1.) + dudn.y * n.x * n.y + dudn.z * n.x * n.z);
      }
    }
  }

  Fp->x = Fps.x;
  Fp->y = Fps.y;
  Fp->z = Fps.z;

  Fmu->x = Fmus.x;
  Fmu->y = Fmus.y;
  Fmu->z = Fmus.z;
}
u.n[embed] = dirichlet(0);
u.t[embed] = dirichlet(0);
#else

coord Force = {0};
scalar cs[];
face vector fs[];
trace static void embed_force3(scalar p, vector u, face vector mu, coord *Fp,
                               coord *Fmu) {
  foreach_dimension() {
    Fmu->x = Force.x;
    Fp->x = 0;
  }
}
struct Cleanup {
  scalar c;
  face vector s;
  double smin;
  bool opposite;
};
trace static int fractions_cleanup(struct Cleanup u) { return 0; }
event velocity(i++) {
  foreach_dimension() Force.x = 0;
  foreach (reduction(+ : Force)) {
    foreach_dimension() {
      u.x[] = cs[] * u.x[];
      Force.x += u.x[] * (cs[] - 1) * Delta * Delta * Delta;
    }
  }
  foreach_dimension() Force.x /= dt;
}
#endif

static double dot3(const double *a, const double *b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static void vorticity_vector(const vector u, vector omega) {
  foreach () {
    double delta;
    delta = (2. * cm[] * Delta + SEPS);
    double fx[3] = {fm.x[1] - fm.x[], fm.x[1], -fm.x[]};
    double fy[3] = {fm.y[0, 1] - fm.y[], fm.y[0, 1], -fm.y[]};
    double fz[3] = {fm.z[0, 0, 1] - fm.z[], fm.z[0, 0, 1], -fm.z[]};
    double xy[3] = {u.x[], u.x[0, 1], u.x[0, -1]};
    double xz[3] = {u.x[], u.x[0, 0, 1], u.x[0, 0, -1]};
    double yx[3] = {u.y[], u.y[1], u.y[-1]};
    double yz[3] = {u.y[], u.y[0, 0, 1], u.y[0, 0, -1]};
    double zx[3] = {u.z[], u.z[1], u.z[-1]};
    double zy[3] = {u.z[], u.z[0, 1], u.z[0, -1]};
    omega.x[] = (dot3(fy, zy) - dot3(fz, yz)) / delta;
    omega.y[] = (dot3(fz, xz) - dot3(fx, zx)) / delta;
    omega.z[] = (dot3(fx, yx) - dot3(fy, xy)) / delta;
  }
}

static int slice(double x, double y, double z, double Delta) {
  double epsilon = Delta / 10;
  return z <= -epsilon && z + Delta + epsilon >= 0;
}
static double shape_cylinder(double x, double y, double z) {
  return sq(x) + sq(y) - sq(1.0 / 2);
}
static double shape_sphere(double x, double y, double z) {
  return sq(x) + sq(y) + sq(z) - sq(1.0 / 2);
}
static double (*Shape[])(double, double, double) = {shape_cylinder,
                                                    shape_sphere};
static const char *shape_names[] = {"cylinder", "sphere"};
static double (*shape)(double, double, double);

static const char *force_path, *output_prefix;
static char *dump_path;
static const int outlevel = 5;
static double reynolds, tend;
static int maxlevel, minlevel, period, Verbose, FullOutput, AdaptFlag;
static face vector muv[];
static scalar l2[];
static vector omega[];
static scalar phi[];

u.n[left] = dirichlet(1);
p[left] = neumann(0);
pf[left] = neumann(0);

u.n[right] = neumann(0);
p[right] = dirichlet(0);
pf[right] = dirichlet(0);

int main(int argc, char **argv) {
  char *end;
  const char *periodic_boundaries;
  int ReynoldsFlag, MaxLevelFlag, MinLevelFlag, PeriodFlag, TendFlag,
      DomainFlag, DTFlag, i;
  double domain, dt_min;
  AdaptFlag = 0;
  DomainFlag = 0;
  FullOutput = 0;
  MaxLevelFlag = 0;
  MinLevelFlag = 0;
  PeriodFlag = 0;
  ReynoldsFlag = 0;
  TendFlag = 0;
  DTFlag = 0;
  Verbose = 0;
  output_prefix = NULL;
  force_path = NULL;
  dump_path = NULL;
  shape = NULL;
  periodic_boundaries = NULL;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(
          stderr,
          "Usage: cylinder [-h] [-v] [-a] [-F] [-r <Reynolds number> "
          "-l <resolution level> -m <maximum resolution level> "
          "-o <prefix> -p <dump period> -e <end time> "
          "-f <force file> -S cylinder|sphere "
          "-z <domain size> [-b <boundaries>] [-d <dump file>]\n\n"
          "Options:\n"
          "  -h          Display this help message\n"
          "  -v          Verbose\n"
          "  -F          Output the full field\n"
          "  -a          Use adoptation\n"
          "  -b <string> Periodic boundary (ft|f|t: front (f), top (t) or both,"
          "default is symmetric boundary)\n"
          "  -r <num>    Reynolds number\n"
          "  -t <num>    dt\n"
          "  -l <num>    Minimum resolution level (positive integer)\n"
          "  -m <num>    Maximum resolution level (positive integer)\n"
          "  -o <string> Prefix for the output files\n"
          "  -p <num>    Dump period (positive integer)\n"
          "  -e <num>    End time of the simulation (decimal number)\n"
          "  -f <file>   Output force file\n"
          "  -S <string> Specify shape (cylinder|sphere)\n"
          "  -d <file>   Restart simulation from the dump file\n"
          "  -z <num>    Domain size\n\n"
	  "  npe: %d\n"
          "Example usage:\n"
          "  ./cylinder -v -r 100 -l 7 -m 10 -p 100 -e 2 -z 2.5 -S sphere\n"
          "  ./cylinder -v -r 100 -l 7 -m 10 -p 100 -e 2 -f force.dat -z 2.5 "
          "-S cylinder -o h -b t\n", npe());
      exit(1);
    case 'r':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "cylinder: error: -r needs an argument\n");
        exit(1);
      }
      reynolds = strtod(*argv, &end);
      if (*end != '\0') {
        fprintf(stderr, "cylinder: error: '%s' is not a number\n", *argv);
        exit(1);
      }
      ReynoldsFlag = 1;
      break;
    case 't':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "cylinder: error: -t needs an argument\n");
        exit(1);
      }
      dt_min = strtod(*argv, &end);
      if (*end != '\0') {
        fprintf(stderr, "cylinder: error: '%s' is not a number\n", *argv);
        exit(1);
      }
      DTFlag = 1;
      break;
    case 'm':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "cylinder: error: -m needs an argument\n");
        exit(1);
      }
      maxlevel = strtol(*argv, &end, 10);
      if (*end != '\0' || maxlevel <= 0) {
        fprintf(stderr, "cylinder: error: '%s' is not a positive integer\n",
                *argv);
        exit(1);
      }
      MaxLevelFlag = 1;
      break;
    case 'l':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "cylinder: error: -l needs an argument\n");
        exit(1);
      }
      minlevel = strtol(*argv, &end, 10);
      if (*end != '\0' || minlevel <= 0) {
        fprintf(stderr, "cylinder: error: '%s' is not a positive integer\n",
                *argv);
        exit(1);
      }
      MinLevelFlag = 1;
      break;
    case 'p':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "cylinder: error: -p needs an argument\n");
        exit(1);
      }
      period = strtol(*argv, &end, 10);
      if (*end != '\0' || period <= 0) {
        fprintf(stderr, "cylinder: error: '%s' is not a positive integer\n",
                *argv);
        exit(1);
      }
      PeriodFlag = 1;
      break;
    case 'v':
      Verbose = 1;
      break;
    case 'a':
      AdaptFlag = 1;
      break;
    case 'd':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "cylinder: error: -d needs an argument\n");
        exit(1);
      }
      dump_path = *argv;
      break;
    case 'e':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "cylinder: error: -e needs an argument\n");
        exit(1);
      }
      tend = strtod(*argv, &end);
      if (*end != '\0') {
        fprintf(stderr, "cylinder: error: '%s' is not a number\n", *argv);
        exit(1);
      }
      TendFlag = 1;
      break;
    case 'f':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "cylinder: error: -f needs an argument\n");
        exit(1);
      }
      force_path = *argv;
      break;
    case 'F':
      FullOutput = 1;
      break;
    case 'S':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "cylinder: error: -S needs an argument\n");
        exit(1);
      }
      for (i = 0; /**/; i++) {
        if (i == sizeof shape_names / sizeof *shape_names) {
          fprintf(stderr, "cylinder: error: unknown shape '%s'\n", *argv);
          exit(1);
        }
        if (strcmp(shape_names[i], *argv) == 0) {
          shape = Shape[i];
          break;
        }
      }
      break;
    case 'b':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "cylinder: error: -b needs an argument\n");
        exit(1);
      }
      periodic_boundaries = *argv;
      break;
    case 'o':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "cylinder: error: -o needs an argument\n");
        exit(1);
      }
      output_prefix = *argv;
      break;
    case 'z':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "cylinder: error: -z needs an argument\n");
        exit(1);
      }
      domain = strtod(*argv, &end);
      if (*end != '\0') {
        fprintf(stderr, "cylinder: error: '%s' is not a number\n", *argv);
        exit(1);
      }
      if (domain < 1) {
        fprintf(stderr,
                "cylinder: error: '%s': domain size (-z) is less then one\n",
                *argv);
        exit(1);
      }
      DomainFlag = 1;
      break;
    default:
      fprintf(stderr, "cylinder: error: unknown option '%s'\n", *argv);
      exit(1);
    }
  if (!ReynoldsFlag) {
    fprintf(stderr, "cylinder: error: -r is not set\n");
    exit(1);
  }
  if (!MinLevelFlag) {
    fprintf(stderr, "cylinder: error: -l is not set\n");
    exit(1);
  }
  if (!MaxLevelFlag) {
    fprintf(stderr, "cylinder: error: -m is not set\n");
    exit(1);
  }
  if (!PeriodFlag) {
    fprintf(stderr, "cylinder: error: -p is not set\n");
    exit(1);
  }
  if (!TendFlag) {
    fprintf(stderr, "cylinder: error: -e must be set\n");
    exit(1);
  }
  if (dump_path == NULL && !DomainFlag) {
    fprintf(stderr,
            "cylinder: error: eather -d (dump) -z (size) must be set\n");
    exit(1);
  }
  if (dump_path == NULL && shape == NULL) {
    fprintf(stderr,
            "cylinder: error: eather -d (dump) or -S (shape) must be set\n");
    exit(1);
  }
  if (Verbose && pid() == 0)
    fprintf(stderr, "cylinder: starting on %d ranks\n", npe());
  if (dump_path == NULL) {
    size(domain);
    origin(-L0 / 2.5, -L0 / 2.0, -L0 / 2.0);
  }
  mu = muv;
  if (periodic_boundaries != NULL)
    for (i = 0; periodic_boundaries[i] != '\0'; i++)
      switch (periodic_boundaries[i]) {
      case 'f':
        periodic(front);
        if (Verbose && pid() == 0)
          fprintf(stderr, "cylinder: front boundary is periodic\n");
        break;
      case 't':
        periodic(top);
        if (Verbose && pid() == 0)
          fprintf(stderr, "cylinder: top boundary is periodic\n");
        break;
      default:
        fprintf(stderr, "cylinder: unknown boundary in '%s'\n",
                periodic_boundaries);
        exit(1);
        break;
      }
  if (DTFlag)
    DT = dt_min;
  init_grid(1 << outlevel);
  run();
  if (Verbose && pid() == 0)
    fprintf(stderr, "cylinder: done\n");
}

event init(t = 0) {
  FILE *dump_file;
  if (dump_path == NULL) {
    refine(x < X0 + 0.9 * L0 && level < minlevel);
    for (;;) {
      solid(cs, fs, shape(x, y, z));
      astats s = adapt_wavelet({cs}, (double[]){0}, maxlevel = maxlevel,
                               minlevel = minlevel);
      if (Verbose && pid() == 0)
        fprintf(stderr, "cylinder: refined %d cells\n", s.nf);
      if (s.nf == 0)
        break;
    }
  } else {
    if (Verbose && pid() == 0)
      fprintf(stderr, "cylinder: reading dump from '%s'\n", dump_path);
    if ((dump_file = fopen(dump_path, "r")) == NULL) {
      fprintf(stderr, "cylinder: error: failed to open '%s'\n", dump_path);
      exit(1);
    }
    restore(fp = dump_file);
    /*
    if (fclose(dump_file) != 0) {
      fprintf(stderr, "cylinder: error: failed to close '%s'\n", dump_path);
      exit(1);
      } */
    if (Verbose && pid() == 0)
      fprintf(stderr, "cylinder: starting from '%s': time: %g, step: %d\n",
              dump_path, t, i);
    if (i == 0)
      fractions(phi, cs, fs);
    fractions_cleanup(cs, fs);
    if (Verbose)
      fields_stats();
  }
  if (i == 0) {
    if (Verbose && pid() == 0)
      fprintf(stderr, "cylinder: initialize velocity\n");
    foreach () {
      u.x[] = cs[];
      u.y[] = 0;
      u.z[] = 0;
    }
    //    event("defaults");
    event("dump");
  }
}

event properties(i++) { foreach_face() muv.x[] = fm.x[] / reynolds; }

event dump(i++; t <= tend) {
  char path[FILENAME_MAX];
  coord Fp, Fmu;
  static FILE *fp;

  if (i % period == 0) {
    if (Verbose) {
      fields_stats();
      if (pid() == 0)
        fprintf(stderr, "cylinder: %d: %09d %.16e %ld\n", npe(), i, t, grid->n);
    }
    if (output_prefix != NULL) {
      vorticity_vector(u, omega);
      lambda2(u, l2);
      if (FullOutput) {
        sprintf(path, "%s.%09d", output_prefix, i);
        output_xdmf(t, {p, l2}, {u, omega}, NULL, path);
      }
      snprintf(path, sizeof path, "%s.slice.%09d", output_prefix, i);
      output_xdmf(t, {p, l2, cs, phi}, {u, omega}, slice, path);
      if (i % (10 * period) == 0) {
        snprintf(path, sizeof path, "%s.%09d.dump", output_prefix, i);
        dump(path);
      }
    }
    if (force_path) {
      embed_force3(p, u, mu, &Fp, &Fmu);
      if (pid() == 0) {
        if (fp == NULL) {
          if ((fp = fopen(force_path, "w")) == NULL) {
            fprintf(stderr, "cylinder: error: fail to open '%s'\n", force_path);
            exit(1);
          }
        } else {
          if ((fp = fopen(force_path, "a")) == NULL) {
            fprintf(stderr, "cylinder: error: fail to open '%s'\n", force_path);
            exit(1);
          }
        }
        fprintf(fp,
                "%d %.16e %.16e %.16e %.16e %.16e %.16e %.16e %.16e %.16e "
                "%.16e %.16e\n",
                i, t, Fp.x + Fmu.x, Fp.y + Fmu.y, Fp.z + Fmu.z, Fp.x, Fp.y,
                Fp.z, Fmu.x, Fmu.y, Fmu.z, dt);
        fflush(fp);
      }
    }
  }
  if (AdaptFlag) {
    astats s = adapt_wavelet((scalar *){cs, u}, (double[]){0, 0.1, 0.1, 0.1},
                             maxlevel = maxlevel, minlevel = minlevel);
    // fractions_cleanup(cs, fs);
    unrefine(!(x < X0 + 0.9 * L0) && level > outlevel);
    fractions_cleanup(cs, fs);
    if (Verbose && i % period == 0 && pid() == 0)
      fprintf(stderr, "cylinder: refined %d cells, coarsened %d cells\n", s.nf,
              s.nc);
  }
}
