#include "predicate.h"
#include "predicate_c.h"
#include <assert.h>
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
static int num_threads(void) {
  int n;
  n = 0;
#pragma omp parallel reduction(+ : n)
  n++;
  return n;
}
#else
static int num_threads(void) { return 1; }
#endif

struct Hash {
  size_t M;
  struct {
    int64_t key;
    void *value;
  } * nodes;
};

struct DumpHeader {
  double t;
  long len;
  int i, depth, npe, version;
  struct {
    double x, y, z;
  } n;
};
struct WallData {
  double z0, z1;
  int level;
};
struct Wall {
  int (*inside)(struct WallData *, int, const double[3]);
  double (*dist2)(struct WallData *, const double[3]);
};
struct Config {
  double R[3], L, dgrid;
  int minlevel, maxlevel, outlevel, npe, ngrid, size_grid, phi_index, Verbose;
  char *stl_path, *dump_path;
  FILE *dump_file;
  struct Hash **hash;
  int32_t stl_nt, **grid, *max_grid;
  float *stl_ver;
  struct DumpHeader header;
  struct Wall *wall;
  struct WallData *wall_data;
};
static uint64_t morton(uint64_t, uint64_t, uint64_t);
static int hash_ini(size_t, void *, struct Hash *);
static int hash_insert(struct Hash *, int64_t, void *);
static int hash_search(struct Hash *, int64_t, void **);
static uint64_t traverse(uint64_t, uint64_t, uint64_t, int, struct Config *);
static double tri_point_distance2(const double[3], const double[3],
                                  const double[3], const double[3]);
static double edg2_sq(const float[2], const float[2]);
static uint64_t create_cell(struct Config *, int64_t, int64_t, int64_t, int,
                            int);
static double dist2_z(struct WallData *, const double[3]);
static int inside_z(struct WallData *, int, const double[3]);
enum { TABLE_DOUBLE, TABLE_INT, TABLE_PCHAR };
static const struct {
  const char *name;
  int type;
  long offset;
} Table[] = {
    {"X0", TABLE_DOUBLE, offsetof(struct Config, R[0])},
    {"Y0", TABLE_DOUBLE, offsetof(struct Config, R[1])},
    {"Z0", TABLE_DOUBLE, offsetof(struct Config, R[2])},
    {"L", TABLE_DOUBLE, offsetof(struct Config, L)},
    {"minlevel", TABLE_INT, offsetof(struct Config, minlevel)},
    {"maxlevel", TABLE_INT, offsetof(struct Config, maxlevel)},
    {"npe", TABLE_INT, offsetof(struct Config, npe)},
    {"stl_path", TABLE_PCHAR, offsetof(struct Config, stl_path)},
    {"dump_path", TABLE_PCHAR, offsetof(struct Config, dump_path)},
};
static const int shift[][3] = {{0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1},
                               {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}};
static char *fields_full[] = {"size",    "cs",      "u.x", "u.y", "u.z",
                              "g.x",     "g.y",     "g.z", "l2",  "omega.x",
                              "omega.y", "omega.z", "phi", NULL};
static char *fields_minimal[] = {"size", "phi", NULL};
static char **fields;

