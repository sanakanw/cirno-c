#include "lex.h"

#include "../common/error.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define MAX_OP 4
#define MAX_WORD 32

typedef struct op_s op_t;
typedef struct keyword_s keyword_t;

struct op_s {
  const char key[MAX_OP];
  token_t token;
};

struct keyword_s {
  const char key[MAX_WORD];
  token_t token;
};

const char *token_str[] = {
  "constant",
  "string literal",
  "identifier",
  ">>=",
  "<<=",
  "+=",
  "-=",
  "*=",
  "/=",
  "%=",
  "&=",
  "^=", 
  "|=", 
  ">>",  
  "<<",
  "++",
  "--",
  "->",
  "&&",
  "||",
  "<=",
  ">=",
  "==",
  "!=",       
  "fn",
  "i8",
  "i32",
  "if",
  "while",
  "return",
  "break",
  "else"
};

op_t op_dict[] = {
  { ">>=",  TK_RIGHT_ASSIGN     }, 
  { "<<=",  TK_LEFT_ASSIGN      },
  { "+=",   TK_ADD_ASSIGN       },
  { "-=",   TK_SUB_ASSIGN       },
  { "*=",   TK_MUL_ASSIGN       },
  { "/=",   TK_DIV_ASSIGN       },
  { "%=",   TK_MOD_ASSIGN       },
  { "&=",   TK_AND_ASSIGN       },
  { "^=",   TK_XOR_ASSIGN       },
  { "|=",   TK_OR_ASSIGN        },
  { ">>",   TK_RIGHT_OP         },
  { "<<",   TK_LEFT_OP          },
  { "++",   TK_INC_OP           },
  { "--",   TK_DEC_OP           },
  { "->",   TK_PTR_OP           },
  { "&&",   TK_AND_OP           },
  { "||",   TK_OR_OP            },
  { "<=",   TK_LE_OP            },
  { ">=",   TK_GE_OP            },
  { "==",   TK_EQ_OP            },
  { "!=",   TK_NE_OP            },
  { ";",    ';'                 },
  { "{",    '{'                 },
  { "}",    '}'                 },
  { ",",    ','                 },
  { ":",    ':'                 },
  { "=",    '='                 },
  { "(",    '('                 },
  { ")",    ')'                 },
  { "[",    '['                 },
  { "]",    ']'                 },
  { ".",    '.'                 },
  { "&",    '&'                 },
  { "!",    '!'                 },
  { "~",    '~'                 },
  { "-",    '-'                 },
  { "+",    '+'                 },
  { "*",    '*'                 },
  { "/",    '/'                 },
  { "%",    '%'                 },
  { "<",    '<'                 },
  { ">",    '>'                 },
  { "^",    '^'                 },
  { "|",    '|'                 },
  { "?",    '?'                 },
  { ":",    ':'                 }
};

keyword_t keyword_dict[] = {
  { "fn",       TK_FN           },
  { "i8",       TK_I8           },
  { "i32",      TK_I32          },
  { "if",       TK_IF           },
  { "while",    TK_WHILE        },
  { "return",   TK_RETURN       },
  { "break",    TK_BREAK        },
  { "else",     TK_ELSE         },
  { "struct",   TK_STRUCT       }
};

const int op_dict_count = sizeof(op_dict) / sizeof(op_t);
const int keyword_dict_count = sizeof(keyword_dict) / sizeof(keyword_t);

lex_t lex;

void token_fprint(FILE *out, token_t token)
{
  if (token >= TK_CONSTANT)
    fprintf(out, "%s", token_str[token - TK_CONSTANT]);
  else
    fprintf(out, "%c", token);
}

void token_fprint_current(FILE *out)
{
  switch (lex.token) {
  case TK_CONSTANT:
    fprintf(out, "%i", lex.token_num);
    break;
  case TK_STRING_LITERAL:
    fprintf(out, "%s", lex.str_ptr);
    break;
  case TK_IDENTIFIER:
    fprintf(out, "%s", hash_get(lex.token_hash));
    break;
  default:
    token_fprint(out, lex.token);
    break;
  }
}

