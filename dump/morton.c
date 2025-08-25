#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
static uint64_t left(uint64_t x) {
  x = (x | x << 32) & 0x1f00000000ffffull;
  x = (x | x << 16) & 0x1f0000ff0000ffull;
  x = (x | x << 8) & 0x100f00f00f00f00full;
  x = (x | x << 4) & 0x10c30c30c30c30c3ull;
  x = (x | x << 2) & 0x1249249249249249ull;
  return x;
}
static uint64_t morton(uint64_t x, uint64_t y, uint64_t z) {
  return (left(z) << 2) | (left(y) << 1) | (left(x) << 0);
}
int main(int argc, char **argv) {
  uint64_t x, y, z;
  char *end;
  x = strtoull(*++argv, &end, 10);
  y = strtoull(*++argv, &end, 10);
  z = strtoull(*++argv, &end, 10);
  printf("[%ld %ld %ld] %ld\n", x, y, z, morton(x, y, z));
}
