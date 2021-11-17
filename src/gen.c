#include "gen.h"

#include "hash.h"
#include "map.h"
#include "error.h"
#include <stdlib.h>

typedef struct label_s label_t;
typedef struct replace_s replace_t;

struct label_s {
  hash_t name;
  instr_t *pos;
  label_t *next;
};

struct replace_s {
  instr_t *pos;
  replace_t *next;
};

static instr_t *instr_buf = NULL, *instr_ptr = NULL;
static sym_t *sym_buf = NULL, *sym_ptr = NULL;
static int max_instr = 1024;
static int max_sym = 32;
static int num_lbl = 0;

static int func_active = 0;
static hash_t ret_lbl;

static map_t *map_replace;
static label_t *label_list;

void gen_jmp_hash(hash_t lbl);

void gen_func(func_t *func);
void gen_param(param_t *param);

void gen_stmt(stmt_t *stmt);
void gen_if(stmt_t *stmt);
void gen_while(stmt_t *stmt);
void gen_ret(stmt_t *stmt);

void gen_expr(expr_t *expr);
void gen_const(expr_t *expr);
void gen_addr(expr_t *expr);
void gen_call(expr_t *expr);
void gen_load(expr_t *expr);
void gen_binop(expr_t *expr);
void gen_condition(expr_t *expr, hash_t end);

label_t *make_label(hash_t name, instr_t *pos)
{
  label_t *label = malloc(sizeof(label_t));
  label->name = name;
  label->pos = pos;
  label->next = NULL;
  return label;
}

replace_t *make_replace(instr_t *pos)
{
  replace_t *replace = malloc(sizeof(replace_t));
  replace->pos = pos;
  replace->next = NULL;
  return replace;
}

hash_t tmp_label()
{
  char name[32];
  
  sprintf(name, ":_%lu", num_lbl++);
  
  return hash_value(name);
}

void set_label(hash_t name)
{
  if (label_list) {
    label_t *head = label_list;
    while (head->next) {
      if (name == head->name)
        error("duplicate label '%s'", hash_get(name));
      
      head = head->next;
    }
    
    if (name == head->name)
      error("duplicate label '%s'", hash_get(name));
    
    head->next = make_label(name, instr_ptr);
  } else {
    label_list = make_label(name, instr_ptr);
  }
}

void add_replace(hash_t name, instr_t *pos)
{
  replace_t *replace = map_get(map_replace, name);
  
  if (replace) {
    replace_t *head = replace;
    
    while (head->next)
      head = head->next;
    
    head->next = make_replace(pos);
  } else {
    map_put(map_replace, name, make_replace(pos));
  }
}

void replace_all()
{
  label_t *label = label_list;
  while (label) {
    replace_t *replace = map_get(map_replace, label->name);
    
    while (replace) {
      *replace->pos = label->pos - instr_buf;
      
      replace = replace->next;
    }
    
    label  = label->next;
  }
}

instr_t *add_instr(instr_t instr)
{
  if (instr_ptr >= &instr_buf[max_instr])
    error("add_instr", "ran out of memory");
  
  instr_t *cache = instr_ptr;
  
  *instr_ptr++ = instr;
  
  return cache;
}

sym_t *add_sym(hash_t name)
{
  if (sym_ptr >= &sym_buf[max_sym])
    error("add_sym", "ran out of memory");
  
  sym_t *cache = sym_ptr;
  
  sym_ptr->name = name;
  sym_ptr->pos = instr_ptr - instr_buf;
  sym_ptr++;
  
  return cache;
}

bin_t *make_bin(instr_t *instr, int num_instr, sym_t *sym, int num_sym)
{
  bin_t *bin = malloc(sizeof(bin_t));
  bin->instr = instr;
  bin->sym = sym;
  bin->num_instr = num_instr;
  bin->num_sym = num_sym;
  return bin;
}

