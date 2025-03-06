#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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
  int i;
  struct Config config;

  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(stderr, "Usage: tile [-h] [-v] <tx> <ty> <tz> input output\n\n"
                      "Options:\n"
                      "  -h          Display this help message and exit\n"
                      "  -v          Enable verbose mode\n\n"
                      "Arguments:\n"
                      "  tx, ty, tz    Tile in corresponding dimension\n"
                      "  input, output Input and output prefix\n");
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
}
