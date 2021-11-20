#ifndef P_LOCAL_H
#define P_LOCAL_H

#include "../common/error.h"
#include "parse.h"

extern spec_t *ty_u0;
extern spec_t *ty_i32;

extern map_t *scope_func;
extern map_t *scope_local;
extern map_t *scope_struct;

extern func_t *current_func;

expr_t *make_const(int i32);
expr_t *make_addr(expr_t *base, taddr_t taddr, type_t *type);
dcltr_t *make_dcltr_pointer(dcltr_t *next);

int is_type_match(type_t *lhs, type_t *rhs);
int type_size(spec_t *spec, dcltr_t *dcltr);
spec_t *spec_cache_find(tspec_t tspec, struct_scope_t *struct_scope);
struct_decl_t *find_struct_decl(struct_scope_t *struct_scope, hash_t name);

int constant_expression(int *num);
expr_t *primary();
expr_t *binop(int level);
expr_t *arg_expr_list();
expr_t *expression();

stmt_t *statement();
stmt_t *compound_statement();
stmt_t *if_statement();
stmt_t *while_statement();
stmt_t *expression_statement();
stmt_t *return_statement();

func_t *func_declaration();
void struct_declarations();
int struct_declaration();
int struct_member_declaration();
void local_declarations();
int local_declaration();
dcltr_t *direct_declarator(hash_t *name);
dcltr_t *abstract_declarator();
dcltr_t *postfix_declarator(dcltr_t *base);
dcltr_t *pointer();
spec_t *specifiers();
int type_name(type_t *type);
param_t *param_type_list();
param_t *param_declaration();
decl_t *insert_decl(spec_t *spec, dcltr_t *dcltr, hash_t name);

#endif
