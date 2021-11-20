#include "bin.h"

#include <stdlib.h>

typedef enum tlump_e tlump_t;
typedef struct lump_s lump_t;
typedef struct header_s header_t;

enum tlump_e {
  LUMP_SYM,
  LUMP_INSTR,
  MAX_LUMP
};

struct lump_s {
  int fileofs;
  int filelen;
};

struct header_s {
  lump_t lumps[MAX_LUMP];
};

static const char *instr_tbl[] = {
  "push",
  "add",
  "sub",
  "mul",
  "div",
  "ldr",
  "str",
  "lbp",
  "enter",
  "leave",
  "call",
  "ret",
  "jmp",
  "cmp",
  "je",
  "jne",
  "jl",
  "jg",
  "jle",
  "jge",
  "sete",
  "setne",
  "setl",
  "setg",
  "setle",
  "setge"
};

bin_t *make_bin(instr_t *instr, int num_instr, sym_t *sym, int num_sym)
{
  bin_t *bin = malloc(sizeof(bin_t));
  bin->instr = instr;
  bin->sym = sym;
  bin->num_instr = num_instr;
  bin->num_sym = num_sym;
  return bin;
}

void bin_dump(bin_t *bin)
{
  int i = 0;
  while (i < bin->num_instr) {
    switch (bin->instr[i]) {
    case PUSH:
    case ENTER:
    case CALL:
    case JMP:
    case JE:
    case JNE:
    case JL:
    case JG:
    case JLE:
    case JGE:
      printf("%03i %s %i\n", i, instr_tbl[bin->instr[i]], bin->instr[i + 1]);
      i += 2;
      break;
    default:
      printf("%03i %s\n", i, instr_tbl[bin->instr[i]]);
      i += 1;
      break;
    }
  }
}

void bin_write(bin_t *bin, FILE *out)
{
  header_t header;
  
  int size_sym = bin->num_sym * sizeof(sym_t);
  int size_instr = bin->num_instr * sizeof(instr_t);
  
  fseek(out, sizeof(header_t), SEEK_SET);
  
  header.lumps[LUMP_SYM].fileofs = ftell(out);
  header.lumps[LUMP_SYM].filelen = size_sym;
  fwrite(bin->sym, 1, size_sym, out);
  
  header.lumps[LUMP_INSTR].fileofs = ftell(out);
  header.lumps[LUMP_INSTR].filelen = size_instr;
  fwrite(bin->instr, 1, size_instr, out);
  
  fseek(out, 0, SEEK_SET);
  fwrite(&header, 1, sizeof(header_t), out);
}

bin_t *bin_read(FILE *in)
{
  header_t header;
  fread(&header, 1, sizeof(header_t), in);
  
  int size_sym = header.lumps[LUMP_SYM].filelen;
  sym_t *sym = malloc(size_sym);
  fread(sym, 1, size_sym, in);
  
  int size_instr = header.lumps[LUMP_INSTR].filelen;
  instr_t *instr = malloc(size_instr);
  fread(instr, 1, size_instr, in);
  
  return make_bin(instr, size_instr / sizeof(instr_t), sym, size_sym / sizeof(sym_t));
}
