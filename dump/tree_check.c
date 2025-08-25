@include <stdint.h>
@include <stdlib.h>
@include <string.h>
#include "grid/octree.h"
#include "fractions.h"
#include "embed.h"
#include "navier-stokes/centered.h"
#include "lambda2.h"
#include "embed.h"
static vector omega[];
static scalar phi[], l2[];
int main(int argc, char **argv) {
  char *dump_path;
  FILE *dump_file;
  int Verbose;
  Verbose = 0;
  while (*++argv != NULL && argv[0][0] == '-')
    switch (argv[0][1]) {
    case 'h':
      fprintf(stderr,
              "tree_check - Check the consistency of a Basilisk dump file.\n\n"
              "Usage: tree_check [-h] [-v] <basilisk.dump>\n\n"
              "Options:\n"
              "  -h        Show this help message and exit\n"
              "  -v        Verbose output (lists all fields and prints tree "
              "stats)\n\n"
              "Arguments:\n"
              "  <dump>    Path to Basilisk .dump file to be loaded and "
              "verified\n");
      exit(1);
    case 'v':
      Verbose = 1;
      break;
    default:
      fprintf(stderr, "tree_check: error: unknown option '%s'\n", *argv);
      exit(1);
    }
  if ((dump_path = *argv) == NULL) {
    fprintf(stderr, "tree_check: error: no dump file given\n");
    exit(1);
  }
  if (Verbose) {
    fprintf(stderr, "tree_check: starting on %d ranks\n", npe());
    for (scalar s in all)
      fprintf(stderr, "tree_check: %s\n", s.name);
  }
  if ((dump_file = fopen(dump_path, "r")) == NULL) {
    fprintf(stderr, "tree_check: error: failed to open '%s'\n", dump_path);
    exit(1);
  }
  restore(fp = dump_file);
  u.n[left] = dirichlet(1);
  p[left] = neumann(0);
  pf[left] = neumann(0);
  u.n[right] = neumann(0);
  p[right] = dirichlet(0);
  pf[right] = dirichlet(0);
  u.n[embed] = dirichlet(0);
  u.t[embed] = dirichlet(0);
  u.r[embed] = dirichlet(0);
  if (Verbose && pid() == 0)
    fprintf(stderr, "tree_check: rank grid->n: %d %ld\n", npe(), grid->n);
  fractions(phi, cs, fs);
  if (Verbose)
    fields_stats();
  tree_check();
  run();
  if (Verbose)
    fprintf(stderr, "tree_check: done\n");
}
