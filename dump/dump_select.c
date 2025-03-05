#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FREAD(ptr, size, nmemb)                                                \
  if (fread(ptr, size, nmemb, input_file) != (uint64_t)(nmemb)) {              \
    fprintf(stderr, "dump_select: error: fail to read from '%s'\n",            \
            input_path);                                                       \
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
static int m_level;
static long traverse(int);
static double X0, Y0, Z0, L0;
static const double shift[8][3] = {
    {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1},
    {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1},
};
static FILE *xyz_file, *attr_file;
static long ncell_total;

int main(int argc, char **argv) {
  FILE *file;
  long i, j, nvect, nattr;
  unsigned len;
  int Verbose;
  double o[4];
  char xyz_path[FILENAME_MAX], attr_path[FILENAME_MAX], xdmf_path[FILENAME_MAX],
      *xyz_base, *attr_base, **names, *output_path;
  Verbose = 0;
  output_path = NULL;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(stderr, "Usage: dump_select [-h] [-v] -o select file.dump\n"
                      "Options:\n"
                      "  -h          Print help message and exit\n"
                      "  -v          Verbose\n"
                      "  -o <string> Prefix for the output files\n"
                      "  file.dump   basilisk dump\n");
      exit(1);
    case 'v':
      Verbose = 1;
      break;
    case 'o':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "dump_select: error: -o needs an argument\n");
        exit(1);
      }
      output_path = *argv;
      break;
    default:
      fprintf(stderr, "dump_select: error: unknown option '%s'\n", *argv);
      exit(1);
    }
  if (output_path == NULL) {
    fprintf(stderr, "dump_select: error: output path (-o) is not set\n");
    exit(1);
  }
  if ((input_path = argv[0]) == NULL) {
    fprintf(stderr, "dump_select: error: file.dump is not set\n");
    exit(1);
  }
  if ((input_file = fopen(input_path, "r")) == NULL) {
    fprintf(stderr, "dump_select: error: fail to open '%s'\n", input_path);
    exit(1);
  }
  snprintf(xyz_path, sizeof xyz_path, "%s.xyz.raw", output_path);
  snprintf(attr_path, sizeof attr_path, "%s.attr.raw", output_path);
  snprintf(xdmf_path, sizeof xdmf_path, "%s.xdmf2", output_path);
  xyz_base = xyz_path;
  attr_base = attr_path;
  for (j = 0; xyz_path[j] != '\0'; j++) {
    if (xyz_path[j] == '/' && xyz_path[j + 1] != '\0') {
      xyz_base = &xyz_path[j + 1];
      attr_base = &attr_path[j + 1];
    }
  }
  FREAD(&header, sizeof header, 1);
  if (Verbose) {
    fprintf(stderr, "verbose: %d\n", header.version);
    fprintf(stderr, "t: %g\n", header.t);
    fprintf(stderr, "len: %ld\n", header.len);
    fprintf(stderr, "npe: %d\n", header.npe);
    fprintf(stderr, "depth: %d\n", header.depth);
    fprintf(stderr, "i: %d\n", header.i);
    fprintf(stderr, "n: [%g %g %g]\n", header.n.x, header.n.y, header.n.z);
  }
  if ((names = malloc(header.len * sizeof *names)) == NULL) {
    fprintf(stderr, "dump_info: error: malloc failed\n");
    exit(1);
  }
  for (i = 0; i < header.len; i++) {
    FREAD(&len, sizeof len, 1);
    names[i] = malloc((len + 1) * sizeof *names[i]);
    FREAD(names[i], sizeof *names[i], len);
    names[i][len] = '\0';
  }
  if (Verbose)
    for (i = 0; i < header.len; i++)
      fprintf(stderr, "[%ld]: %s\n", i, names[i]);
  FREAD(o, sizeof o, 1);
  if (Verbose) {
    fprintf(stderr, "origin: [%g %g %g]\n", o[0], o[1], o[2]);
    fprintf(stderr, "size: %g\n", o[3]);
  }
  X0 = o[0];
  Y0 = o[1];
  Z0 = o[2];
  L0 = o[3];
  m_level = 0;
  index = NULL;
  if ((values = malloc(header.len * sizeof *values)) == NULL) {
    fprintf(stderr, "dump_info: error: malloc failed\n");
    exit(1);
  }
  if ((xyz_file = fopen(xyz_path, "w")) == NULL) {
    fprintf(stderr, "%s:%d: fail to open '%s'\n", __FILE__, __LINE__, xyz_path);
    return 1;
  }
  if ((attr_file = fopen(attr_path, "w")) == NULL) {
    fprintf(stderr, "%s:%d: fail to open '%s'\n", __FILE__, __LINE__,
            attr_path);
    return 1;
  }
  ncell_total = 0;
  traverse(0);
  fprintf(stderr, "ncell_total: %ld\n", ncell_total);
  if (fclose(xyz_file) != 0) {
    fprintf(stderr, "dump_select: error: fail to close '%s'\n", xyz_path);
    exit(1);
  }
  if (fclose(attr_file) != 0) {
    fprintf(stderr, "dump_select: error: fail to close '%s'\n", attr_path);
    exit(1);
  }
  free(index);
  free(values);
  if (fclose(input_file) != 0) {
    fprintf(stderr, "dump_select: error: fail to close '%s'\n", input_path);
    exit(1);
  }
  if ((file = fopen(xdmf_path, "w")) == NULL) {
    fprintf(stderr, "%s:%d: fail to open '%s'\n", __FILE__, __LINE__,
            xdmf_path);
    return 1;
  }
  fprintf(file,
          "<Xdmf\n"
          "    Version=\"2\">\n"
          "  <Domain>\n"
          "    <Grid>\n"
          "      <Topology\n"
          "          TopologyType=\"Hexahedron\"\n"
          "          Order=\"0 1 3 2 4 5 7 6\"\n"
          "          Dimensions=\"%ld\"/>\n"
          "      <Geometry>\n"
          "        <DataItem\n"
          "            Dimensions=\"%ld 3\"\n"
          "            Format=\"Binary\">\n"
          "          %s\n"
          "        </DataItem>\n"
          "      </Geometry>\n",
          ncell_total, 8 * ncell_total, xyz_base);
  j = 0;
  nvect = 0;
  nattr = header.len;
  for (i = 0; i < header.len; i++)
    fprintf(file,
            "      <Attribute\n"
            "          Name=\"%s\"\n"
            "          Center=\"Cell\">\n"
            "        <DataItem\n"
            "            ItemType=\"HyperSlab\"\n"
            "            Dimensions=\"%ld\"\n"
            "            Type=\"HyperSlab\">\n"
            "          <DataItem Dimensions=\"3 1\">\n"
            "            %ld %ld %ld\n"
            "          </DataItem>\n"
            "          <DataItem\n"
            "              Precision=\"8\"\n"
            "              Dimensions=\"%ld\"\n"
            "              Format=\"Binary\">\n"
            "            %s\n"
            "          </DataItem>\n"
            "         </DataItem>\n"
            "      </Attribute>\n",
            names[i], ncell_total, j++, nattr + 3 * nvect, ncell_total,
            (nattr + 3 * nvect) * ncell_total, attr_base);
  fprintf(file, "    </Grid>\n"
                "  </Domain>\n"
                "</Xdmf>\n");
  fclose(file);
  for (i = 0; i < header.len; i++)
    free(names[i]);
  free(names);
}

