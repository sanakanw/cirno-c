#include "p_local.h"

#include <stdlib.h>

unit_t *make_unit()
{
  return malloc(sizeof(unit_t));
}

unit_t *translation_unit()
{
  func_t *body, *head;
  body = head = func_declaration();
  while (head)
    head = head->next = func_declaration();
  
  unit_t *unit = make_unit();
  unit->func = body;
  return unit;
}
