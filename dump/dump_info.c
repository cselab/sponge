#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FREAD(ptr, size, nmemb)                                                \
  if (fread(ptr, size, nmemb, input_file) != (uint64_t)(nmemb)) {              \
    fprintf(stderr, "dump_info: error: fail to read from '%s'\n", input_path); \
    exit(1);                                                                   \
  }

static FILE *input_file;
static char *input_path;
struct coord {
  double x, y, z;
};
struct DumpHeader {
  double t;
  long len;
  int i, depth, npe, version;
  struct coord n;
};
static struct DumpHeader header;
static int *index;
static double *values;
static int malloc_level;
static long traverse(int);
static double X0, Y0, Z0, L0;
static long nleaf;
int main(int argc, char **argv) {
  long i;
  unsigned len;
  double o[4];
  char **names;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(stderr,
              "Usage: dump_info [-h] [-v] file.dump\n"
              "Options:\n"
              "  -h                          Print help message and exit\n"
              "  file.dump                   basilisk dump\n");
      exit(1);
    default:
      fprintf(stderr, "dump_info: error: unknown option '%s'\n", *argv);
      exit(1);
    }
  if ((input_path = argv[0]) == NULL) {
    fprintf(stderr, "dump_info: error: file.dump xois not given\n");
    exit(1);
  }

  if ((input_file = fopen(input_path, "r")) == NULL) {
    fprintf(stderr, "dump_info: error: fail to open '%s'\n", input_path);
    exit(1);
  }
  FREAD(&header, sizeof header, 1);
  fprintf(stderr,
          "version:             dump version: %d\n"
          "      t:          simulation time: %.16e\n"
          "    len:          numer of fields: %ld\n"
          "    npe:     number of processors: %d\n"
          "  depth:          multigrid depth: %d\n"
          "      i:     simulation iteration: %d\n"
          "      n: multigrid MPI dimensions: [%g %g %g]\n",
          header.version, header.t, header.len, header.npe, header.depth,
          header.i, header.n.x, header.n.y, header.n.z);
  if ((names = malloc(header.len * sizeof *names)) == NULL) {
    fprintf(stderr, "dump_info: error: malloc failed\n");
    exit(1);
  }
  for (i = 0; i < header.len; i++) {
    FREAD(&len, sizeof len, 1);
    names[i] = malloc((len + 1) * sizeof *names[i]);
    FREAD(names[i], sizeof *names[i], len);
    names[i][len] = '\0';
    fprintf(stderr, "name: %s\n", names[i]);
  }
  FREAD(o, sizeof o, 1);
  fprintf(stderr,
          " origin: [%.16e %.16e %.16e]\n"
          "   size: %.16e\n",
          o[0], o[1], o[2], o[3]);
  X0 = o[0];
  Y0 = o[1];
  Z0 = o[2];
  L0 = o[3];
  malloc_level = 0;
  index = NULL;
  if ((values = malloc(header.len * sizeof *values)) == NULL) {
    fprintf(stderr, "dump_info: error: malloc failed\n");
    exit(1);
  }
  nleaf = 0;
  traverse(0);
  fprintf(stderr, "nleaf: %ld\n", nleaf);
  free(index);
  for (i = 0; i < header.len; i++)
    free(names[i]);
  free(names);
  if (fclose(input_file) != 0) {
    fprintf(stderr, "dump_info: error: fail to close '%s'\n", input_path);
    exit(1);
  }
}
static void process(int level) { nleaf++; }
static long traverse(int level) {
  enum { leaf = 2 };
  unsigned flags;
  long size, size0;

  if (fread(&flags, sizeof flags, 1, input_file) != 1 ||
      fread(values, sizeof *values, header.len, input_file) != header.len) {
    fprintf(stderr, "dump_info: fail to read '%s' at level '%d'\n", input_path,
            level);
    exit(1);
  }
  size = values[0];
  size0 = 1;
  if (flags & leaf)
    process(level);
  if (flags & leaf) {
    /* */
  } else {
    while (level + 1 >= malloc_level) {
      malloc_level = 2 * malloc_level + 2;
      index = realloc(index, malloc_level * sizeof *index);
    }
    for (index[level + 1] = 0; index[level + 1] < 8; index[level + 1]++)
      size0 += traverse(level + 1);
  }
  assert(size0 == size);
  return size;
}
