#include "p_local.h"

#include <stdlib.h>

#define MAX_OP 8

typedef enum order_e order_t;
typedef struct opset_s opset_t;

enum order_e {
  ORDER_ASSIGNMENT,
  ORDER_OR,
  ORDER_AND,
  ORDER_EQUALATIVE,
  ORDER_ADDITIVE,
  ORDER_MULTIPLICATIVE
};

struct opset_s {
  token_t op[MAX_OP];
};

opset_t opset_dict[] = {
  { '=', TK_ADD_ASSIGN, TK_SUB_ASSIGN, TK_MUL_ASSIGN, TK_DIV_ASSIGN },
  { TK_OR_OP },
  { TK_AND_OP },
  { TK_EQ_OP, TK_NE_OP },
  { '<', '>', TK_LE_OP, TK_GE_OP },
  { '+', '-' },
  { '*', '/' }
};

int num_opset_dict = sizeof(opset_dict) / sizeof(opset_t);

expr_t *make_expr()
{
  return malloc(sizeof(expr_t));
}

expr_t *make_const(int num)
{
  expr_t *expr = make_expr();
  expr->texpr = EXPR_CONST;
  expr->num = num;
  expr->type.spec = ty_i32;
  expr->type.dcltr = NULL;
  return expr;
}

expr_t *make_addr(expr_t *base, taddr_t taddr, type_t *type)
{
  expr_t *expr = make_expr();
  expr->texpr = EXPR_ADDR;
  expr->addr.base = base;
  expr->addr.taddr = taddr;
  expr->type.spec = type->spec;
  expr->type.dcltr = type->dcltr;
  return expr;
}

expr_t *make_load(expr_t *base, taddr_t taddr, type_t *type)
{
  expr_t *expr = make_expr();
  expr->texpr = EXPR_LOAD;
  expr->addr.base = base;
  expr->addr.taddr = taddr;
  expr->type.spec = type->spec;
  expr->type.dcltr = type->dcltr;
  return expr;
}

expr_t *make_func_expr(func_t *func)
{
  expr_t *expr = make_expr();
  expr->texpr = EXPR_FUNC;
  expr->func.func = func;
  expr->type.spec = spec_cache_find(TY_FUNC);
  expr->type.dcltr = NULL;
  return expr;
}

expr_t *make_call(expr_t *func, expr_t *arg)
{
  expr_t *expr = make_expr();
  expr->texpr = EXPR_CALL;
  expr->post.base = func;
  expr->post.post = arg;
  expr->type.spec = func->func.func->type.spec;
  expr->type.dcltr = func->func.func->type.dcltr;
  return expr;
}

expr_t *make_arg(expr_t *base)
{
  expr_t *expr = make_expr();
  expr->texpr = EXPR_ARG;
  expr->arg.base = base;
  expr->arg.next = NULL;
  return expr;
}

expr_t *make_binop(expr_t *lhs, operator_t op, expr_t *rhs)
{
  if (lhs->texpr == EXPR_CONST && rhs->texpr == EXPR_CONST) {
    switch (op) {
    case OPERATOR_ADD:
      return make_const(lhs->num + rhs->num);
    case OPERATOR_SUB:
      return make_const(lhs->num - rhs->num);
    case OPERATOR_MUL:
      return make_const(lhs->num * rhs->num);
    case OPERATOR_DIV:
      return make_const(lhs->num / rhs->num);
    default:
      token_error("unknown operator");
      break;
    }
  }
  
  expr_t *expr = make_expr();
  expr->texpr = EXPR_BINOP;
  expr->binop.op = op;
  expr->binop.lhs = lhs;
  expr->binop.rhs = rhs;
  expr->type.spec = lhs->type.spec;
  expr->type.dcltr = lhs->type.dcltr;
  return expr;
}

int read_assign_op(operator_t *op)
{
  switch (lex.token) {
  case '=':
    *op = OPERATOR_ASSIGN;
    break;
  case TK_ADD_ASSIGN:
    *op = OPERATOR_ADD;
    break;
  case TK_SUB_ASSIGN:
    *op = OPERATOR_SUB;
    break;
  case TK_MUL_ASSIGN:
    *op = OPERATOR_MUL;
    break;
  case TK_DIV_ASSIGN:
    *op = OPERATOR_DIV;
    break;
  default:
    return 0;
  }
  
  next();
  return 1;
}

int read_expr_op(operator_t *op, int level)
{
  for (int i = 0; i < MAX_OP; i++) {
    if (opset_dict[level].op[i] == lex.token) {
      switch (lex.token) {
      case '+':
        *op = OPERATOR_ADD;
        break;
      case '-':
        *op = OPERATOR_SUB;
        break;
      case '*':
        *op = OPERATOR_MUL;
        break;
      case '/':
        *op = OPERATOR_DIV;
        break;
      case TK_OR_OP:
        *op = OPERATOR_OR;
        break;
      case TK_AND_OP:
        *op = OPERATOR_AND;
        break;
      case TK_EQ_OP:
        *op = OPERATOR_EQ;
        break;
      case TK_NE_OP:
        *op = OPERATOR_NE;
        break;
      case '>':
        *op = OPERATOR_GTR;
        break;
      case '<':
        *op = OPERATOR_LSS;
        break;
      case TK_LE_OP:
        *op = OPERATOR_LE;
        break;
      case TK_GE_OP:
        *op = OPERATOR_GE;
        break;
      default:
        return 0;
      }
      
      next();
      return 1;
    }
  }
  
  return 0;
}

