#include "p_local.h"

#include <stdlib.h>

unit_t *make_unit(func_t *func, stmt_t *stmt, int global_size)
{
  unit_t *unit = malloc(sizeof(unit_t));
  unit->func = func;
  unit->stmt = stmt;
  unit->global_size = global_size;
  return unit;
}

unit_t *translation_unit()
{
  func_t *func_body = NULL, *func_head, *func;
  stmt_t *stmt_body = NULL, *stmt_head = NULL, *stmt = NULL;
  
  while (lex.token != EOF) {
    if ((func = func_declaration())) {
      if (func_body)
        func_head = func_head->next = func;
      else
        func_body = func_head = func;
    } else if ((stmt = statement())) {
      if (stmt_body)
        stmt_head = stmt_head->next = stmt;
      else
        stmt_body = stmt_head = stmt;
    } else {
      struct_declaration();
    }
  }
  
  return make_unit(func_body, stmt_body, scope_global->size);
}
