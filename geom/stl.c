#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  char *input_path, header[80];
  FILE *input_file;
  uint32_t i, nf;
  int Ignore, Verbose;
  float normal[3], vert[3 * 3];
  uint16_t count;

  Ignore = 0;
  Verbose = 0;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(stderr, "stl [-i] [-v] file.stl\n");
      exit(1);
    case 'i':
      Ignore = 1;
      break;
    case 'v':
      Verbose = 1;
      break;
    default:
      fprintf(stderr, "stl: error: unrecognized command-line option '%s'\n",
              *argv);
      exit(1);
    }

  input_path = *argv++;
  if (input_path == NULL) {
    fprintf(stderr, "stl: error: not input file\n");
    exit(1);
  }
  if ((input_file = fopen(input_path, "r")) == NULL) {
    fprintf(stderr, "stl: error: cannot open '%s'\n", input_path);
    exit(1);
  }

  if (fread(header, 1, sizeof header, input_file) != sizeof header) {
    fprintf(stderr, "stl: error: fail to read '%s'\n", input_path);
    exit(1);
  }
  if (Ignore == 0 && header[0] == 's' && header[1] == 'o' && header[2] == 'l' &&
      header[3] == 'i' && header[4] == 'd') {
    fprintf(stderr, "stl: error: STL ASCII format is not supported\n");
    exit(1);
  }

  if (fread(&nf, sizeof nf, 1, input_file) != 1) {
    fprintf(stderr, "stl: error: fail to read '%s'\n", input_path);
    exit(1);
  }
  if (Verbose)
    fprintf(stderr, "%" PRIu32 "\n", nf);
  for (i = 0; i < nf; i++) {
    if (fread(&normal, sizeof normal, 1, input_file) != 1) {
      fprintf(stderr, "stl: error: fail to read '%s'\n", input_path);
      exit(1);
    }
    if (fread(&vert, sizeof vert, 1, input_file) != 1) {
      fprintf(stderr, "stl: error: fail to read '%s'\n", input_path);
      exit(1);
    }
    if (fread(&count, sizeof count, 1, input_file) != 1) {
      fprintf(stderr, "stl: error: fail to read '%s'\n", input_path);
      exit(1);
    }
  }

  if (fclose(input_file) != 0) {
    fprintf(stderr, "stl: error: cannot clone '%s'\n", input_path);
    exit(1);
  }
}
