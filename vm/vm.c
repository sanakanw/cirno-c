#include "vm.h"

#include "../common/error.h"
#include <stdio.h>
#include <stdlib.h>

#define ALIGN_32(X) (X / 4)

int main(int argc, char **argv)
{
  FILE *in = fopen("test.out", "rb");
  
  hash_init();
  
  hash_value("main");
  
  bin_t *bin = bin_read(in);
  bin_dump(bin);
  
  vm_t *vm = make_vm();
  vm_load(vm, bin);
  
  vm_exec(vm);
  
  fclose(in);
  
  for (int i = MAX_MEM - 1; i >= MAX_MEM - 8; i--)
    printf("%i %i\n", i * sizeof(int), vm->mem[i]);
}

vm_t *make_vm()
{
  vm_t *vm = malloc(sizeof(vm_t));
  vm->ip = 0;
  vm->sp = 0;
  vm->bp = MAX_MEM * sizeof(int);
  vm->cp = 0;
  vm->fp = 0;
  vm->s_i32 = vm->stack;
  vm->m_i32 = vm->mem;
  return vm;
}

instr_t fetch(vm_t *vm)
{
  return vm->bin->instr[vm->ip++];
}

int pop(vm_t *vm)
{
  return vm->s_i32[--vm->sp];
}

static inline void vm_push(vm_t *vm, int i32)
{
  vm->s_i32[vm->sp++] = i32;
}

static inline void vm_add(vm_t *vm)
{
  vm->s_i32[vm->sp - 2] += vm->s_i32[vm->sp - 1];
  --vm->sp;
}

static inline void vm_sub(vm_t *vm)
{
  vm->s_i32[vm->sp - 2] -= vm->s_i32[vm->sp - 1];
  --vm->sp;
}

static inline void vm_mul(vm_t *vm)
{
  vm->s_i32[vm->sp - 2] *= vm->s_i32[vm->sp - 1];
  --vm->sp;
}

static inline void vm_div(vm_t *vm)
{
  vm->s_i32[vm->sp - 2] /= vm->s_i32[vm->sp - 1];
  --vm->sp;
}

static inline void vm_ldr(vm_t *vm)
{
  vm->s_i32[vm->sp - 1] = vm->m_i32[ALIGN_32(vm->s_i32[vm->sp - 1])];
}

static inline void vm_str(vm_t *vm)
{
  vm->m_i32[ALIGN_32(vm->s_i32[vm->sp - 1])] = vm->s_i32[vm->sp - 2];
  vm->sp -= 2;
}

static inline void vm_lbp(vm_t *vm)
{
  vm->s_i32[vm->sp++] = vm->bp;
}

static inline void vm_enter(vm_t *vm, int i32)
{
  vm->frame[vm->fp++] = vm->bp;
  vm->bp -= i32;
}

static inline void vm_leave(vm_t *vm)
{
  vm->bp = vm->frame[--vm->fp];
}

static inline void vm_call(vm_t *vm, int i32)
{
  vm->call[vm->cp++] = vm->ip;
  vm->ip = i32;
}

static inline void vm_ret(vm_t *vm)
{
  vm->ip = vm->call[--vm->cp];
}

static inline void vm_jmp(vm_t *vm, int i32)
{
  vm->ip = i32;
}

static inline void vm_cmp(vm_t *vm)
{
  int tmp = vm->s_i32[vm->sp - 2] - vm->s_i32[vm->sp - 1];
  vm->sp -= 2;
  
  vm->f_gtr = tmp > 0;
  vm->f_lss = tmp < 0;
  vm->f_equ = tmp == 0;
}

static inline void vm_je(vm_t *vm, int i32)
{
  if (vm->f_equ)
    vm->ip = i32;
}

static inline void vm_jne(vm_t *vm, int i32)
{
  if (!vm->f_equ)
    vm->ip = i32;
}

static inline void vm_jl(vm_t *vm, int i32)
{
  if (vm->f_lss)
    vm->ip = i32;
}

static inline void vm_jg(vm_t *vm, int i32)
{
  if (vm->f_gtr)
    vm->ip = i32;
}

static inline void vm_jle(vm_t *vm, int i32)
{
  if (vm->f_equ || vm->f_lss)
    vm->ip = i32;
}

static inline void vm_jge(vm_t *vm, int i32)
{
  if (vm->f_equ || vm->f_gtr)
    vm->ip = i32;
}

static inline void vm_sete(vm_t *vm)
{
  vm_push(vm, vm->f_equ);
}

static inline void vm_setne(vm_t *vm)
{
  vm_push(vm, !vm->f_equ);
}

static inline void vm_setl(vm_t *vm)
{
  vm_push(vm, vm->f_lss);
}

static inline void vm_setg(vm_t *vm)
{
  vm_push(vm, vm->f_gtr);
}

static inline void vm_setle(vm_t *vm)
{
  vm_push(vm, vm->f_equ || vm->f_lss);
}

static inline void vm_setge(vm_t *vm)
{
  vm_push(vm, vm->f_equ || vm->f_gtr);
}

void vm_load(vm_t *vm, bin_t *bin)
{
  vm->bin = bin;
  vm->ip = 0;
}

sym_t *find_sym(bin_t *bin, hash_t hash)
{
  for (int i = 0; i < bin->num_sym; i++)
    if (bin->sym[i].name == hash)
      return &bin->sym[i];
  
  return NULL;
}

void vm_exec(vm_t *vm)
{
  vm->ip = -1;
  
  sym_t *sym = find_sym(vm->bin, hash_value("main"));
  if (!sym)
    error("vm_exec", "could not find entrance point 'main()'");
  
  vm_call(vm, sym->pos);
  
  while (vm->ip != -1) {
    switch (fetch(vm)) {
    case PUSH:
      vm_push(vm, fetch(vm));
      break;
    case ENTER:
      vm_enter(vm, fetch(vm));
      break;
    case ADD:
      vm_add(vm);
      break;
    case SUB:
      vm_sub(vm);
      break;
    case MUL:
      vm_mul(vm);
      break;
    case DIV:
      vm_div(vm);
      break;
    case LDR:
      vm_ldr(vm);
      break;
    case STR:
      vm_str(vm);
      break;
    case LBP:
      vm_lbp(vm);
      break;
    case CALL:
      vm_call(vm, fetch(vm));
      break;
    case LEAVE:
      vm_leave(vm);
      break;
    case RET:
      vm_ret(vm);
      break;
    case JMP:
      vm_jmp(vm, fetch(vm));
      break;
    case CMP:
      vm_cmp(vm);
      break;
    case JE:
      vm_je(vm, fetch(vm));
      break;
    case JNE:
      vm_jne(vm, fetch(vm));
      break;
    case JL:
      vm_jl(vm, fetch(vm));
      break;
    case JG:
      vm_jg(vm, fetch(vm));
      break;
    case JLE:
      vm_jle(vm, fetch(vm));
      break;
    case JGE:
      vm_jge(vm, fetch(vm));
      break;
    case SETE:
      vm_sete(vm);
      break;
    case SETNE:
      vm_setne(vm);
      break;
    case SETL:
      vm_setl(vm);
      break;
    case SETG:
      vm_setg(vm);
      break;
    case SETLE:
      vm_setle(vm);
      break;
    case SETGE:
      vm_setge(vm);
      break;
    }
  }
}
