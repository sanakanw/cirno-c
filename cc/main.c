#include <stdio.h>

#include "lex.h"
#include "gen.h"
#include "parse.h"

int main(int argc, char **argv)
{
  FILE *in = fopen("test.c", "rb");
  
  hash_init();
  decl_init();
  
  lexify(in, "test.c");
  
  unit_t *unit = translation_unit();
  printf("[1] translated\n");
  
  bin_t *bin = gen(unit);
  printf("[2] compiled\n");
  
  FILE *out = fopen("test.out", "wb");
  
  bin_write(bin, out); 
  printf("[3]:done\n");
  
  fclose(out);
  fclose(in);
  
  return 0;
}
