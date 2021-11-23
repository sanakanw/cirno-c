#ifndef PARSE_H
#define PARSE_H

#include "../common/map.h"
#include "../common/hash.h"
#include "lex.h"

typedef enum tspec_e tspec_t;
typedef enum tdcltr_e tdcltr_t;
typedef struct spec_s spec_t;
typedef struct dcltr_s dcltr_t;
typedef struct type_s type_t;
typedef struct decl_s decl_t;
typedef struct func_s func_t;
typedef struct param_s param_t;
typedef struct struct_decl_s struct_decl_t;
typedef struct struct_scope_s struct_scope_t;

typedef enum operator_e operator_t;
typedef enum texpr_e texpr_t;
typedef enum taddr_e taddr_t;
typedef struct expr_s expr_t;

typedef enum tstmt_e tstmt_t;
typedef struct stmt_s stmt_t;

typedef struct unit_s unit_t;

enum tspec_e {
  TY_U0,
  TY_I8,
  TY_I32,
  TY_STRUCT,
  TY_FUNC
};

enum tdcltr_e {
  DCLTR_ARRAY,
  DCLTR_POINTER
};

struct spec_s {
  tspec_t tspec;
  struct_scope_t *struct_scope;
};

struct dcltr_s {
  tdcltr_t type;
  
  int size;
  
  dcltr_t *next;
};

struct type_s {
  spec_t *spec;
  dcltr_t *dcltr;
};

struct decl_s {
  type_t type;
  hash_t name;
  
  int offset;
};

struct struct_scope_s {
  struct_decl_t *list;
  int size;
};

struct struct_decl_s {
  type_t type;
  hash_t name;
  
  int offset;
  
  struct_decl_t *next;
};

struct param_s {
  type_t type;
  expr_t *addr;
  param_t *next;
};

struct func_s {
  hash_t name;
  type_t type;
  stmt_t *body;
  param_t *params;
  int local_size;
  func_t *next;
};


enum operator_e {
  OPERATOR_ADD,
  OPERATOR_SUB,
  OPERATOR_MUL,
  OPERATOR_DIV,
  OPERATOR_ASSIGN,
  OPERATOR_OR,
  OPERATOR_AND,
  OPERATOR_EQ,
  OPERATOR_NE,
  OPERATOR_LSS,
  OPERATOR_GTR,
  OPERATOR_LE,
  OPERATOR_GE
};

enum texpr_e {
  EXPR_CONST,
  EXPR_ADDR,
  EXPR_LOAD,
  EXPR_BINOP,
  EXPR_FUNC,
  EXPR_CALL,
  EXPR_ARG,
  EXPR_CAST
};

enum taddr_e {
  ADDR_GLOBAL,
  ADDR_LOCAL
};

enum tstmt_e {
  STMT_EXPR,
  STMT_IF,
  STMT_WHILE,
  STMT_RETURN
};

struct expr_s {
  union {
    int num;
    struct {
      expr_t *base;
      taddr_t taddr;
    } addr;
    struct {
      expr_t *base;
    } unary;
    struct {
      operator_t op;
      expr_t *lhs, *rhs;
    } binop;
    struct {
      expr_t *base, *post;
    } post;
    struct {
      func_t *func;
    } func;
    struct {
      expr_t *base;
      expr_t *next;
    } arg;
  };
  type_t type;
  texpr_t texpr;
};

struct stmt_s {
  union {
    expr_t *expr;
    struct {
      expr_t *cond;
      stmt_t *body;
    } if_stmt;
    struct {
      expr_t *cond;
      stmt_t *body;
    } while_stmt;
    struct {
      expr_t *value;
    } ret_stmt;
  };
  tstmt_t tstmt;
  stmt_t *next;
};

struct unit_s {
  func_t *func;
};

void decl_init();

unit_t *translation_unit();

#endif