static void process(int level) {
  int i, j;
  double Delta, l2, x, y, z, epsilon;
  float xyz[8 * 3];
  x = X0 + L0 / 2;
  y = Y0 + L0 / 2;
  z = Z0 + L0 / 2;
  for (i = 1; i <= level; i++) {
    Delta = L0 * (1. / (1 << i));
    x += Delta * (shift[index[i]][0] - 0.5);
    y += Delta * (shift[index[i]][1] - 0.5);
    z += Delta * (shift[index[i]][2] - 0.5);
  }
  l2 = values[8];
  epsilon = Delta / 10;
  if (l2 > -0.60 && l2 <= -0.40) {
    // if (z <= -epsilon && z + Delta + epsilon >= 0) {
    j = 0;
    for (i = 0; i < 8; i++) {
      xyz[j++] = x + Delta * (shift[i][0] - 0.5);
      xyz[j++] = y + Delta * (shift[i][1] - 0.5);
      xyz[j++] = z + Delta * (shift[i][2] - 0.5);
    }
    if (fwrite(xyz, sizeof xyz, 1, xyz_file) != 1) {
      fprintf(stderr, "dump_select: failed to write coordinates\n");
      exit(1);
    }
    if (fwrite(values, sizeof *values, header.len, attr_file) !=
        (size_t)header.len) {
      fprintf(stderr, "dump_select: failed to write attributes\n");
      exit(1);
    }
    ncell_total++;
  }
}

static long traverse(int level) {
  enum {
    leaf = 1 << 1,
  };
  unsigned flags;
  double val;
  long size, size0;
  int i;
  FREAD(&flags, sizeof flags, 1);
  FREAD(values, sizeof *values, header.len);
  size = values[0];
  size0 = 1;
  if (flags & leaf)
    process(level);
  if (flags & leaf) {
    /* */
  } else {
    while (level + 1 >= m_level) {
      m_level = 2 * m_level + 1;
      index = realloc(index, m_level * sizeof *index);
    }
    for (index[level + 1] = 0; index[level + 1] < 8; index[level + 1]++)
      size0 += traverse(level + 1);
  }
  assert(size0 == size);
  return size;
}
