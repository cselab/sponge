#include <stdint.h>
#include <stdbool.h>
#include "grid/octree.h"
#include "navier-stokes/centered.h"
#include "fractions.h"
#include "two-phase.h"
#include "distance.h"
#include "output_htg.h"
static const double diameter = 0.3733333285861546;
static double Reynolds;
static int maxlevel, period, Scale, Inside;
static char *stl_path;
static long level;

u.n[right] = y > 0 ? neumann(0) : dirichlet(-1);
p[right] = y > 0 ? dirichlet(0) : neumann(0);
pf[right] = y > 0 ? dirichlet(0) : neumann(0);

/*
u.n[left] = dirichlet(1);
p[left] = neumann(0);
pf[left] = neumann(0);

u.n[right] = neumann(0);
p[right] = dirichlet(0);
pf[right] = dirichlet(0);
*/

face vector muv[];
scalar omega[], tangaroa[];

void append(Array *a, void *elem, size_t size) {
  if (a->len + size >= a->max) {
    a->max += max(size, 4096);
    a->p = realloc(a->p, a->max);
    if (a->p == NULL) {
      fprintf(stderr, "stl: error: realloc failed\n");
      exit(1);
    }
  }
  memcpy(((char *)a->p) + a->len, elem, size);
  a->len += size;
}

trace coord *read_stl(FILE *fp) {
  Array *a = array_new();
  char tag[6];
  uint32_t nf, i;
  char header[80];
  float x, y, z;
  unsigned j;
  uint16_t attbytecount;

  if (fread(header, sizeof(char), 80, fp) != 80) {
    fprintf(stderr, "Input file is not a valid STL file\n"
                    "stdin: incomplete header\n");
    exit(1);
  }
  if (fread(&nf, sizeof(uint32_t), 1, fp) != 1) {
    fprintf(stderr, "Input file is not a valid STL file\n"
                    "stdin: missing number of facets\n");
    exit(1);
  }
  fprintf(stderr, "std: nf: %d\n", (int)nf);
  for (i = 0; i < nf; i++) {
    if (fread(&x, sizeof(float), 1, fp) != 1) {
      fprintf(stderr, "Input file is not a valid STL file\n"
                      "stdin: missing normal x-coordinate\n");
      exit(1);
    }
    if (fread(&y, sizeof(float), 1, fp) != 1) {
      fprintf(stderr, "Input file is not a valid STL file\n"
                      "stdin: missing normal y-coordinate\n");
      exit(1);
    }
    if (fread(&z, sizeof(float), 1, fp) != 1) {
      fprintf(stderr, "Input file is not a valid STL file\n"
                      "stdin: missing normal z-coordinate\n");
      exit(1);
    }
    for (j = 0; j < 3; j++) {
      if (fread(&x, sizeof(float), 1, fp) != 1) {
        fprintf(stderr, "Input file is not a valid STL file\n"
                        "stdin: missing vertex x-coordinate\n");
        exit(1);
      }
      if (fread(&y, sizeof(float), 1, fp) != 1) {
        fprintf(stderr, "Input file is not a valid STL file\n"
                        "stdin: missing vertex y-coordinate\n");
        exit(1);
      }
      if (fread(&z, sizeof(float), 1, fp) != 1) {
        fprintf(stderr, "Input file is not a valid STL file\n"
                        "stdin: missing vertex z-coordinate\n");
        exit(1);
      }
      coord p = {x, y, z};
      append(a, &p, sizeof(coord));
    }
    if (fread(&attbytecount, sizeof(uint16_t), 1, fp) != 1) {
      fprintf(stderr, "Input file is not a valid STL file\n"
                      "stdin: missing attribute byte count\n");
      exit(1);
    }
  }
  coord p = {nodata};
  append(a, &p, sizeof(coord));
  return (coord *)array_shrink(a);
}

void fraction_from_stl(scalar f) {
  coord *p, *q, center, min, max;
  double scale;
  FILE *fp;
  scalar d[];
  vertex scalar phi[];
  if ((fp = fopen(stl_path, "r")) == NULL) {
    fprintf(stderr, "stl: error: fail to open '%s'\n", stl_path);
    exit(1);
  }
  p = read_stl(fp);
  if (fclose(fp) != 0) {
    fprintf(stderr, "stl: fail to close '%s'\n", stl_path);
    exit(1);
  }
  bounding_box(p, &min, &max);
  if (Scale) {
    scale = 0.8 / (max.z - min.z);
    foreach_dimension() center.x = (min.x + max.x) / 2;
    for (q = p; q->x != nodata; q++) {
      foreach_dimension() { (*q).x = (q->x - center.x) * scale; }
    }
    bounding_box(p, &min, &max);
  }
  fprintf(stderr, "stl: min: %g %g %g\n", min.x, min.y, min.z);
  fprintf(stderr, "stl: max: %g %g %g\n", max.x, max.y, max.z);
  distance(d, p);
  for (;;) {
    astats s = adapt_wavelet({d}, (double[]){0.0}, maxlevel, level);
    fprintf(stderr, "# refined %d cells, coarsened %d cells\n", s.nf, s.nc);
    if (s.nf == 0)
      break;
  }
  foreach_vertex() {
    double p0, s0;
    s0 = (d[] + d[-1] + d[0, -1] + d[-1, -1] + d[0, 0, -1] + d[-1, 0, -1] +
          d[0, -1, -1] + d[-1, -1, -1]) /
         8.;
    /*p0 = min(-s0, sq(x) + sq(y) - sq(diameter / 2));
    p0 = max(p0, z - 0.4);
    p0 = max(p0, -0.4 - z); */
    phi[] = Inside ? s0 : -s0;
  }
  fractions(phi, f);
}

