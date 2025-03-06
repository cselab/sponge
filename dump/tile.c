#include <float.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Config {
  int tile[3], Verbose;
  char *input_path, *output_path;
};

enum { TABLE_INT, TABLE_PCHAR };
static const struct {
  const char *name;
  int type;
  long offset;
} Table[] = {
    {"tx", TABLE_INT, offsetof(struct Config, tile[0])},
    {"ty", TABLE_INT, offsetof(struct Config, tile[1])},
    {"tz", TABLE_INT, offsetof(struct Config, tile[2])},
    {"input", TABLE_PCHAR, offsetof(struct Config, input_path)},
    {"output", TABLE_PCHAR, offsetof(struct Config, output_path)},
};

int main(int argc, char **argv) {
  char *end;
  long size, ntri, nver, i, n, d;
  struct Config config;
  char input_attr_path[FILENAME_MAX], input_tri_path[FILENAME_MAX],
      input_xyz_path[FILENAME_MAX], output_attr_path[FILENAME_MAX],
      output_tri_path[FILENAME_MAX], output_xyz_path[FILENAME_MAX],
      output_xdmf_path[FILENAME_MAX];
  char *xyz_base, *attr_base, *tri_base;
  FILE *input_attr_file, *input_tri_file, *input_xyz_file, *output_attr_file,
      *output_tri_file, *output_xyz_file, *output_xdmf_file;
  int *index, t[3], tri[3];
  float *xyz, x, y, z, tlo[3], thi[3];
  float lo[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
  float hi[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(stderr,
              "Usage: tile [-h] [-v] <tx> <ty> <tz> input.xdmf2 output\n\n"
              "Options:\n"
              "  -h          Display this help message and exit\n"
              "  -v          Enable verbose mode\n\n"
              "Arguments:\n"
              "  tx, ty, tz    Tile in corresponding dimension\n"
              "  input.xdmf2   Input file\n"
              "  output        Output prefix\n");
      exit(1);
    case 'v':
      config.Verbose = 1;
      break;
    default:
      fprintf(stderr, "tile: error: unknown option '%s'\n", *argv);
      exit(1);
    }
  for (i = 0; i < sizeof(Table) / sizeof(*Table); i++) {
    if (*argv == NULL) {
      fprintf(stderr, "tile: error: missing '%s' option\n", Table[i].name);
      exit(1);
    }
    switch (Table[i].type) {
    case TABLE_INT:
      *(int *)((void *)&config + Table[i].offset) = strtol(*argv, &end, 10);
      if (*end != '\0') {
        fprintf(stderr, "tile: error: '%s' is not an integer\n", *argv);
        exit(1);
      }
      break;
    case TABLE_PCHAR:
      *(char **)((void *)&config + Table[i].offset) = *argv;
      break;
    }
    argv++;
  }
  // fprintf(stderr, "%d %d %d\n", config.tile[0], config.tile[1],
  // config.tile[2]);

  strcpy(input_attr_path, config.input_path);
  strcpy(input_tri_path, config.input_path);
  strcpy(input_xyz_path, config.input_path);

  n = strlen(config.input_path);
  for (i = n; i != 0; i--) {
    if (config.input_path[i] == '.')
      break;
  }
  if (i == 0)
    i = n;
  strcpy(&input_attr_path[i], ".attr.raw");
  strcpy(&input_tri_path[i], ".tri.raw");
  strcpy(&input_xyz_path[i], ".xyz.raw");
  fprintf(stderr, "[%s]\n", input_xyz_path);
  fprintf(stderr, "[%s]\n", input_attr_path);
  fprintf(stderr, "[%s]\n", input_tri_path);

  if ((input_tri_file = fopen(input_tri_path, "r")) == NULL) {
    fprintf(stderr, "tile: error: fail to open '%s'\n", input_tri_path);
    exit(1);
  }
  if (fseek(input_tri_file, 0, SEEK_END) != 0 ||
      (size = ftell(input_tri_file)) == -1 ||
      fseek(input_tri_file, 0, SEEK_SET) != 0) {
    fprintf(stderr, "tile: error: fail to seek '%s'\n", input_tri_path);
    exit(1);
  }
  ntri = size / (3 * sizeof(int));
  fprintf(stderr, "ntri: %ld\n", ntri);
  if ((input_xyz_file = fopen(input_xyz_path, "r")) == NULL) {
    fprintf(stderr, "tile: error: fail to open '%s'\n", input_xyz_path);
    exit(1);
  }
  if ((input_attr_file = fopen(input_attr_path, "r")) == NULL) {
    fprintf(stderr, "tile: error: fail to open '%s'\n", input_attr_path);
    exit(1);
  }
  if (fseek(input_xyz_file, 0, SEEK_END) != 0 ||
      (size = ftell(input_xyz_file)) == -1 ||
      fseek(input_xyz_file, 0, SEEK_SET) != 0) {
    fprintf(stderr, "tile: error: fail to seek '%s'\n", input_xyz_path);
    exit(1);
  }
  if ((xyz = malloc(size)) == NULL) {
    fprintf(stderr, "tile: error: malloc failed\n");
    exit(1);
  }
  if (fread(xyz, size, 1, input_xyz_file) != 1) {
    fprintf(stderr, "stl2dump: error: fail to read '%s'\n", input_xyz_path);
    exit(1);
  }

  nver = size / (3 * sizeof *xyz);
  fprintf(stderr, "nver: %ld\n", nver);
  for (i = 0; i < nver; i++)
    for (d = 0; d < 3; d++) {
      x = xyz[3 * i + d];
      if (x < lo[d])
        lo[d] = x;
      if (x > hi[d])
        hi[d] = x;
    }
  fprintf(stderr, "%g %g %g\n", lo[0], lo[1], lo[2]);
  fprintf(stderr, "%g %g %g\n", hi[0], hi[1], hi[2]);

  if ((index = malloc(nver * sizeof *index)) == NULL) {
    fprintf(stderr, "tile: error: malloc failed\n");
    exit(1);
  }
  for (t[0] = 0; t[0] < config.tile[0]; t[0]++)
    for (t[1] = 0; t[1] < config.tile[1]; t[1]++)
      for (t[2] = 0; t[2] < config.tile[2]; t[2]++) {

        snprintf(output_xyz_path, sizeof output_xyz_path,
                 "%s.%03d.%03d.%03d.xyz.raw", config.output_path, t[0], t[1],
                 t[2]);
        snprintf(output_attr_path, sizeof output_attr_path,
                 "%s.%03d.%03d.%03d.attr.raw", config.output_path, t[0], t[1],
                 t[2]);
        snprintf(output_tri_path, sizeof output_tri_path,
                 "%s.%03d.%03d.%03d.tri.raw", config.output_path, t[0], t[1],
                 t[2]);
        snprintf(output_xdmf_path, sizeof output_xdmf_path,
                 "%s.%03d.%03d.%03d.xdmf2", config.output_path, t[0], t[1],
                 t[2]);

        xyz_base = output_xyz_path;
        attr_base = output_attr_path;
        tri_base = output_tri_path;
        for (i = 0; output_xyz_path[i] != '\0'; i++) {
          if (output_xyz_path[i] == '/' && output_xyz_path[i + 1] != '\0') {
            xyz_base = &output_xyz_path[i + 1];
            attr_base = &output_attr_path[i + 1];
            tri_base = &output_tri_path[i + 1];
          }
        }
        if ((output_tri_file = fopen(output_tri_path, "w")) == NULL) {
          fprintf(stderr, "tile: error: fail to open '%s'\n", output_tri_path);
          exit(1);
        }
        if ((output_attr_file = fopen(output_attr_path, "w")) == NULL) {
          fprintf(stderr, "tile: error: fail to open '%s'\n", output_attr_path);
          exit(1);
        }
        if ((output_xyz_file = fopen(output_xyz_path, "w")) == NULL) {
          fprintf(stderr, "tile: error: fail to open '%s'\n", output_xyz_path);
          exit(1);
        }
        if ((output_xdmf_file = fopen(output_xdmf_path, "w")) == NULL) {
          fprintf(stderr, "tile: error: fail to open '%s'\n", output_xdmf_path);
          exit(1);
        }

        for (d = 0; d < 3; d++) {
          tlo[d] = lo[d] + (hi[d] - lo[d]) * t[d] / config.tile[d];
          thi[d] = lo[d] + (hi[d] - lo[d]) * (t[d] + 1) / config.tile[d];
        }
        fprintf(stderr, "\n");
        fprintf(stderr, "%g %g %g\n", tlo[0], tlo[1], tlo[2]);
        fprintf(stderr, "%g %g %g\n", thi[0], thi[1], thi[2]);

        memset(index, 0, nver * sizeof *index);
        if (fseek(input_tri_file, 0, SEEK_SET) != 0) {
          fprintf(stderr, "tile: error: fail to seek '%s'\n", input_tri_path);
          exit(1);
        }
        for (i = 0; i < ntri; i++) {
          if (fread(tri, sizeof tri, 1, input_tri_file) != 1) {
            fprintf(stderr, "stl2dump: error: fail to read '%s'\n",
                    input_tri_path);
            exit(1);
          }
          if (tri[0] == 0)
            fprintf(stderr, "%d %d %d\n", tri[0], tri[1], tri[2]);
        }

        if (fclose(output_tri_file) != 0) {
          fprintf(stderr, "dump_select: error: fail to close '%s'\n",
                  output_tri_path);
          exit(1);
        }
        if (fclose(output_attr_file) != 0) {
          fprintf(stderr, "dump_select: error: fail to close '%s'\n",
                  output_attr_path);
          exit(1);
        }
        if (fclose(output_xyz_file) != 0) {
          fprintf(stderr, "dump_select: error: fail to close '%s'\n",
                  output_xyz_path);
          exit(1);
        }
        if (fclose(output_xdmf_file) != 0) {
          fprintf(stderr, "dump_select: error: fail to close '%s'\n",
                  output_xdmf_path);
          exit(1);
        }
      }
  free(index);
  free(xyz);
  if (fclose(input_tri_file) != 0) {
    fprintf(stderr, "dump_select: error: fail to close '%s'\n", input_tri_path);
    exit(1);
  }
  if (fclose(input_xyz_file) != 0) {
    fprintf(stderr, "dump_select: error: fail to close '%s'\n", input_xyz_path);
    exit(1);
  }
  if (fclose(input_attr_file) != 0) {
    fprintf(stderr, "dump_select: error: fail to close '%s'\n",
            input_attr_path);
    exit(1);
  }
}