int main(int argc, char **argv) {
  char *end;
  double lo[3], hi[3], s[3], r, d2, d2max, delta;
  FILE *stl_file;
  float *a, *b, *c;
  int OutletFlag;
  int32_t i, ilo[3], ihi[3];
  int64_t inv_delta, ncells, ncells_wall, x, y, z, size;
  int index, iy, iz, iv, iw, d, j;
  size_t nbytes, nfull, nmax;
  struct Config config;
  unsigned len;
  void **work;
  struct WallData wall_data;
  struct Wall wall_z = {inside_z, dist2_z};
  config.wall_data = NULL;
  config.wall = NULL;
  config.Verbose = 0;
  OutletFlag = 0;
  fields = fields_full;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(stderr,
              "Usage: stl2dump [options] X0 Y0 Z0 L minlevel maxlevel npe "
              "file.stl basilisk.dump\n\n"
              "Options:\n"
              "  -w <axis> <start> <end>    Add extra wall (e.g., -w z 0 1)\n"
              "  -o          Refine the outlet to a minimum level\n"
              "  -m          Minimal output (size and phi values only)\n"
              "  -h          Display this help message and exit\n"
              "  -v          Enable verbose mode\n\n"
              "Arguments:\n"
              "  X0, Y0, Z0    Origin coordinates\n"
              "  L             Length scale\n"
              "  minlevel      Minimum refinement level\n"
              "  maxlevel      Maximum refinement level\n"
              "  npe           Number of MPI ranks\n"
              "  file.stl      Input STL file\n"
              "  basilisk.dump Output file\n\n"
              "Examples:\n"
              "  stl2dump -o -v -- -5 -6.25 -6.25 12.5  5 6  64 center.stl "
              "basilisk.dump\n"
              "  stl2dump -v -w z 8 -2 2 -o -- -5 -6.25 -6.25 12.5  6 9  64 "
              "center.stl basilisk.dump\n"
              "Additional Info:\n"
              "  num_threads: %d\n",
              num_threads());
      exit(1);
    case 'w':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "stl2dump: error: -w needs more arguments\n");
        exit(1);
      }
      if (strcmp(*argv, "z") != 0) {
        fprintf(stderr, "stl2dump: error: unsupported wall type '%s'\n", *argv);
        exit(1);
      }
      argv++;
      if (argv[0] == NULL || argv[1] == NULL || argv[2] == NULL) {
        fprintf(stderr, "stl2dump: error: -w needs three arguments\n");
        exit(1);
      }
      wall_data.level = strtol(*argv, &end, 10);
      if (*end != '\0') {
        fprintf(stderr, "stl2dump: error: '%s' is not an integer\n", *argv);
        exit(1);
      }
      argv++;
      wall_data.z0 = strtod(*argv, &end);
      if (*end != '\0') {
        fprintf(stderr, "stl2dump: error: '%s' is not a double\n", *argv);
        exit(1);
      }
      argv++;
      wall_data.z1 = strtod(*argv, &end);
      if (*end != '\0') {
        fprintf(stderr, "stl2dump: error: '%s' is not a double\n", *argv);
        exit(1);
      }
      if (wall_data.z1 <= wall_data.z0) {
        fprintf(stderr, "stl2dump: error: z1 <= z0\n");
        exit(1);
      }
      config.wall_data = &wall_data;
      config.wall = &wall_z;
      break;
    case 'v':
      config.Verbose = 1;
      break;
    case 'o':
      OutletFlag = 1;
      break;
    case 'm':
      fields = fields_minimal;
      break;
    case '-':
      argv++;
      goto positional;
    default:
      fprintf(stderr, "stl2dump: error: unknown option '%s'\n", *argv);
      exit(1);
    }
