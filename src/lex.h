#ifndef LEX_H
#define LEX_H

#include "hash.h"
#include <stdio.h>

#define MAX_STR 1024
#define MAX_TMP 32
#define MIN_AVAILABLE 8

typedef struct lex_s lex_t;
typedef enum token_e token_t;

enum token_e {
  TK_CONSTANT = 128,
  TK_STRING_LITERAL,
  TK_IDENTIFIER,
  TK_RIGHT_ASSIGN,
  TK_LEFT_ASSIGN,
  TK_ADD_ASSIGN,
  TK_SUB_ASSIGN,
  TK_MUL_ASSIGN,
  TK_DIV_ASSIGN,
  TK_MOD_ASSIGN,
  TK_AND_ASSIGN,
  TK_XOR_ASSIGN, 
  TK_OR_ASSIGN, 
  TK_RIGHT_OP,  
  TK_LEFT_OP,
  TK_INC_OP,
  TK_DEC_OP,
  TK_PTR_OP,
  TK_AND_OP,
  TK_OR_OP,
  TK_LE_OP,
  TK_GE_OP,
  TK_EQ_OP,
  TK_NE_OP,
  TK_FN,
  TK_I32,
  TK_IF,
  TK_WHILE,
  TK_RETURN,
  TK_BREAK,
  TK_ELSE
};

struct lex_s {
  FILE *file;
  const char *filename;
  
  int tmp_buf[MAX_TMP];
  int *c;
  
  char str_buf[MAX_STR];
  char *str_ptr;
  
  int line_no;
  
  token_t token;
  int     token_num;
  hash_t  token_hash;
  char    *token_str;
};

extern lex_t lex;

void lexify(FILE *file, const char *filename);
void next();
void match(token_t tok);
void token_error(const char *fmt, ...);
void token_warning(const char *fmt, ...);

#endif
