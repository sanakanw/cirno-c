#ifndef BIN_H
#define BIN_H

#include "instr.h"
#include "hash.h"
#include <stdio.h>

typedef struct bin_s bin_t;
typedef struct sym_s sym_t;

struct bin_s {
  instr_t *instr;
  sym_t *sym;
  int num_instr;
  int num_sym;
};

struct sym_s {
  hash_t name;
  int pos;
};

void bin_dump(bin_t *bin);
void bin_write(bin_t *bin, FILE *out);
bin_t *bin_read(FILE *in);

bin_t *make_bin(instr_t *instr, int num_instr, sym_t *sym, int num_sym);

#endif
