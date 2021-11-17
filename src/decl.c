#include "p_local.h"

#define MAX_SPEC_CACHE 8

#include <stdlib.h>

spec_t *ty_u0;
spec_t *ty_i32;

map_t *scope_local;
map_t *scope_func;

func_t *current_func;

int local_offset = 0;

spec_t spec_cache[MAX_SPEC_CACHE];
int num_spec = 0;

void decl_init()
{
  num_spec = 0;
  local_offset = 0;
  
  scope_func = make_map();
  scope_local = make_map();
  
  ty_u0 = spec_cache_find(TY_U0);
  ty_i32 = spec_cache_find(TY_I32);
  
  current_func = NULL;
}

dcltr_t *make_dcltr()
{
  return malloc(sizeof(dcltr_t));
}

dcltr_t *make_dcltr_pointer(dcltr_t *next)
{
  dcltr_t *dcltr = make_dcltr();
  dcltr->type = DCLTR_POINTER;
  dcltr->next = next;
  return dcltr;
}

dcltr_t *make_dcltr_array(int size, dcltr_t *next)
{
  dcltr_t *dcltr = make_dcltr();
  dcltr->type = DCLTR_ARRAY;
  dcltr->size = size;
  dcltr->next = next;
  return dcltr;
}

decl_t *make_decl(spec_t *spec, dcltr_t *dcltr, int offset)
{
  decl_t *decl = malloc(sizeof(decl_t));
  decl->type.spec = spec;
  decl->type.dcltr = dcltr;
  decl->offset = offset;
  return decl;
}

param_t *make_param(spec_t *spec, dcltr_t *dcltr, expr_t *addr)
{
  param_t *param = malloc(sizeof(param_t));
  param->type.spec = spec;
  param->type.dcltr = dcltr;
  param->addr = addr;
  param->next = NULL;
  return param;
}

func_t *make_func(hash_t name, type_t *type, param_t *params, stmt_t *body, int local_size)
{
  func_t *func = malloc(sizeof(func_t));
  func->name = name;
  func->type.spec = type->spec;
  func->type.dcltr = type->dcltr;
  func->params = params;
  func->body = body;
  func->local_size = local_size;
  func->next = NULL;
  return func;
}

int is_type_match(type_t *lhs, type_t *rhs)
{
  if (!rhs
  || !lhs
  || !lhs->spec
  || !rhs->spec
  || lhs->spec->tspec != rhs->spec->tspec
  || lhs->spec->tag != rhs->spec->tag)
    return 0;
  
  if ((!lhs->dcltr && rhs->dcltr) || (lhs->dcltr && !rhs->dcltr))
    return 0;
  
  dcltr_t *lhs_dcltr = lhs->dcltr;
  dcltr_t *rhs_dcltr = rhs->dcltr;
  
  while (lhs_dcltr && rhs_dcltr) {
    if (lhs_dcltr->type != rhs_dcltr->type)
      return 0;
    
    if (lhs_dcltr->type == DCLTR_ARRAY) {
      if (lhs_dcltr->size != rhs_dcltr->size)
        return 0;
    }
    
    lhs_dcltr = lhs_dcltr->next;
    rhs_dcltr = rhs_dcltr->next;
    
    if ((!lhs->dcltr && rhs->dcltr) || (lhs->dcltr && !rhs->dcltr))
      return 0;
  }
  
  return 1;
}

int type_size(spec_t *spec, dcltr_t *dcltr)
{
  if (dcltr) {
    switch (dcltr->type) {
    case DCLTR_POINTER:
      return type_size(ty_i32, NULL);
    case DCLTR_ARRAY:
      return type_size(spec, dcltr->next) * dcltr->size;
    }
  } else {
    switch (spec->tspec) {
    case TY_U0:
      return 0;
    case TY_I32:
      return 4;
    }
  }
}

func_t *func_declaration()
{
  if (lex.token != TK_FN)
    return 0;
  
  match(TK_FN);
  
  hash_t name = lex.token_hash;
  match(TK_IDENTIFIER);
  
  match('(');
  param_t *params = param_type_list();
  match(')');
  
  type_t type = { 0 };
  if (lex.token == ':') {
    match(':');
    if (!type_name(&type))
      token_error("expected type-name");
  }
  
  func_t *func = make_func(name, &type, params, NULL, 0);
  
  current_func = func;
  
  map_put(scope_func, name, func);
  
  match('{');
  
  local_declarations(params);
  
  stmt_t *body, *head;
  body = head = statement();
  while (lex.token != '}')
    head = head->next = statement();
  
  match('}');
  
  func->body = body;
  func->local_size = local_offset;
  
  map_flush(scope_local);
  local_offset = 0;
  
  current_func = NULL;
  
  return func;
}

