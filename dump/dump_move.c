#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FREAD(ptr, size, nmemb, file, path)                                    \
  if (fread(ptr, size, nmemb, file) != (uint64_t)(nmemb)) {                    \
    fprintf(stderr, "dump_move: error: fail to read from '%s'\n", path);       \
    exit(1);                                                                   \
  }

static FILE *from_file, *to_file, *output_file;
static char *from_path, *to_path, *output_path;
struct coord {
  double x, y, z;
};
struct DumpHeader {
  double t;
  long len;
  int i, depth, npe, version;
  struct coord n;
};
static struct DumpHeader from_header, to_header;
static double *values;
static long traverse(int);
static double X0, Y0, Z0, L0;
static long nleaf;
int main(int argc, char **argv) {
  long i;
  unsigned len;
  double o[4];
  char **from_names, *to_name;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(stderr,
              "Usage: dump_info [-h] [-v] from.dump to.dump output.dump\n"
              "Options:\n"
              "  -h                               Print help message and exit\n"
              "  from.dump, to.dump, output.dump  basilisk dumps\n");
      exit(1);
    default:
      fprintf(stderr, "dump_move: error: unknown option '%s'\n", *argv);
      exit(1);
    }
  if ((from_path = *argv) == NULL) {
    fprintf(stderr, "dump_move: error: from.dump is not given\n");
    exit(1);
  }
  argv++;

  if ((to_path = *argv) == NULL) {
    fprintf(stderr, "dump_move: error: to.dump is not given\n");
    exit(1);
  }
  argv++;

  if ((output_path = *argv) == NULL) {
    fprintf(stderr, "dump_move: error: output.dump is not given\n");
    exit(1);
  }
  argv++;

  if ((from_file = fopen(from_path, "r")) == NULL) {
    fprintf(stderr, "dump_move: error: fail to open '%s'\n", from_path);
    exit(1);
  }

  if ((to_file = fopen(to_path, "r")) == NULL) {
    fprintf(stderr, "dump_move: error: fail to open '%s'\n", to_path);
    exit(1);
  }

  if ((output_file = fopen(output_path, "w")) == NULL) {
    fprintf(stderr, "dump_move: error: fail to open '%s'\n", output_path);
    exit(1);
  }

  FREAD(&from_header, sizeof from_header, 1, from_file, from_path);
  fprintf(stderr,
          "version:             dump version: %d\n"
          "      t:          simulation time: %.16e\n"
          "    len:          numer of fields: %ld\n"
          "    npe:     number of processors: %d\n"
          "  depth:          multigrid depth: %d\n"
          "      i:     simulation iteration: %d\n"
          "      n: multigrid MPI dimensions: [%g %g %g]\n",
          from_header.version, from_header.t, from_header.len, from_header.npe,
          from_header.depth, from_header.i, from_header.n.x, from_header.n.y,
          from_header.n.z);
  if ((from_names = malloc(from_header.len * sizeof *from_names)) == NULL) {
    fprintf(stderr, "dump_move: error: malloc failed\n");
    exit(1);
  }
  for (i = 0; i < from_header.len; i++) {
    FREAD(&len, sizeof len, 1, from_file, from_path);
    from_names[i] = malloc((len + 1) * sizeof *from_names[i]);
    FREAD(from_names[i], sizeof *from_names[i], len, from_file, from_path);
    from_names[i][len] = '\0';
    fprintf(stderr, "name: %s\n", from_names[i]);
  }
  FREAD(o, sizeof o, 1, from_file, from_path);
  fprintf(stderr,
          " origin: [%.16e %.16e %.16e]\n"
          "   size: %.16e\n",
          o[0], o[1], o[2], o[3]);
  X0 = o[0];
  Y0 = o[1];
  Z0 = o[2];
  L0 = o[3];
  FREAD(&to_header, sizeof to_header, 1, to_file, to_path);
  if (fwrite(&from_header, sizeof(from_header), 1, output_file) != 1) {
    fprintf(stderr, "dump_move: error: fail to write '%s'\n", output_path);
    exit(1);
  }
  assert(from_header.len == to_header.len);
  assert(from_header.version == to_header.version);
  for (i = 0; i < to_header.len; i++) {
    FREAD(&len, sizeof len, 1, to_file, to_path);
    to_name = malloc((len + 1) * sizeof *to_name);
    FREAD(to_name, sizeof *to_name, len, to_file, to_path);
    to_name[len] = '\0';

    if (fwrite(&len, sizeof len, 1, output_file) != 1) {
      fprintf(stderr, "dump_move: error: fail to write '%s'\n", output_path);
      exit(1);
    }
    if (fwrite(to_name, sizeof *to_name, len, output_file) != len) {
      fprintf(stderr, "dump_move: error: fail to write '%s'\n", output_path);
      exit(1);
    }
    assert(strcmp(to_name, from_names[i]) == 0);
    free(to_name);
  }
  FREAD(o, sizeof o, 1, to_file, to_path);
  assert(X0 == o[0]);
  assert(Y0 == o[1]);
  assert(Z0 == o[2]);
  assert(L0 == o[3]);
  if (fwrite(o, sizeof o, 1, output_file) != 1) {
    fprintf(stderr, "dump_move: error: fail to write '%s'\n", output_path);
    exit(1);
  }
  if ((values = malloc(from_header.len * sizeof *values)) == NULL) {
    fprintf(stderr, "dump_move: error: malloc failed\n");
    exit(1);
  }
  nleaf = 0;
  traverse(0);
  fprintf(stderr, "nleaf: %ld\n", nleaf);
  for (i = 0; i < from_header.len; i++)
    free(from_names[i]);
  free(from_names);
  if (fclose(from_file) != 0) {
    fprintf(stderr, "dump_move: error: fail to close '%s'\n", from_path);
    exit(1);
  }
  if (fclose(to_file) != 0) {
    fprintf(stderr, "dump_move: error: fail to close '%s'\n", to_path);
    exit(1);
  }
  if (fclose(output_file) != 0) {
    fprintf(stderr, "dump_move: error: fail to close '%s'\n", output_path);
    exit(1);
  }
}
static void process(int level, unsigned flags) {
  if (fwrite(&flags, sizeof flags, 1, output_file) != 1) {
    fprintf(stderr, "dump_move: error: fail to write '%s'\n", output_path);
    exit(1);
  }
  if (fwrite(values, sizeof *values, from_header.len, output_file) !=
      from_header.len) {
    fprintf(stderr, "dump_move: error: fail to write '%s'\n", output_path);
    exit(1);
  }
  nleaf++;
}
static long traverse(int level) {
  enum { leaf = 2 };
  unsigned flags, i;
  long size, size0;

  if (fread(&flags, sizeof flags, 1, from_file) != 1 ||
      fread(values, sizeof *values, from_header.len, from_file) !=
          from_header.len) {
    fprintf(stderr, "dump_move: fail to read '%s' at level '%d'\n", from_path,
            level);
    exit(1);
  }
  size = values[0];
  size0 = 1;
  // if (flags & leaf)
  process(level, flags);
  if (flags & leaf) {
    /* */
  } else {
    for (i = 0; i < 8; i++)
      size0 += traverse(level + 1);
  }
  assert(size0 == size);
  return size;
}