bin_t *gen(unit_t *unit)
{
  instr_buf = malloc(max_instr);
  instr_ptr = instr_buf;
  
  sym_buf = malloc(max_sym);
  sym_ptr = sym_buf;
  num_lbl = 0;
  
  map_replace = make_map();
  
  gen_func(unit->func);
  
  replace_all();
  
  return make_bin(instr_buf, instr_ptr - instr_buf, sym_buf, sym_ptr - sym_buf);
}

void gen_instr_label(instr_t instr, hash_t lbl)
{
  add_instr(instr);
  instr_t *pos = add_instr(0);
  
  add_replace(lbl, pos);
}

void gen_func(func_t *func)
{
  func_active = 1;
  
  while (func) {
    ret_lbl = tmp_label();
    
    add_sym(func->name);
    set_label(func->name);
    
    add_instr(ENTER);
    add_instr(func->local_size);
    
    gen_param(func->params);
    
    gen_stmt(func->body);
    
    set_label(ret_lbl);
    
    add_instr(LEAVE);
    add_instr(RET);
    
    func = func->next;
  }
  
  func_active = 0;
}

void gen_param(param_t *param)
{
  if (!param)
    return;
  
  if (param->next)
    gen_param(param->next);
  
  gen_addr(param->addr);
  add_instr(STR);
}

void gen_stmt(stmt_t *stmt)
{
  while (stmt) {
    switch (stmt->tstmt) {
    case STMT_EXPR:
      gen_expr(stmt->expr);
      break;
    case STMT_IF:
      gen_if(stmt);
      break;
    case STMT_WHILE:
      gen_while(stmt);
      break;
    case STMT_RETURN:
      gen_ret(stmt);
      break;
    default:
      error("gen_stmt", "unknown case");
      break;
    }
    
    stmt = stmt->next;
  }
}

void gen_ret(stmt_t *stmt)
{
  if (!func_active)
    error("gen_ret", "ret_label while func inactive");
  
  gen_expr(stmt->ret_stmt.value);
  
  gen_instr_label(JMP, ret_lbl);
}

void gen_if(stmt_t *stmt)
{
  hash_t end_lbl = tmp_label();
  
  gen_condition(stmt->if_stmt.cond, end_lbl);
  gen_stmt(stmt->if_stmt.body);
  
  set_label(end_lbl);
}

void gen_while(stmt_t *stmt)
{
  hash_t end_lbl = tmp_label();
  hash_t cond_lbl = tmp_label();
  
  set_label(cond_lbl);
  gen_condition(stmt->while_stmt.cond, end_lbl);
  gen_stmt(stmt->while_stmt.body);
  gen_instr_label(JMP, cond_lbl);
  set_label(end_lbl);
}

void gen_expr(expr_t *expr)
{
  if (!expr)
    return;
  
  switch (expr->texpr) {
  case EXPR_CONST:
    gen_const(expr);
    break;
  case EXPR_ADDR:
    gen_addr(expr);
    break;
  case EXPR_LOAD:
    gen_load(expr);
    break;
  case EXPR_BINOP:
    gen_binop(expr);
    break;
  case EXPR_CALL:
    gen_call(expr);
    break;
  default:
    error("gen_expr", "unknown case");
    break;
  }
}

void gen_call(expr_t *expr)
{
  func_t *func = expr->post.base->func.func;
  
  expr_t *arg = expr->post.post;
  while (arg) {
    gen_expr(arg->arg.base);
    arg = arg->arg.next;
  }
  
  add_instr(CALL);
  instr_t *pos = add_instr(0);
  
  add_replace(func->name, pos);
}

void gen_const(expr_t *expr)
{
  add_instr(PUSH);
  add_instr(expr->num);
}

void gen_addr(expr_t *expr)
{
  switch (expr->addr.taddr) {
  case ADDR_GLOBAL:
    gen_expr(expr->addr.base);
    break;
  case ADDR_LOCAL:
    add_instr(LBP);
    gen_expr(expr->addr.base);
    add_instr(ADD);
    break;
  default:
    error("gen_addr", "unknown case");
    break;
  }
  
}