positional:
  if (config.Verbose) {
    fprintf(stderr, "stl2dump: num_threads: %d\n", num_threads());
  }
  for (i = 0; i < sizeof(Table) / sizeof(*Table); i++) {
    if (*argv == NULL) {
      fprintf(stderr, "stl2dump: error: missing '%s' option\n", Table[i].name);
      exit(1);
    }
    switch (Table[i].type) {
    case TABLE_DOUBLE:
      *(double *)((void *)&config + Table[i].offset) = strtod(*argv, &end);
      if (*end != '\0') {
        fprintf(stderr, "stl2dump: error: '%s' is not a double\n", *argv);
        exit(1);
      }
      break;
    case TABLE_INT:
      *(int *)((void *)&config + Table[i].offset) = strtol(*argv, &end, 10);
      if (*end != '\0') {
        fprintf(stderr, "stl2dump: error: '%s' is not an integer\n", *argv);
        exit(1);
      }
      break;
    case TABLE_PCHAR:
      *(char **)((void *)&config + Table[i].offset) = *argv;
      break;
    }
    argv++;
  }
  if (!(config.minlevel <= config.maxlevel)) {
    fprintf(stderr,
            "stl2dump: error: fail to open minlevel should be <= maxlevel\n");
    exit(1);
  }
  if ((stl_file = fopen(config.stl_path, "r")) == NULL) {
    fprintf(stderr, "stl2dump: error: fail to open '%s'\n", config.stl_path);
    exit(1);
  }
  if (fseek(stl_file, 80, SEEK_SET) != 0) {
    fprintf(stderr, "stl2dump: error: fail to read '%s'\n", config.stl_path);
    exit(1);
  }
  if (fread(&config.stl_nt, sizeof(config.stl_nt), 1, stl_file) != 1) {
    fprintf(stderr, "stl2dump: error: fail to read '%s'\n", config.stl_path);
    exit(1);
  }
  if (config.Verbose)
    fprintf(stderr, "stl2dump: stl_nt: %d\n", config.stl_nt);

  if ((config.stl_ver = malloc(9 * config.stl_nt * sizeof *config.stl_ver)) ==
      NULL) {
    fprintf(stderr, "stl2dump: error: malloc failed\n");
    exit(1);
  }
  for (i = 0; i < config.stl_nt; i++) {
    fseek(stl_file, 3 * sizeof *config.stl_ver, SEEK_CUR);
    if (fread(&config.stl_ver[9 * i], sizeof *config.stl_ver, 9, stl_file) !=
        9) {
      fprintf(stderr, "stl2dump: error: fail to read '%s'\n", config.stl_path);
      exit(1);
    }
    fseek(stl_file, 2, SEEK_CUR);
  }
  if (fclose(stl_file) != 0) {
    fprintf(stderr, "stl2dump: error: fail to close '%s'\n", config.stl_path);
    exit(1);
  }
  config.hash = malloc((config.maxlevel + 1) * sizeof *config.hash);
  work = malloc((config.maxlevel + 1) * sizeof *work);
  if (config.hash == NULL || work == NULL) {
    fprintf(stderr, "stl2dump: error: malloc failed\n");
    exit(1);
  }
  nmax = (1ul << 26) * sizeof *(*config.hash)->nodes;
  nfull = sizeof *(*config.hash)->nodes;
  for (i = 0; i < config.maxlevel + 1; i++) {
    nbytes = nfull < nmax ? nfull : nmax;
    if ((work[i] = malloc(nbytes)) == NULL) {
      fprintf(stderr, "stl2dump: error: malloc failed\n");
      exit(1);
    }
    if ((config.hash[i] = malloc(sizeof *config.hash[i])) == NULL) {
      fprintf(stderr, "stl2dump: error: malloc failed\n");
      exit(1);
    }
    hash_ini(nbytes, work[i], config.hash[i]);
    nfull <<= 3;
  }

  d2max = 0;
  for (i = 0; i < config.stl_nt; i++) { /* yz */
    a = &config.stl_ver[9 * i];
    b = &config.stl_ver[9 * i + 3];
    c = &config.stl_ver[9 * i + 6];
    if ((d2 = edg2_sq(a + 1, b + 1)) > d2max)
      d2max = d2;
    if ((d2 = edg2_sq(a + 1, c + 1)) > d2max)
      d2max = d2;
    if ((d2 = edg2_sq(b + 1, c + 1)) > d2max)
      d2max = d2;
  }
  config.dgrid = sqrt(d2max);
  config.ngrid = ceil(config.L / config.dgrid);
  config.size_grid = config.ngrid * config.ngrid;
  if ((config.grid = malloc(config.size_grid * sizeof *config.grid)) == NULL) {
    fprintf(stderr, "stl2dump: error: malloc failed\n");
    exit(1);
  }
  if ((config.max_grid = malloc(config.size_grid * sizeof *config.max_grid)) ==
      NULL) {
    fprintf(stderr, "stl2dump: error: malloc failed\n");
    exit(1);
  }
  for (i = 0; i < config.size_grid; i++) {
    config.grid[i] = NULL;
    config.max_grid[i] = 0;
  }
  ncells = 0;
  for (i = 0; i < config.stl_nt; i++) { /* yz */
    a = &config.stl_ver[9 * i];
    b = &config.stl_ver[9 * i + 3];
    c = &config.stl_ver[9 * i + 6];
    s[0] = (a[0] + b[0] + c[0]) / 3;
    s[1] = (a[1] + b[1] + c[1]) / 3;
    s[2] = (a[2] + b[2] + c[2]) / 3;
    iv = (s[1] - config.R[1]) / config.dgrid;
    iw = (s[2] - config.R[2]) / config.dgrid;
    for (iy = iv - 1; iy <= iv + 1; iy++)
      for (iz = iw - 1; iz <= iw + 1; iz++) {
        index = iy * config.ngrid + iz;
        if (0 <= index && index < config.size_grid) {
          config.max_grid[index]++;
          config.grid[index] =
              realloc(config.grid[index],
                      config.max_grid[index] * sizeof *config.grid[index]);
          config.grid[index][config.max_grid[index] - 1] = i;
        }
      }
  }
  inv_delta = 1ul << config.maxlevel;
  for (i = 0; i < config.stl_nt; i++) {
    lo[0] = lo[1] = lo[2] = DBL_MAX;
    hi[0] = hi[1] = hi[2] = -DBL_MAX;
    for (j = 0; j < 3; j++) {
      for (d = 0; d < 3; d++) {
        r = config.stl_ver[9 * i + 3 * j + d];
        if (r < lo[d])
          lo[d] = r;
        if (r > hi[d])
          hi[d] = r;
      }
    }
    for (d = 0; d < 3; d++) {
      ilo[d] = (lo[d] - config.R[d]) / config.L * inv_delta;
      ihi[d] = ceil((hi[d] - config.R[d]) / config.L * inv_delta);
      if (ilo[d] < 0)
        ilo[d] = 0;
      if (ilo[d] > inv_delta)
        ilo[d] = inv_delta;
      if (ihi[d] < 0)
        ihi[d] = 0;
      if (ihi[d] > inv_delta)
        ihi[d] = inv_delta;
    }
    for (x = ilo[0]; x < ihi[0]; x++)
      for (y = ilo[1]; y < ihi[1]; y++)
        for (z = ilo[2]; z < ihi[2]; z++)
          ncells += create_cell(&config, x, y, z, config.maxlevel, 1);
  }

  inv_delta = 1ul << config.minlevel;
  delta = config.L / inv_delta;
  for (z = 0; z < inv_delta; z++)
    for (y = 0; y < inv_delta; y++)
      if (OutletFlag)
        for (x = 0; 10 * x < 9 * inv_delta; x++) {
          s[0] = config.R[0] + delta * (x + 0.5);
          s[1] = config.R[1] + delta * (y + 0.5);
          s[2] = config.R[2] + delta * (z + 0.5);
          if (config.wall->inside(config.wall_data, 1, s))
            ncells += create_cell(&config, x, y, z, config.minlevel, 1);
        }
      else
        for (x = 0; x < inv_delta; x++)
          ncells += create_cell(&config, x, y, z, config.minlevel, 1);

  ncells_wall = 0;
  if (config.wall) {
    inv_delta = 1ul << config.wall_data->level;
    delta = config.L / inv_delta;
    for (z = 0; z < inv_delta; z++)
      for (y = 0; y < inv_delta; y++)
        for (x = 0; x < inv_delta; x++) {
          s[0] = config.R[0] + delta * (x + 0.5);
          s[1] = config.R[1] + delta * (y + 0.5);
          s[2] = config.R[2] + delta * (z + 0.5);
          d2 = config.wall->dist2(config.wall_data, s);
          if (d2 < delta * delta) {
            ncells += create_cell(&config, x, y, z, config.wall_data->level, 1);
            ncells_wall++;
          }
        }
  }

  if (config.Verbose) {
    fprintf(stderr, "stl2dump: ncells_wall: %ld\n", ncells_wall);
    fprintf(stderr, "stl2dump: ncells: %ld\n", ncells);
  }

  if ((config.dump_file = fopen(config.dump_path, "w")) == NULL) {
    fprintf(stderr, "stl2dump: error: fail to open '%s'\n", config.dump_path);
    exit(1);
  }
  config.header.t = 0;
  for (config.header.len = 0; fields[config.header.len] != NULL;
       config.header.len++)
    ;
  config.header.i = 0;
  config.header.depth = config.maxlevel;
  config.header.npe = config.npe;
  config.header.version = 170901;
  config.header.n.x = 0;
  config.header.n.y = 0;
  config.header.n.z = 0;

  config.phi_index = -1;
  for (i = 0; i < config.header.len; i++)
    if (strcmp(fields[i], "phi") == 0)
      config.phi_index = i;
  if (config.phi_index == -1) {
    fprintf(stderr, "stl2dump: error: not `phi' in fields\n");
    exit(1);
  }
  if (config.Verbose)
    fprintf(stderr, "config.phi_index: %d\n", config.phi_index);
  if (fwrite(&config.header, sizeof(config.header), 1, config.dump_file) != 1) {
    fprintf(stderr, "stl2dump: error: fail to write '%s'\n", config.dump_path);
    exit(1);
  }
  for (i = 0; i < config.header.len; i++) {
    len = strlen(fields[i]);
    if (fwrite(&len, sizeof(len), 1, config.dump_file) != 1) {
      fprintf(stderr, "stl2dump: error: fail to write '%s'\n",
              config.dump_path);
      exit(1);
    }
    if (fwrite(fields[i], len, 1, config.dump_file) != 1) {
      fprintf(stderr, "stl2dump: error: fail to write '%s'\n",
              config.dump_path);
      exit(1);
    }
  }
  if (fwrite(config.R, sizeof(config.R), 1, config.dump_file) != 1) {
    fprintf(stderr, "stl2dump: error: fail to write '%s'\n", config.dump_path);
    exit(1);
  }
  if (fwrite(&config.L, sizeof(config.L), 1, config.dump_file) != 1) {
    fprintf(stderr, "stl2dump: error: fail to write '%s'\n", config.dump_path);
    exit(1);
  }
  predicate_ini();
  size = traverse(0, 0, 0, 0, &config);
  if (config.Verbose)
    fprintf(stderr, "stl2dump: size: %" PRIu64 "\n", size);
  for (i = 0; i < config.size_grid; i++)
    free(config.grid[i]);
  free(config.grid);
  free(config.max_grid);
  for (i = 0; i < config.maxlevel + 1; i++) {
    free(config.hash[i]);
    free(work[i]);
  }
  free(work);
  free(config.hash);
  free(config.stl_ver);
  if (fclose(config.dump_file) != 0) {
    fprintf(stderr, "stl2dump: error: fail to close '%s'\n", config.dump_path);
    exit(1);
  }
}

