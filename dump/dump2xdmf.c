#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FREAD(ptr, size, nmemb)                                                \
  if (fread(ptr, size, nmemb, input_file) != (uint64_t)(nmemb)) {              \
    fprintf(stderr, "dump2xdmf: error: fail to read from '%s'\n", input_path); \
    exit(1);                                                                   \
  }
static void traverse(int, void *);
static void process(int, void *);
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
static const double shift[8][3] = {
    {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1},
    {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1},
};
struct Context {
  struct DumpHeader header;
  int *index;
  double *values;
  int m_alloc_level, maxlevel;
  double X0, Y0, Z0, L0;
  FILE *xyz_file, *attr_file;
  char xyz_path[FILENAME_MAX], attr_path[FILENAME_MAX];
  long ncell_total;
};
int main(int argc, char **argv) {
  FILE *file;
  long i, j, nvect, nattr;
  unsigned len;
  int Verbose;
  double o[4];
  char xdmf_path[FILENAME_MAX], *xyz_base, *attr_base, **names, *output_path,
      *end;
  struct Context context;
  Verbose = 0;
  context.maxlevel = -1;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(stderr, "Usage: dump2xdmf [-h] [-v] [-l int] file.dump output\n"
                      "Options:\n"
                      "  -h          print help message and exit\n"
                      "  -v          verbose\n"
                      "  -l <int>    maximum resolution level\n"
                      "  file.dump   basilisk dump\n"
                      "  ouput       output file prefix\n");
      exit(1);
    case 'v':
      Verbose = 1;
      break;
    case 'l':
      argv++;
      if (*argv == NULL) {
        fprintf(stderr, "dump2xdmf: error: -l needs an argument\n");
        exit(1);
      }
      context.maxlevel = strtol(*argv, &end, 10);
      if (*end != '\0' || context.maxlevel < 0) {
        fprintf(stderr, "dump2xdmf: error: '%s' is integer >= 0\n", *argv);
        exit(1);
      }
      break;
    default:
      fprintf(stderr, "dump2xdmf: error: unknown option '%s'\n", *argv);
      exit(1);
    }
  if ((input_path = argv[0]) == NULL) {
    fprintf(stderr, "dump2xdmf: error: file.dump is not set\n");
    exit(1);
  }
  if ((output_path = argv[1]) == NULL) {
    fprintf(stderr, "dump2xdmf: error: output is not set\n");
    exit(1);
  }
  if ((input_file = fopen(input_path, "r")) == NULL) {
    fprintf(stderr, "dump2xdmf: error: fail to open '%s'\n", input_path);
    exit(1);
  }
  snprintf(context.xyz_path, sizeof context.xyz_path, "%s.xyz.raw",
           output_path);
  snprintf(context.attr_path, sizeof context.attr_path, "%s.attr.raw",
           output_path);
  snprintf(xdmf_path, sizeof xdmf_path, "%s.xdmf2", output_path);
  xyz_base = context.xyz_path;
  attr_base = context.attr_path;
  for (j = 0; context.xyz_path[j] != '\0'; j++) {
    if (context.xyz_path[j] == '/' && context.xyz_path[j + 1] != '\0') {
      xyz_base = &context.xyz_path[j + 1];
      attr_base = &context.attr_path[j + 1];
    }
  }
  FREAD(&context.header, sizeof context.header, 1);
  if (Verbose)
    fprintf(stderr,
            "version:             dump version: %d\n"
            "      t:          simulation time: %.16e\n"
            "    len:          numer of fields: %ld\n"
            "    npe:     number of processors: %d\n"
            "  depth:          multigrid depth: %d\n"
            "      i:     simulation iteration: %d\n"
            "      n: multigrid MPI dimensions: [%g %g %g]\n",
            context.header.version, context.header.t, context.header.len,
            context.header.npe, context.header.depth, context.header.i,
            context.header.n.x, context.header.n.y, context.header.n.z);
  if ((names = malloc(context.header.len * sizeof *names)) == NULL) {
    fprintf(stderr, "dump_info: error: malloc failed\n");
    exit(1);
  }
  for (i = 0; i < context.header.len; i++) {
    FREAD(&len, sizeof len, 1);
    if ((names[i] = malloc((len + 1) * sizeof *names[i])) == NULL) {
      fprintf(stderr, "dump2xdmf: malloc failed\n");
      exit(1);
    }
    FREAD(names[i], sizeof *names[i], len);
    names[i][len] = '\0';
  }
  if (Verbose)
    for (i = 0; i < context.header.len; i++)
      fprintf(stderr, "   f[%ld]: %s\n", i, names[i]);
  FREAD(o, sizeof o, 1);
  if (Verbose)
    fprintf(stderr,
            " origin: [%.16e %.16e %.16e]\n"
            "   size: %.16e\n",
            o[0], o[1], o[2], o[3]);
  context.X0 = o[0];
  context.Y0 = o[1];
  context.Z0 = o[2];
  context.L0 = o[3];
  context.m_alloc_level = 0;
  context.index = NULL;
  if ((context.values = malloc(context.header.len * sizeof *context.values)) ==
      NULL) {
    fprintf(stderr, "dump_info: error: malloc failed\n");
    exit(1);
  }
  if ((context.xyz_file = fopen(context.xyz_path, "w")) == NULL) {
    fprintf(stderr, "%s:%d: fail to open '%s'\n", __FILE__, __LINE__,
            context.xyz_path);
    exit(1);
  }
  if ((context.attr_file = fopen(context.attr_path, "w")) == NULL) {
    fprintf(stderr, "%s:%d: fail to open '%s'\n", __FILE__, __LINE__,
            context.attr_path);
    exit(1);
  }
  context.ncell_total = 0;
  traverse(0, &context);
  if (Verbose)
    fprintf(stderr, "ncell_total: %ld\n", context.ncell_total);
  if (fclose(context.xyz_file) != 0) {
    fprintf(stderr, "dump2xdmf: error: fail to close '%s'\n", context.xyz_path);
    exit(1);
  }
  if (Verbose) {
    fprintf(stderr, "dump2xdmf: write: %s\n", context.xyz_path);
    fprintf(stderr, "dump2xdmf: write: %s\n", context.attr_path);
    fprintf(stderr, "dump2xdmf: write: %s\n", xdmf_path);
  }

  if (fclose(context.attr_file) != 0) {
    fprintf(stderr, "dump2xdmf: error: fail to close '%s'\n",
            context.attr_path);
    exit(1);
  }
  free(context.index);
  free(context.values);
  if (fclose(input_file) != 0) {
    fprintf(stderr, "dump2xdmf: error: fail to close '%s'\n", input_path);
    exit(1);
  }
  if ((file = fopen(xdmf_path, "w")) == NULL) {
    fprintf(stderr, "%s:%d: fail to open '%s'\n", __FILE__, __LINE__,
            xdmf_path);
    exit(1);
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
          context.ncell_total, 8 * context.ncell_total, xyz_base);
  j = 0;
  nvect = 0;
  nattr = context.header.len;
  for (i = 0; i < context.header.len; i++)
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
            names[i], context.ncell_total, j++, nattr + 3 * nvect,
            context.ncell_total, (nattr + 3 * nvect) * context.ncell_total,
            attr_base);
  fprintf(file, "    </Grid>\n"
                "  </Domain>\n"
                "</Xdmf>\n");
  fclose(file);
  for (i = 0; i < context.header.len; i++)
    free(names[i]);
  free(names);
}

