#include <float.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Config {
  int tile[3], Box, Verbose;
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
  long size, ntri, nver, tile_ntri, tile_nver, i, j, n, d;
  struct Config config;
  char input_attr_path[FILENAME_MAX], input_tri_path[FILENAME_MAX],
      input_xyz_path[FILENAME_MAX], output_attr_path[FILENAME_MAX],
      output_tri_path[FILENAME_MAX], output_xyz_path[FILENAME_MAX],
      output_xdmf_path[FILENAME_MAX], box_path[FILENAME_MAX];
  char *xyz_base, *attr_base, *tri_base;
  FILE *input_attr_file, *input_tri_file, *input_xyz_file, *output_attr_file,
      *output_tri_file, *output_xyz_file, *output_xdmf_file, *box_file;
  int *index, t[3], tri[3];
  float *xyz, *attr, x, y, z, tlo[3], thi[3];
  float lo[3] = {FLT_MAX, FLT_MAX, FLT_MAX};
  float hi[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

  config.Verbose = 0;
  config.Box = 0;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(stderr,
              "Usage: tile [-h] [-v] [-b] <tx> <ty> <tz> input.xdmf2 output\n\n"
              "Options:\n"
              "  -b          Dump also a box\n"
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
    case 'b':
      config.Box = 1;
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
  if (config.Verbose) {
    fprintf(stderr, "tile: [%s]\n", input_xyz_path);
    fprintf(stderr, "tile: [%s]\n", input_attr_path);
    fprintf(stderr, "tile: [%s]\n", input_tri_path);
  }

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
  if (config.Verbose)
    fprintf(stderr, "tile: ntri: %ld\n", ntri);
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
  nver = size / (3 * sizeof *xyz);
  if ((xyz = malloc(size)) == NULL) {
    fprintf(stderr, "tile: error: malloc failed\n");
    exit(1);
  }
  if ((attr = malloc(size)) == NULL) {
    fprintf(stderr, "tile: error: malloc failed\n");
    exit(1);
  }
  if (fread(xyz, size, 1, input_xyz_file) != 1) {
    fprintf(stderr, "tile: error: fail to read '%s'\n", input_xyz_path);
    exit(1);
  }
  if (fread(attr, nver * sizeof *attr, 1, input_attr_file) != 1) {
    fprintf(stderr, "tile: error: fail to read '%s'\n", input_attr_path);
    exit(1);
  }
  if (config.Verbose)
    fprintf(stderr, "tile: nver: %ld\n", nver);
  for (i = 0; i < nver; i++)
    for (d = 0; d < 3; d++) {
      x = xyz[3 * i + d];
      if (x < lo[d])
        lo[d] = x;
      if (x > hi[d])
        hi[d] = x;
    }
  if (config.Verbose) {
    fprintf(stderr, "tile: %g %g %g\n", lo[0], lo[1], lo[2]);
    fprintf(stderr, "tile: %g %g %g\n", hi[0], hi[1], hi[2]);
  }

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
        if (config.Verbose) {
          fprintf(stderr, "tile: %d %d %d\n", t[0], t[1], t[2]);
          fprintf(stderr, "tile: %g %g %g\n", tlo[0], tlo[1], tlo[2]);
          fprintf(stderr, "tile: %g %g %g\n", thi[0], thi[1], thi[2]);
        }

        memset(index, 0, nver * sizeof *index);
        if (fseek(input_tri_file, 0, SEEK_SET) != 0) {
          fprintf(stderr, "tile: error: fail to seek '%s'\n", input_tri_path);
          exit(1);
        }
        tile_ntri = 0;
        tile_nver = 0;
        for (i = 0; i < ntri; i++) {
          if (fread(tri, sizeof tri, 1, input_tri_file) != 1) {
            fprintf(stderr, "tile: error: fail to read '%s'\n", input_tri_path);
            exit(1);
          }
          if (tlo[0] <= xyz[3 * tri[0] + 0] && xyz[3 * tri[0] + 0] <= thi[0] &&
              tlo[1] <= xyz[3 * tri[0] + 1] && xyz[3 * tri[0] + 1] <= thi[1] &&
              tlo[2] <= xyz[3 * tri[0] + 2] && xyz[3 * tri[0] + 2] <= thi[2]) {
            for (d = 0; d < 3; d++) {
              if (index[tri[d]] == 0) {
                if (fwrite(&xyz[3 * tri[d]], 3 * sizeof *xyz, 1,
                           output_xyz_file) != 1) {
                  fprintf(stderr, "tile: error: fail to write '%s'\n",
                          output_xyz_path);
                  exit(1);
                }
                if (fwrite(&attr[tri[d]], sizeof *attr, 1, output_attr_file) !=
                    1) {
                  fprintf(stderr, "tile: error: fail to write '%s'\n",
                          output_attr_path);
                  exit(1);
                }
                tile_nver++;
                index[tri[d]] = tile_nver;
              }
              tri[d] = index[tri[d]] - 1;
            }
            if (fwrite(tri, sizeof tri, 1, output_tri_file) != 1) {
              fprintf(stderr, "tile: error: fail to write '%s'\n",
                      output_tri_path);
              exit(1);
            }
            tile_ntri++;
          }
        }
        if (config.Verbose) {
	  fprintf(stderr, "tile: writing: %s\n", output_xdmf_path);
          fprintf(stderr, "tile: ntri, nver: %ld %ld\n", tile_ntri, tile_nver);
	}
        fprintf(output_xdmf_file,
                "<Xdmf\n"
                "    Version=\"2\">\n"
                "  <Domain>\n"
                "    <Grid>\n"
                "      <Topology\n"
                "         TopologyType=\"Triangle\"\n"
                "         Dimensions=\"%ld\">\n"
                "        <DataItem\n"
                "            Dimensions=\"%ld 3\"\n"
                "            NumberType=\"Int\"\n"
                "            Format=\"Binary\">\n"
                "          %s\n"
                "        </DataItem>\n"
                "      </Topology>\n"
                "      <Geometry>\n"
                "        <DataItem\n"
                "            Dimensions=\"%ld 3\"\n"
                "            Precision=\"4\"\n"
                "            Format=\"Binary\">\n"
                "          %s\n"
                "        </DataItem>\n"
                "      </Geometry>\n"
                "      <Attribute\n"
                "          Center=\"Node\"\n"
                "          Name=\"u\">\n"
                "        <DataItem\n"
                "            Dimensions=\"%ld\"\n"
                "            Precision=\"4\"\n"
                "            Format=\"Binary\">\n"
                "          %s\n"
                "        </DataItem>\n"
                "      </Attribute>\n"
                "    </Grid>\n"
                "  </Domain>\n"
                "</Xdmf>\n",
                tile_ntri, tile_ntri, tri_base, tile_nver, xyz_base, tile_nver,
                attr_base);

        if (fclose(output_tri_file) != 0) {
          fprintf(stderr, "tile: error: fail to close '%s'\n", output_tri_path);
          exit(1);
        }
        if (fclose(output_attr_file) != 0) {
          fprintf(stderr, "tile: error: fail to close '%s'\n",
                  output_attr_path);
          exit(1);
        }
        if (fclose(output_xyz_file) != 0) {
          fprintf(stderr, "tile: error: fail to close '%s'\n", output_xyz_path);
          exit(1);
        }
        if (fclose(output_xdmf_file) != 0) {
          fprintf(stderr, "tile: error: fail to close '%s'\n",
                  output_xdmf_path);
          exit(1);
        }
      }
  free(index);
  free(xyz);
  free(attr);
  if (fclose(input_tri_file) != 0) {
    fprintf(stderr, "tile: error: fail to close '%s'\n", input_tri_path);
    exit(1);
  }
  if (fclose(input_xyz_file) != 0) {
    fprintf(stderr, "tile: error: fail to close '%s'\n", input_xyz_path);
    exit(1);
  }
  if (fclose(input_attr_file) != 0) {
    fprintf(stderr, "tile: error: fail to close '%s'\n", input_attr_path);
    exit(1);
  }

  if (config.Box) {
    snprintf(box_path, sizeof box_path, "%s.box.xdmf2", config.output_path);
    if (config.Verbose)
      fprintf(stderr, "tile: writing: %s\n", box_path);
    if ((box_file = fopen(box_path, "w")) == NULL) {
      fprintf(stderr, "tile: error: fail to open '%s'\n", box_path);
      exit(1);
    }
    fprintf(box_file,
            "<Xdmf\n"
            "    Version=\"2\">\n"
            "  <Domain>\n"
            "    <Grid>\n"
            "      <Topology\n"
            "	  TopologyType=\"3DCoRectMesh\"\n"
            "	  Dimensions=\"2 2 2\"/>\n"
            "      <Geometry\n"
            "	  GeometryType=\"ORIGIN_DXDYDZ\">\n"
            "	<DataItem\n"
            "	    Dimensions=\"3\">\n"
            "	  %.16e\n"
            "	  %.16e\n"
            "	  %.16e\n"
            "	</DataItem>\n"
            "	<DataItem\n"
            "	    Dimensions=\"3\">\n"
            "	  %.16e\n"
            "	  %.16e\n"
            "	  %.16e\n"
            "	</DataItem>\n"
            "      </Geometry>\n"
            "    </Grid>\n"
            "  </Domain>\n"
            "</Xdmf>\n",
            lo[2], lo[1], lo[0], hi[2] - lo[2], hi[1] - lo[1], hi[0] - lo[0]);
    if (fclose(box_file) != 0) {
      fprintf(stderr, "tile: error: fail to close '%s'\n", box_path);
      exit(1);
    }
  }
}
