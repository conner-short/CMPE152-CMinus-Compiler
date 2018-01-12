/**
 * C-Minus symbol table generator
 *
 * Conner Short
 * December 2017
 * CMPE 152
 *
 * nt.h: Nonterminal structure types
**/
#ifndef _NT_H
#define _NT_H

#include "symbol.h"

enum factor_t {F_EXPR, F_VAR, F_CALL, F_NUM};
enum opr {
	OP_LT, OP_LTE, OP_GT, OP_GTE,
	OP_EQ, OP_NEQ, OP_ADD, OP_SUB,
	OP_MUL, OP_DIV, OP_NOP
};

enum stmt_t {
	RETURN_STMT, EXPR_STMT, COMPOUND_STMT,
	EMPTY_STMT, SEL_STMT, ITER_STMT
};

enum data_t {INT_TYPE, VOID_TYPE};
enum decl_t {VAR_DECL, FUNC_DECL};

struct expr;
struct simple_expr;
struct add_expr;
struct term;
struct factor;

struct term {
	enum opr op;
	struct term* l;
	struct factor* r;
};

struct add_expr {
	enum opr op;
	struct add_expr* l;
	struct term* r;
};

struct simple_expr {
	enum opr op;
	struct add_expr* l;
	struct add_expr* r;
};

struct expr {
	struct var_ref* dest_var;
	union {
		struct simple_expr* s_expr;
		struct expr* c_expr;
	};
};

struct arg_list {
	enum symbol_t type;
	struct expr* e;
	struct arg_list* prev;
};

struct call {
	struct symbol* id;
	struct arg_list* args;
};

struct var_ref {
	struct symbol* id;
	struct expr* offset_expr;
};

struct factor {
	enum factor_t type;

	union {
		struct expr* e;
		struct var_ref* v;
		struct call* c;
		int n;
	};
};

struct selection_stmt {
	struct expr* e;
	struct statement* t;
	struct statement* f;
};

struct iteration_stmt {
	struct expr* e;
	struct statement* body;
};

struct statement {
	enum stmt_t type;

	union {
		struct expr* e;
		struct statement* stmt_list;
		struct selection_stmt* sel;
		struct iteration_stmt* iter;
	};

	struct statement* next;
};

struct param {
	struct symbol* symbol;
	struct param* next;
};

struct function {
	struct symbol* id;
	struct scope* sc;
	struct param* params;
	int local_bytes;
	struct statement* body;
	enum data_t return_type;
};

struct declaration {
	enum decl_t type;
	union {
		struct symbol* var;
		struct function* func;
	};

	struct declaration* next;
};

#endif
