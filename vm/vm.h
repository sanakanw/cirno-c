#ifndef VM_H
#define VM_H

#define KB(B) (B * 1024)

#define MAX_STACK 64
#define MAX_CALL 64
#define MAX_MEM 64
#define MAX_CALL 64
#define MAX_FRAME 64

#include "../src/hash.h"

typedef enum instr_e instr_t;
typedef struct vm_s vm_t;
typedef struct bin_s bin_t;
typedef struct sym_s sym_t;
typedef struct call_s call_t;

enum instr_e {
  PUSH,
  ADD,
  SUB,
  MUL,
  DIV,
  LDR,
  STR,
  LBP,
  ENTER,
  LEAVE,
  CALL,
  RET,
  JMP,
  CMP,
  JE,
  JNE,
  JL,
  JG,
  JLE,
  JGE,
  SETE,
  SETNE,
  SETL,
  SETG,
  SETLE,
  SETGE
};

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

struct vm_s {
  bin_t *bin;
  int ip, sp, bp, cp, fp;
  int f_gtr, f_lss, f_equ;
  int mem[MAX_MEM];
  int stack[MAX_STACK];
  int call[MAX_CALL];
  int frame[MAX_FRAME];
  int *s_i32;
  int *m_i32;
};

void bin_dump(bin_t *bin);

vm_t *make_vm();
void vm_load(vm_t *vm, bin_t *bin);
void vm_exec(vm_t *vm);

#endif
