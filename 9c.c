#include <stdio.h>
#include <stdlib.h>

#include "cc/lex.h"
#include "cc/gen.h"
#include "cc/parse.h"
#include "vm/vm.h"

int main(int argc, char **argv)
{
  if (argc != 2) {
    printf("usage: %s [file]\n", argv[0]);
    exit(-1);
  }
  
  FILE *in = fopen(argv[1], "rb");
  
  hash_init();
  decl_init();
  
  lexify(in, "test.c");
  
  unit_t *unit = translation_unit();
  // printf("[1] translated\n");
  
  bin_t *bin = gen(unit);
  // printf("[2] compiled\n");
  
  // FILE *out = fopen("test.out", "wb");
  // bin_write(bin, out); 
  
  vm_t *vm = make_vm();
  vm_load(vm, bin);
  
  vm_exec(vm);
  
  // printf("[3] done\n");
  
  // fclose(out);
  fclose(in);
  
  return 0;
}