static uint64_t left(uint64_t x) {
  x = (x | x << 32) & 0x1f00000000ffffull;
  x = (x | x << 16) & 0x1f0000ff0000ffull;
  x = (x | x << 8) & 0x100f00f00f00f00full;
  x = (x | x << 4) & 0x10c30c30c30c30c3ull;
  x = (x | x << 2) & 0x1249249249249249ull;
  return x;
}

static uint64_t morton(uint64_t x, uint64_t y, uint64_t z) {
  return (left(z) << 2) | (left(y) << 1) | (left(x) << 0);
}

static int hash_ini(size_t nbytes, void *memory, struct Hash *set) {
  size_t i;
  set->M = nbytes / sizeof *set->nodes;
  set->nodes = memory;
  for (i = 0; i < set->M; i++)
    set->nodes[i].key = -1;
  return 0;
}
static int hash_insert(struct Hash *set, int64_t key, void *value) {
  int64_t key0;
  size_t cnt;
  uint64_t x;
  assert(key >= 0);
  x = key % set->M;
  for (cnt = 0; cnt < set->M; cnt++) {
    key0 = set->nodes[x].key;
    if (key0 == -1) {
      set->nodes[x].key = key;
      set->nodes[x].value = value;
      return 1;
    } else if (key0 == key) {
      set->nodes[x].value = value;
      return 0;
    }
    x = (x + 1 + cnt) % set->M;
  }
  fprintf(stderr,
          "stl2dump: error: hash_insert over capacity (M: %ld, key: %ld)\n",
          set->M, key % set->M);
  exit(1);
}
static int hash_search(struct Hash *set, int64_t key, void **pvalue) {
  int64_t key0;
  size_t cnt;
  uint64_t x;
  if (key < 0) {
    fprintf(stderr, "stl2dump: error: key < 0\n");
    exit(1);
  }
  x = key % set->M;
  for (cnt = 0; cnt < set->M; cnt++) {
    key0 = set->nodes[x].key;
    if (key0 == key) {
      if (pvalue != NULL)
        *pvalue = set->nodes[x].value;
      return 1;
    } else if (key0 == -1) {
      return 0;
    }
    x = (x + 1 + cnt) % set->M;
  }
  fprintf(stderr, "stl2dump: error: hash_search failed (M: %ld, key: %ld)\n",
          set->M, key % set->M);
  exit(1);
}

