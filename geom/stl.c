#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(stderr, "stl file.stl\n");
      exit(1);
    default:
      fprintf(stderr, "stl: error: unrecognized command-line option '%s'\n", *argv);
      exit(1);
    }

  

}