void token_fprintf(FILE *out, const char *fmt, va_list args)
{
  while (*fmt) {
    if (*fmt == '%') {
      switch (*++fmt) {
      case 'c':
        fprintf(out, "%c", va_arg(args, int));
        break;
      case 'i':
        fprintf(out, "%i", va_arg(args, int));
        break;
      case 's':
        fprintf(out, "%s", va_arg(args, char *));
        break;
      case 't':
        token_fprint(out, va_arg(args, token_t));
        break;
      case 'n':
        token_fprint_current(out);
        break;
      }
      
      *++fmt;
    } else {
      fprintf(out, "%c", *fmt++);
    }
  }
}

void token_warning(const char *fmt, ...)
{
  printf("%s:%i:warning: ", lex.filename, lex.line_no);
  
  va_list args;
  va_start(args, fmt);
  token_fprintf(stdout, fmt, args);
  va_end(args);
  
  printf("\n");
}

void token_error(const char *fmt, ...)
{
  fprintf(stderr, "%s:%i:error: ", lex.filename, lex.line_no);
  
  va_list args;
  va_start(args, fmt);
  token_fprintf(stderr, fmt, args);
  va_end(args);
  
  fprintf(stderr, "\n");
  exit(-1);
}

void populate_buffer()
{
  int bytes_old = lex.c - lex.tmp_buf;
  int available = MAX_TMP - bytes_old;
  
  if (available < MIN_AVAILABLE) {
    if (available)
      memmove(lex.tmp_buf, lex.c, available * sizeof(int));
    
    for (int i = 0; i < bytes_old; i++)
      lex.tmp_buf[available + i] = fgetc(lex.file);
    
    lex.c = lex.tmp_buf;
  }
}

void lex_putc(char c)
{
  *lex.str_ptr++ = c;
  
  if (lex.str_ptr - lex.str_buf >= MAX_STR)
    error("lex_putc", "ran out of memory");
}

int read_char()
{
  int c = *lex.c++;
  populate_buffer();
  
  return c;
}

int count(int n)
{
  lex.c += n;
  populate_buffer();
}

void match(token_t token)
{
  if (lex.token == token)
    next();
  else
    token_error("unexpected token '%n', expected token '%t'", token);
}

int is_digit(int c)
{
  return c >= '0' && c <= '9';
}

int to_digit(int c)
{
  if (!is_digit(c))
    error("to_digit", "cannot convert '%c' into digit.", c);
  
  return c - '0';
}