int main(int argc, char **argv) {
  int LevelFlag, PeriodFlag, MaxLevelFlag, ReynoldsFlag;
  char *end;
  LevelFlag = 0;
  PeriodFlag = 0;
  MaxLevelFlag = 0;
  ReynoldsFlag = 0;
  Scale = 0;
  Inside = 0;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(stderr, "stl [-s] [-i] -l INT -p INT -r FLOAT file.stl\n");
      exit(1);
    case 'l':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "stl: -l needs an argument\n");
        exit(1);
      }
      level = strtol(*argv, &end, 10);
      if (*end != '\0' || level <= 0) {
        fprintf(stderr, "stl: '%s' is not a positive integer\n", *argv);
        exit(1);
      }
      LevelFlag = 1;
      break;
    case 'p':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "stl: -p needs an argument\n");
        exit(1);
      }
      period = strtol(*argv, &end, 10);
      if (*end != '\0' || period <= 0) {
        fprintf(stderr, "stl: '%s' is not a positive integer\n", *argv);
        exit(1);
      }
      PeriodFlag = 1;
      break;
    case 'm':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "stl: -m needs an argument\n");
        exit(1);
      }
      maxlevel = strtol(*argv, &end, 10);
      if (*end != '\0' || maxlevel <= 0) {
        fprintf(stderr, "stl: '%s' is not a positive integer\n", *argv);
        exit(1);
      }
      MaxLevelFlag = 1;
      break;
    case 'r':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "stl: -r needs an argument\n");
        exit(1);
      }
      Reynolds = strtod(*argv, &end);
      if (*end != '\0') {
        fprintf(stderr, "stl: error: '%s' is not a number\n", *argv);
        exit(1);
      }
      ReynoldsFlag = 1;
      break;
    case 's':
      Scale = 1;
      break;
    case 'i':
      Inside = 1;
      break;
    default:
      fprintf(stderr, "stl: error: unrecognized command-line option '%s'\n",
              *argv);
      exit(1);
    }
  if (LevelFlag == 0) {
    fprintf(stderr, "stl: error: -l must be set\n");
    exit(1);
  }
  if (!PeriodFlag) {
    fprintf(stderr, "stl: error: -p must be set\n");
    exit(1);
  }
  if (!MaxLevelFlag) {
    fprintf(stderr, "stl: error: -m must be set\n");
    exit(1);
  }
  if (!ReynoldsFlag) {
    fprintf(stderr, "stl: error: -r must be set\n");
    exit(1);
  }
  stl_path = *argv;
  if (Scale) {
    size(5.0);
    origin(-1.0, -L0 / 2, -L0 / 2);
  } else {
    origin(-L0 / 2, -L0 / 2, -L0 / 2);
  }
  init_grid(1 << level);
  mu = muv;
  run();
}
event properties(i++) { foreach_face() muv.x[] = fm.x[] * diameter / Reynolds; }

event init(t = 0) {
  if (!restore(file = "restart")) {
    if (npe() > 1) {
      fprintf(stderr, "stl: not compatible with MPI\n");
      exit(1);
    }
    if (stl_path == NULL) {
      fprintf(stderr, "stl: error: need STL file\n");
      exit(1);
    }
    fraction_from_stl(tangaroa);
    foreach () {
      u.x[] = 1;
      u.y[] = 0;
      u.z[] = 0;
    }
    dump(file = "restart");
    exit(0);
  } else {
    if (pid() == 0)
      fprintf(stderr, "stl: rank: %d/%d: reading restart\n", pid(), npe());
  }
}

event velocity(i++) {
  foreach ()
    foreach_dimension() u.x[] = tangaroa[] * u.x[];
}

event dump(i++; t <= 10000) {
  static long iframe = 0;
  int rank;
  char hdg[FILENAME_MAX];
  char path[] = ".";
  if (iframe % period == 0) {
    vorticity(u, omega);
    sprintf(hdg, "h.%09ld", iframe);
    output_htg({p, omega, tangaroa}, {u, g}, path, hdg, iframe, t);
    fields_stats();
  }
  iframe++;
}

event adapt(i++) {
  double uemax = 0.01;
  astats s =
      adapt_wavelet({tangaroa, u}, (double[]){0.01, 0.01, uemax, uemax, uemax},
                    maxlevel = maxlevel, minlevel = level);
  fprintf(stderr, "# refined %d cells, coarsened %d cells\n", s.nf, s.nc);
}