static void process(int level, void *vcontext) {
  int i, j;
  double Delta, x, y, z;
  float xyz[8 * 3];
  struct Context *context;
  context = vcontext;

  x = context->X0 + context->L0 / 2;
  y = context->Y0 + context->L0 / 2;
  z = context->Z0 + context->L0 / 2;
  for (i = 1; i <= level; i++) {
    Delta = context->L0 * (1. / (1 << i));
    x += Delta * (shift[context->index[i]][0] - 0.5);
    y += Delta * (shift[context->index[i]][1] - 0.5);
    z += Delta * (shift[context->index[i]][2] - 0.5);
  }
  if (level == 0)
    fprintf(stderr, "%g %g %g\n", x, y, z);
  Delta = context->L0 * (1. / (1 << level));
  j = 0;
  for (i = 0; i < 8; i++) {
    xyz[j++] = x + Delta * (shift[i][0] - 0.5);
    xyz[j++] = y + Delta * (shift[i][1] - 0.5);
    xyz[j++] = z + Delta * (shift[i][2] - 0.5);
  }
  if (fwrite(xyz, sizeof xyz, 1, context->xyz_file) != 1) {
    fprintf(stderr, "dump2xdmf: failed to write coordinates: %s\n",
            context->xyz_path);
    exit(1);
  }
  if (fwrite(context->values, sizeof *context->values, context->header.len,
             context->attr_file) != (size_t)context->header.len) {
    fprintf(stderr, "dump2xdmf: failed to write attributes: %s\n",
            context->attr_path);
    exit(1);
  }
  context->ncell_total++;
}

static void traverse(int level, void *vcontext) {
  enum {
    leaf = 1 << 1,
  };
  unsigned flags;
  struct Context *context;

  context = vcontext;
  FREAD(&flags, sizeof flags, 1);
  FREAD(context->values, sizeof *context->values, context->header.len);
  if ((flags & leaf) ||
      (context->maxlevel != -1 && level >= context->maxlevel)) {
    process(level, context);
  } else {
    while (level + 1 >= context->m_alloc_level) {
      context->m_alloc_level = 2 * context->m_alloc_level + 1;
      context->index = realloc(context->index,
                               context->m_alloc_level * sizeof *context->index);
      if (context->index == NULL) {
        fprintf(stderr, "dump2xdmf: realloc failed\n");
        exit(1);
      }
    }
    for (context->index[level + 1] = 0; context->index[level + 1] < 8;
         context->index[level + 1]++)
      traverse(level + 1, context);
  }
}