static uint64_t traverse(uint64_t x, uint64_t y, uint64_t z, int level,
                         struct Config *config) {
  double delta, minimum, s[3], *values;
  int leaf, i, intersect, iy, iz, index;
  uint32_t leaf_code;
  uint64_t cell_size, u, v, w;
  long pos, curr, code_ch;
  if ((values = malloc(config->header.len * sizeof *values)) == NULL) {
    fprintf(stderr, "stl2dump: error: malloc failed\n");
    exit(1);
  }
  delta = config->L / (1ul << level);
  s[0] = config->R[0] + delta * (x + 0.5);
  s[1] = config->R[1] + delta * (y + 0.5);
  s[2] = config->R[2] + delta * (z + 0.5);

  iy = (s[1] - config->R[1]) / config->dgrid;
  iz = (s[2] - config->R[2]) / config->dgrid;
  index = iy * config->ngrid + iz;
  assert(0 <= index && index < config->size_grid);
  intersect = 0;
  minimum = config->dgrid;
  if (config->Verbose && level == 3)
    fprintf(stderr, "stl2dump: level: %d [%ld %ld %ld]\n", level, x, y, z);
#pragma omp parallel for reduction(min : minimum) reduction(+ : intersect)
  for (i = 0; i < config->max_grid[index]; i++) {
    int j;
    double a[3], b[3], c[3], e[3], dist2;
    j = 9 * config->grid[index][i];
    a[0] = config->stl_ver[j];
    a[1] = config->stl_ver[j + 1];
    a[2] = config->stl_ver[j + 2];

    b[0] = config->stl_ver[j + 3];
    b[1] = config->stl_ver[j + 4];
    b[2] = config->stl_ver[j + 5];

    c[0] = config->stl_ver[j + 6];
    c[1] = config->stl_ver[j + 7];
    c[2] = config->stl_ver[j + 8];

    e[0] = s[0] + 3 * config->L;
    e[1] = s[1];
    e[2] = s[2];
    dist2 = tri_point_distance2(a, b, c, s);
    if (dist2 < minimum)
      minimum = dist2;
    intersect += predicate_ray(s, e, a, b, c);
  }
  for (i = 0; i < config->header.len; i++)
    values[i] = 0.0;

  if (config->wall == NULL) {
    values[config->phi_index] =
        intersect % 2 == 0 ? sqrt(minimum) : -sqrt(minimum);
  } else {
    double dist2;
    int inside;
    inside = config->wall->inside(config->wall_data, intersect % 2 == 0, s);
    dist2 = config->wall->dist2(config->wall_data, s);
    if (dist2 < minimum)
      minimum = dist2;
    values[config->phi_index] = inside ? sqrt(minimum) : -sqrt(minimum);
  }
  code_ch = morton(x << 1, y << 1, z << 1);
  leaf = level + 1 > config->maxlevel ||
         !hash_search(config->hash[level + 1], code_ch, NULL);
  leaf_code = leaf ? 2 : 0;
  if (fwrite(&leaf_code, sizeof(leaf_code), 1, config->dump_file) != 1) {
    fprintf(stderr, "stl2dump: error: fail to write '%s'\n", config->dump_path);
    exit(1);
  }
  pos = ftell(config->dump_file);
  if (fwrite(values, config->header.len * sizeof *values, 1,
             config->dump_file) != 1) {
    fprintf(stderr, "stl2dump: error: fail to write '%s'\n", config->dump_path);
    exit(1);
  }
  cell_size = 1;
  if (!leaf) {
    for (i = 0; i < sizeof shift / sizeof *shift; i++) {
      u = (x << 1) + shift[i][0];
      v = (y << 1) + shift[i][1];
      w = (z << 1) + shift[i][2];
      cell_size += traverse(u, v, w, level + 1, config);
    }
  }
  curr = ftell(config->dump_file);
  fseek(config->dump_file, pos, SEEK_SET);
  values[0] = cell_size;
  if (fwrite(&values[0], sizeof(values[0]), 1, config->dump_file) != 1) {
    fprintf(stderr, "stl2dump: error: fail to write '%s'\n", config->dump_path);
    exit(1);
  }
  free(values);
  fseek(config->dump_file, curr, SEEK_SET);
  return cell_size;
}

