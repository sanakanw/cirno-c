#include "p_local.h"

#include <stdlib.h>

unit_t *make_unit(func_t *func)
{
  unit_t *unit = malloc(sizeof(unit_t));
  unit->func = func;
  return unit;
}

unit_t *translation_unit()
{
  func_t *func_body = NULL, *func_head, *func;
  // stmt_t *stmt_body, *stmt_head, *stmt;
  
  while (lex.token != EOF) {
    if ((func = func_declaration())) {
      if (func_body)
        func_head = func_head->next = func;
      else
        func_body = func_head = func;
    } /* else if ((stmt = statement())) {
      if (stmt_body)
        stmt_body = stmt_head = stmt;
      else
        stmt_head = stmt_head->next = stmt;
    } */ else {
      struct_declarations();
    }
  }
  
  return make_unit(func_body);
}