void gen_load(expr_t *expr)
{
  gen_addr(expr);
  add_instr(LDR);
}

void gen_condition(expr_t *expr, hash_t end)
{
  hash_t next_cond, yes_cond;
  instr_t *pos;
  
  switch (expr->texpr) {
  case EXPR_BINOP:
    switch (expr->binop.op) {
    case OPERATOR_AND:
      gen_condition(expr->binop.lhs, end);
      gen_condition(expr->binop.rhs, end);
      break;
    case OPERATOR_OR:
      next_cond = tmp_label();
      yes_cond = tmp_label();
      
      gen_condition(expr->binop.lhs, next_cond);
      
      gen_instr_label(JMP, yes_cond);
      
      set_label(next_cond);
      gen_condition(expr->binop.rhs, end);
      
      set_label(yes_cond);
      
      break;
    case OPERATOR_EQ:
    case OPERATOR_NE:
    case OPERATOR_LE:
    case OPERATOR_GE:
    case OPERATOR_LSS:
    case OPERATOR_GTR:
      gen_expr(expr->binop.lhs);
      gen_expr(expr->binop.rhs);
      add_instr(CMP);
      
      switch (expr->binop.op) {
      case OPERATOR_EQ:
        gen_instr_label(JNE, end);
        break;
      case OPERATOR_NE:
        gen_instr_label(JE, end);
        break;
      case OPERATOR_LE:
        gen_instr_label(JG, end);
        break;
      case OPERATOR_GE:
        gen_instr_label(JL, end);
        break;
      case OPERATOR_LSS:
        gen_instr_label(JGE, end);
        break;
      case OPERATOR_GTR:
        gen_instr_label(JLE, end);
        break;
      }
      
      break;
    default:
      goto expr_cond;
    }
    break;
  expr_cond:
  default:
    gen_expr(expr);
    add_instr(PUSH);
    add_instr(0);
    add_instr(CMP);
    gen_instr_label(JE, end);
    break;
  }
}

void gen_binop(expr_t *expr)
{
  if (expr->binop.op == OPERATOR_ASSIGN) {
    gen_expr(expr->binop.rhs);
    gen_addr(expr->binop.lhs);
    
    add_instr(STR);
  } else if (expr->binop.op == OPERATOR_OR || expr->binop.op == OPERATOR_AND) {
    hash_t cond_end, body_end;
    
    cond_end = tmp_label();
    body_end = tmp_label();
    
    gen_condition(expr, cond_end);
    
    add_instr(PUSH);
    add_instr(1);
    add_instr(JMP);
    instr_t *pos = add_instr(0);
    
    add_replace(body_end, pos);
    
    set_label(cond_end);
    
    add_instr(PUSH);
    add_instr(0);
    
    set_label(body_end);
  } else {
    gen_expr(expr->binop.lhs);
    gen_expr(expr->binop.rhs);
    
    switch (expr->type.spec->tspec) {
    case TY_I32:
      switch (expr->binop.op) {
      case OPERATOR_ADD:
        add_instr(ADD);
        break;
      case OPERATOR_SUB:
        add_instr(SUB);
        break;
      case OPERATOR_MUL:
        add_instr(MUL);
        break;
      case OPERATOR_DIV:
        add_instr(DIV);
        break;
      case OPERATOR_EQ:
        add_instr(CMP);
        add_instr(SETE);
        break;
      case OPERATOR_NE:
        add_instr(CMP);
        add_instr(SETNE);
        break;
      case OPERATOR_LSS:
        add_instr(CMP);
        add_instr(SETL);
        break;
      case OPERATOR_GTR:
        add_instr(CMP);
        add_instr(SETG);
        break;
      case OPERATOR_LE:
        add_instr(CMP);
        add_instr(SETLE);
        break;
      case OPERATOR_GE:
        add_instr(CMP);
        add_instr(SETGE);
        break;
      default:
        error("gen_binop", "unknown case '%i'", expr->binop.op);
        break;
      }
    }
  }
}
