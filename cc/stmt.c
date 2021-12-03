#include "p_local.h"

#include <stdlib.h>

stmt_t *make_stmt()
{
  return malloc(sizeof(stmt_t));
}

stmt_t *make_expr_stmt(expr_t *expr)
{
  stmt_t *stmt = make_stmt();
  stmt->tstmt = STMT_EXPR;
  stmt->expr = expr;
  stmt->next = NULL;
  return stmt;
}

stmt_t *make_while_stmt(expr_t *cond, stmt_t *body)
{
  stmt_t *stmt = make_stmt();
  stmt->tstmt = STMT_WHILE;
  stmt->while_stmt.cond = cond;
  stmt->while_stmt.body = body;
  stmt->next = NULL;
  return stmt;
}

stmt_t *make_if_stmt(expr_t *cond, stmt_t *body, stmt_t *next_if, stmt_t *else_body)
{
  stmt_t *stmt = make_stmt();
  stmt->tstmt = STMT_IF;
  stmt->if_stmt.cond = cond;
  stmt->if_stmt.body = body;
  stmt->if_stmt.next_if = next_if;
  stmt->if_stmt.else_body = else_body;
  stmt->next = NULL;
  return stmt;
}

stmt_t *make_ret_stmt(expr_t *value)
{
  stmt_t *stmt = make_stmt();
  stmt->tstmt = STMT_RETURN;
  stmt->ret_stmt.value = value;
  stmt->next = NULL;
  return stmt;
}

stmt_t *make_inline_asm_stmt(char *code)
{
  stmt_t *stmt = make_stmt();
  stmt->tstmt = STMT_INLINE_ASM;
  stmt->inline_asm_stmt.code = code;
  stmt->next = NULL;
  return stmt;
}

stmt_t *statement()
{
  stmt_t *stmt = NULL;
  if ((stmt = if_statement())
  || (stmt = while_statement())
  || (stmt = compound_statement())
  || (stmt = return_statement())
  || (stmt = inline_asm_statement())
  || (stmt = expression_statement()))
    return stmt;
  
  return NULL;
}

stmt_t *inline_asm_statement()
{
  if (lex.token != TK_ASM)
    return NULL;
  
  match(TK_ASM);
  
  match('(');
  char *code = lex.token_str;
  match(TK_STRING_LITERAL);
  match(')');
  match(';');
  
  return make_inline_asm_stmt(code);
}

stmt_t *compound_statement()
{
  if (lex.token != '{')
    return NULL;
  
  match('{');
  
  stmt_t *body, *head;
  body = head = statement();
  while (lex.token != '}')
    head = head->next = statement();
  
  match('}');
  
  return body;
}

stmt_t *return_statement()
{
  if (lex.token != TK_RETURN)
    return NULL;
  
  match(TK_RETURN);
  
  expr_t *value = NULL;
  
  if (lex.token != ';')
    value = expression();
  
  match(';');
  
  if (current_func->type.spec) {
    if (!value)
      token_error("expected return value");
    
    if (!is_type_match(&current_func->type, &value->type))
      token_error("type mismatch");
  } else {
    if (value)
      token_error("cannot return value in non-return function");
  }
  
  return make_ret_stmt(value);
}

stmt_t *if_statement()
{
  if (lex.token != TK_IF)
    return NULL;
  
  match(TK_IF);
  
  match('(');
  expr_t *cond = expression();
  match(')');
  
  if (!cond)
    token_error("expected expression");
  
  stmt_t *body = statement();
  
  if (lex.token == TK_ELSE) {
    match(TK_ELSE);
    
    if (lex.token == TK_IF)
      return make_if_stmt(cond, body, if_statement(), NULL);
    else
      return make_if_stmt(cond, body, NULL, statement());
  }
  
  return make_if_stmt(cond, body, NULL, NULL);
}

stmt_t *while_statement()
{
  if (lex.token != TK_WHILE)
    return NULL;
  
  match(TK_WHILE);
  
  match('(');
  expr_t *cond = expression();
  match(')');
  
  if (!cond)
    token_error("expected expression");
  
  stmt_t *body = statement();
  
  return make_while_stmt(cond, body);
}

stmt_t *expression_statement()
{
  expr_t *expr = NULL;
  if (!(expr = expression()))
    return NULL;
  
  match(';');
  
  return make_expr_stmt(expr);
}