param_t *param_type_list()
{
  param_t *param = param_declaration();
  if (!param)
    return NULL;
  
  param_t *head = param;
  while (lex.token == ',') {
    match(',');
    head = head->next = param_declaration();
  }
  
  return param;
}

param_t *param_declaration()
{
  hash_t name;
  
  spec_t *spec = specifiers();
  if (!spec)
    return NULL;
  
  dcltr_t *dcltr = direct_declarator(&name);
  decl_t *decl = insert_decl(spec, dcltr, name);
  expr_t *addr = make_addr(make_const(decl->offset), ADDR_LOCAL, &decl->type);
  
  return make_param(spec, dcltr, addr);
}

void local_declarations(param_t *params)
{
  while (local_declaration());
}

int local_declaration()
{
  hash_t name;
  
  spec_t *spec = specifiers();
  
  if (!spec)
    return 0;
  
  while (1) {
    dcltr_t *dcltr = direct_declarator(&name);
    insert_decl(spec, dcltr, name);
    
    if (lex.token == ',')
      match(',');
    else
      break;
  }
  
  match(';');
  
  return 1;
}

decl_t *insert_decl(spec_t *spec, dcltr_t *dcltr, hash_t name)
{
  decl_t *decl = make_decl(spec, dcltr, local_offset);
  local_offset += type_size(spec, dcltr);
  map_put(scope_local, name, decl);
  return decl;
}

int type_name(type_t *type)
{
  type->spec = specifiers();
  if (!type->spec)
    return 0;
  
  type->dcltr = abstract_declarator();
  
  return 1;
}

dcltr_t *direct_declarator(hash_t *name)
{
  dcltr_t *ptr = pointer();
  
  if (lex.token == '(') {
    match('(');
    dcltr_t *dcltr = direct_declarator(name);
    match(')');
    
    if (!dcltr)
      token_error("expected declarator");
    
    dcltr_t *chain = dcltr;
    while (chain->next)
      chain = chain->next;
    chain->next = postfix_declarator(ptr);
    
    return dcltr;
  } else if (lex.token == TK_IDENTIFIER) {
    *name = lex.token_hash;
    match(TK_IDENTIFIER);
    return postfix_declarator(ptr);
  } else {
    if (ptr)
      token_error("expected declarator");
    
    return NULL;
  }
}

dcltr_t *abstract_declarator()
{
  dcltr_t *ptr = pointer();
  
  if (lex.token == '(') {
    match('(');
    dcltr_t *dcltr = abstract_declarator();
    match('(');
    
    if (!dcltr)
      token_error("expected abstract-declarator");
    
    dcltr_t *chain = dcltr;
    while (chain->next)
      chain = chain->next;
    chain->next = postfix_declarator(ptr);
    
    return dcltr;
  } else {
    return ptr;
  }
}

dcltr_t *postfix_declarator(dcltr_t *base)
{
  dcltr_t *dcltr = base;
  
  while (1) {
    if (lex.token == '[') {
      match('[');
      int size;
      if (!constant_expression(&size))
        token_error("array size is not constant");
      dcltr = make_dcltr_array(size, dcltr);
      match(']');
    } else {
      break;
    }
  }
  
  return dcltr;
}

dcltr_t *pointer()
{
  dcltr_t *dcltr = NULL;
  
  while (lex.token == '*') {
    match('*');
    dcltr = make_dcltr_pointer(dcltr);
  }
  
  return dcltr;
}

spec_t *specifiers()
{
  tspec_t tspec;
  switch (lex.token) {
  case TK_I32:
    tspec = TY_I32;
    break;
  default:
    return NULL;
  }
  
  next();
  
  return spec_cache_find(tspec);
}

spec_t *spec_cache_find(tspec_t tspec)
{
  for (int i = 0; i < num_spec; i++) {
    if (spec_cache[i].tspec == tspec)
      return &spec_cache[i];
  }
  
  spec_t *spec = &spec_cache[++num_spec];
  spec->tspec = tspec;
  
  return spec;
}
