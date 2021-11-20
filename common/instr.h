#ifndef INSTR_H
#define INSTR_H

typedef enum instr_e instr_t;

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

#endif