int is_func(expr_t *expr)
{
  return expr->texpr == EXPR_FUNC;
}

int is_lvalue(expr_t *expr)
{
  return expr->texpr == EXPR_LOAD;
}

int is_array(expr_t *expr)
{
  return expr->type.dcltr && expr->type.dcltr->type == DCLTR_ARRAY;
}

int is_pointer(expr_t *expr)
{
  return expr->type.dcltr && expr->type.dcltr->type == DCLTR_POINTER;
}

expr_t *find_identifier()
{
  hash_t name = lex.token_hash;
  
  decl_t *decl;
  func_t *func;
  if ((decl = map_get(scope_local, name))) {
    match(TK_IDENTIFIER);
    return make_load(make_const(decl->offset), ADDR_LOCAL, &decl->type);
  } else if ((func = map_get(scope_func, name))) {
    match(TK_IDENTIFIER);
    return make_func_expr(func);
  } else {
    token_error("'%n' undeclared");
  }
}

expr_t *primary()
{
  expr_t *expr = NULL;
  switch (lex.token) {
  case TK_CONSTANT:
    expr = make_const(lex.token_num);
    match(TK_CONSTANT);
    break;
  case TK_IDENTIFIER:
    expr = find_identifier();
    break;
  case '(':
    match('(');
    expr = expression();
    match(')');
    break;
  default:
    return NULL;
  }
  
  return expr;
}

expr_t *postfix()
{
  expr_t *expr = primary();
  
  if (!expr)
    return NULL;
  
  while (1) {
    if (lex.token == '[') {
      match('[');
      expr_t *post = expression();
      match(']');
      
      if (!is_array(expr) && !is_pointer(expr))
        token_error("cannot index non-array");
      
      type_t array_type = { expr->type.spec, expr->type.dcltr->next };
      
      expr_t *align = make_const(type_size(array_type.spec, array_type.dcltr));
      expr_t *offset = make_binop(post, OPERATOR_MUL, align);
      
      if (is_array(expr)) {
        expr_t *base = make_binop(expr->addr.base, OPERATOR_ADD, offset);
        expr = make_load(base, expr->addr.taddr, &array_type);
      } else if (is_pointer(expr)) {
        expr_t *base = make_binop(expr, OPERATOR_ADD, offset);
        expr = make_load(base, ADDR_GLOBAL, &array_type);
      }
    } else if (lex.token == '(') {
      match('(');
      expr_t *post = arg_expr_list();
      match(')');
      
      if (!is_func(expr))
        token_error("cannot call non-function");
      
      return make_call(expr, post);
    } else {
      break;
    }
  }
  
  return expr;
}

expr_t *arg_expr_list()
{
  expr_t *args, *head, *base;
  
  base = expression(ORDER_ASSIGNMENT);
  if (!base)
    return NULL;
  
  head = args = make_arg(base);
  
  while (lex.token == ',') {
    match(',');
    
    base = expression(ORDER_ASSIGNMENT);
    if (!head)
      token_error("expected expression");
    
    head = head->arg.next = make_arg(base);
  }
  
  return args;
}

expr_t *unary()
{
  if (lex.token == '&') {
    match('&');
    
    expr_t *expr = postfix();
    
    if (!is_lvalue(expr))
      token_error("unary operator '&' requires lvalue");
    
    expr->texpr = EXPR_ADDR;
    expr->type.dcltr = make_dcltr_pointer(expr->type.dcltr);
    
    return expr;
  } else if(lex.token == '*') {
    match('*');
    
    expr_t *expr = postfix();
    
    if (!is_pointer(expr))
      token_error("cannot cast indirection on non-pointer");
    
    type_t indirect_type = { expr->type.spec, expr->type.dcltr->next };
    
    return make_load(expr, ADDR_GLOBAL, &indirect_type);
  } if (lex.token == '-') {
    match('-');
    return make_binop(postfix(), OPERATOR_MUL, make_const(-1));
  } else if (lex.token == '+') {
    match('+');
    return postfix();
  } else {
    return postfix();
  }
}

expr_t *binop(int level)
{
  if (level >= num_opset_dict)
    return unary();
  
  expr_t *lhs = binop(level + 1);
  
  if (!lhs)
    return NULL;
  
  operator_t op;
  if (level == ORDER_ASSIGNMENT) {
   if (read_assign_op(&op)) {
      expr_t *rhs = binop(level + 1);
      
      if (!rhs)
        token_error("expected expression");
      
      if (!is_lvalue(lhs))
        token_error("cannot assign non-lvalue");
      
      if (!is_type_match(&lhs->type, &rhs->type))
        token_error("type mismatch");
      
      if (op == OPERATOR_ASSIGN)
        return make_binop(lhs, op, rhs);
      else
        return make_binop(lhs, OPERATOR_ASSIGN, make_binop(lhs, op, rhs));
    }
  } else {
    if (read_expr_op(&op, level))
      return make_binop(lhs, op, binop(level + 1));
  }
  
  return lhs;
}

expr_t *expression()
{
  return binop(ORDER_ASSIGNMENT);
}

int constant_expression(int *num)
{
  expr_t *expr = binop(0);
  
  if (expr->texpr != EXPR_CONST)
    return 0;
  
  *num = expr->num;
  
  return 1;
}