static double edg2_sq(const float a[2], const float b[2]) {
  double x, y;
  x = a[0] - b[0];
  y = a[1] - b[1];
  return x * x + y * y;
}

static double vec_dot(const double a[3], const double b[3]) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static double edg_sq(const double a[3], const double b[3]) {
  double u[3];
  u[0] = a[0] - b[0];
  u[1] = a[1] - b[1];
  u[2] = a[2] - b[2];
  return vec_dot(u, u);
}

static double edg_point_distance2(const double a[3], const double b[3],
                                  const double p[3]) {
  enum { X, Y, Z };
  double t, s, x, y, z;

  s = edg_sq(a, b);
  if (s == 0)
    return edg_sq(p, a);
  t = ((b[X] - a[X]) * (p[X] - a[X]) + (b[Y] - a[Y]) * (p[Y] - a[Y]) +
       (b[Z] - a[Z]) * (p[Z] - a[Z])) /
      s;
  if (t > 1.0)
    return edg_sq(p, b);
  if (t < 0.0)
    return edg_sq(p, a);
  x = (1 - t) * a[X] + t * b[X] - p[X];
  y = (1 - t) * a[Y] + t * b[Y] - p[Y];
  z = (1 - t) * a[Z] + t * b[Z] - p[Z];
  return x * x + y * y + z * z;
}

