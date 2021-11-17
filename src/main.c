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
  
  bin_dump(bin);
  
  vm_t *vm = make_vm();
  vm_load(vm, bin);
  
  vm_exec(vm);
  
  printf("[3] executed\n");
  
  for (int i = MAX_MEM - 1; i >= MAX_MEM - 8; i--)
    printf("%i %i\n", i * sizeof(int), vm->mem[i]);
  
  return 0;
}
