#ifndef VM_H
#define VM_H

#define KB(B) (B * 1024)

#define MAX_STACK 64
#define MAX_CALL 64
#define MAX_MEM 64
#define MAX_CALL 64
#define MAX_FRAME 64

#include "../common/bin.h"
#include "../common/hash.h"
#include "../common/instr.h"

typedef struct vm_s vm_t;
typedef struct call_s call_t;

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

vm_t *make_vm();
void vm_load(vm_t *vm, bin_t *bin);
void vm_exec(vm_t *vm);

#endif