static void vec_minus(const double a[3], const double b[3], /**/ double c[3]) {
  enum { X, Y, Z };
  c[X] = a[X] - b[X];
  c[Y] = a[Y] - b[Y];
  c[Z] = a[Z] - b[Z];
}

static double tri_point_distance2(const double a[3], const double b[3],
                                  const double c[3], const double p[3]) {
  enum { X, Y, Z };

  double u[3], v[3], q[3];
  double A, B, C, D, E, det;
  double t1, t2;
  double x, y, z;
  double d1, d2;

  vec_minus(b, a, u);
  vec_minus(c, a, v);
  B = vec_dot(v, u);
  E = vec_dot(u, u);
  C = vec_dot(v, v);
  det = B * B - E * C;
  if (det == 0) {
    d1 = edg_point_distance2(a, b, p);
    d2 = edg_point_distance2(b, c, p);
    if (d1 < d2)
      return d1;
    return d2;
  }
  vec_minus(a, p, q);
  A = vec_dot(v, q);
  D = vec_dot(u, q);
  t1 = (D * C - A * B) / det;
  t2 = (A * E - D * B) / det;
  if (t1 < 0)
    return edg_point_distance2(a, c, p);
  if (t2 < 0)
    return edg_point_distance2(a, b, p);
  if (t1 + t2 > 1)
    return edg_point_distance2(b, c, p);
  x = q[X] + t1 * u[X] + t2 * v[X];
  y = q[Y] + t1 * u[Y] + t2 * v[Y];
  z = q[Z] + t1 * u[Z] + t2 * v[Z];
  return x * x + y * y + z * z;
}

static uint64_t create_cell(struct Config *config, int64_t x, int64_t y,
                            int64_t z, int level, int need_siblings) {
  int i;
  uint64_t px, py, pz, code, ncells, delta;
  int64_t sx, sy, sz, u, v, w;
  if (x < 0 || y < 0 || z < 0)
    return 0;
  delta = 1 << level;
  if (x >= delta || y >= delta || z >= delta)
    return 0;
  ncells = 0;
  code = morton(x, y, z);
  if (level > 0 && !hash_search(config->hash[level], code, NULL)) {
    if (need_siblings) {
      px = x >> 1;
      py = y >> 1;
      pz = z >> 1;
      ncells += create_cell(config, px, py, pz, level - 1, 1);
      for (i = 0; i < sizeof shift / sizeof *shift; i++) {
        u = (px << 1) + shift[i][0];
        v = (py << 1) + shift[i][1];
        w = (pz << 1) + shift[i][2];
        ncells += create_cell(config, u, v, w, level, 0);
      }
    }
    sx = (x & 1) ? x + 1 : x - 1;
    sy = (y & 1) ? y + 1 : y - 1;
    sz = (z & 1) ? z + 1 : z - 1;
    for (i = 0; i < sizeof shift / sizeof *shift; i++) {
      u = (sx + shift[i][0]) >> 1;
      v = (sy + shift[i][1]) >> 1;
      w = (sz + shift[i][2]) >> 1;
      ncells += create_cell(config, u, v, w, level - 1, 1);
    }
    ncells += hash_insert(config->hash[level], code, NULL);
  }
  return ncells;
}
static double dist2_z(struct WallData *wall_data, const double s[3]) {
  double d0, d1;
  d0 = fabs(s[2] - wall_data->z0);
  d1 = fabs(s[2] - wall_data->z1);
  if (d1 < d0)
    d0 = d1;
  return d0 * d0;
}
static int inside_z(struct WallData *wall_data, int inside, const double s[3]) {
  return inside && (wall_data->z0 < s[2] && s[2] < wall_data->z1);
}