int is_letter(int c)
{
  return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

int read_escape_sequence()
{
  if (*lex.c != '\\')
    token_error("expected '\\'");
  
  count(1);
  
  int num = 0;
  switch (*lex.c) {
  case 'a':
    num = '\a';
    count(1);
    break;
  case 'b':
    num = '\b';
    count(1);
    break;
  case 'f':
    num = '\f';
    count(1);
    break;
  case 'n':
    num = '\n';
    count(1);
    break;
  case 'r':
    num = '\r';
    count(1);
    break;
  case 't':
    num = '\t';
    count(1);
    break;
  case 'v':
    num = '\v';
    count(1);
    break;
  case '\\':
    num = '\\';
    count(1);
    break;
  case '\'':
    num = '\'';
    count(1);
    break;
  case '\"':
    num = '\"';
    count(1);
    break;
  case '\?':
    num = '\?';
    count(1);
    break;
  default:
    if (is_digit(*lex.c)) {
      num = *lex.c;
      count(1);
      for (int i = 0; i < 2; i++) {
        if (is_digit(*lex.c))
          num = num * 10 + *lex.c;
        else
          break;
        count(1);
      }
    } else {
      token_error("invalid escape sequence");
    }
  }
  
  return num;
}

int read_constant()
{
  if (*lex.c == '\'') {
    count(1);
    
    if (*lex.c == '\'')
      token_error("empty character constant");
    
    if (*lex.c == '\\') {
      lex.token_num = read_escape_sequence();
    } else {
      lex.token_num = *lex.c;
      count(1);
    }
    
    if (*lex.c != '\'')
      token_error("multi-character chararacter constant");
    
    count(1);
    
    lex.token = TK_CONSTANT;
    
    return 1;
  } else if (is_digit(*lex.c)) {
    lex.token_num = to_digit(read_char());
    
    while (is_digit(*lex.c))
      lex.token_num = lex.token_num * 10 + to_digit(read_char());
    
    lex.token = TK_CONSTANT;
    
    return 1;
  }
  
  return 0;
}

int keyword_match(keyword_t *keyword, const char *word)
{
  for (int i = 0; i < strlen(keyword->key); i++) {
    if (word[i] != keyword->key[i])
      return 0;
  }
  
  return 1; 
}

int read_word()
{
  static char word[MAX_WORD];
  char *letter = &word[0];
  
  if (is_letter(*lex.c) || *lex.c == '_') {
    *letter++ = read_char();
    
    while (is_letter(*lex.c)
    || is_digit(*lex.c)
    || *lex.c == '_') {
      if (letter + 1 >= &word[MAX_WORD])
        error("read_word", "word exceeded length MAX_WORD: '%i'.", MAX_WORD);
      
      *letter++ = read_char();
    }
    
    *letter++ = '\0';
    
    hash_t word_hash = hash_value(word);
    
    for (int i = 0; i < keyword_dict_count; i++) {
      if (keyword_match(&keyword_dict[i], word)) {
        lex.token = keyword_dict[i].token;
        return 1;
      }
    }
    
    lex.token = TK_IDENTIFIER;
    lex.token_hash = word_hash;
    
    return 1;
  }
  
  return 0;
}

int op_match(op_t *op)
{
  for (int i = 0; i < strlen(op->key); i++) {
    if (lex.c[i] != op->key[i])
      return 0;
  }
  
  return 1;
}

int read_op()
{
  for (int i = 0; i < op_dict_count; i++) {
    if (op_match(&op_dict[i])) {
      count(strlen(op_dict[i].key));
      
      lex.token = op_dict[i].token;
      return 1;
    }
  }
  
  return 0;
}

int read_string_literal()
{
  if (*lex.c != '"')
    return 0;
  
  count(1);
  
  lex.token_str = lex.str_ptr;
  while (*lex.c != '"') {
    if (*lex.c == '\\') {
      lex_putc(read_escape_sequence());
    } else {
      lex_putc(*lex.c);
      count(1);
    }
  }
  
  lex_putc('\0');
  
  count(1);
  
  lex.token = TK_STRING_LITERAL;
  
  return 1;
}

int read_comment()
{
  if (lex.c[0] == '/'&& lex.c[1] == '/') {
    count(2);
    while (*lex.c != '\n')
      count(1);
  } else if (lex.c[0] == '/' && lex.c[1] == '*') {
    count(2);
    while (lex.c[0] != '*' && lex.c[1] != '/')
      count(1);
  } else {
    return 0;
  }
  
  return 1;
}

void reset_token()
{
  lex.token = 0;
  lex.token_num = 0;
  lex.token_hash = 0;
  lex.token_str = NULL;
}

void next()
{
  reset_token();
  
  while (1) {
    switch (*lex.c) {
    case EOF:
      lex.token = EOF;
      return;
    case '\n':
      ++lex.line_no;
    case ' ':
    case '\t':
      count(1);
      break;
    case '"':
      read_string_literal();
      break;
    default:
        if (read_comment()
        || read_constant()
        || read_word()
        || read_op())
          return;
      
      token_error("unknown character: '%c (%i)', ignoring.", *lex.c, *lex.c);
      read_char();
      
      break;
    }
  }
}

void lexify(FILE *file, const char *filename)
{
  lex.file = file;
  lex.filename = filename;
  
  lex.c = &lex.tmp_buf[MAX_TMP - 1];
  lex.str_ptr = lex.str_buf;
  
  lex.line_no = 1;
  
  count(1);
  next();
}

