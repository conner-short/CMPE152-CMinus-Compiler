/**
 * C-Minus symbol table generator
 *
 * Conner Short
 * December 2017
 * CMPE 152
 *
 * symbol.h: Symbol type definitions and functions
**/
#ifndef _SYMBOL_H
#define _SYMBOL_H

enum symbol_t {S_INT, S_INT_ARRAY, S_FUNC};

struct symbol;
struct scope;

/**
 * Members:
 *   id: String identifier of symbol
 *   ref_count: Number of times symbol is referenced
 *   ebp_offset: Offset of symbol relative to EBP
 *   type: Type of symbol
 *   attr: Pointer to attribute structure for symbol
 *   next: Pointer to next symbol in table
**/
struct symbol {
	char* id;
	int ref_count;
	int ebp_offset;
	int size_bytes;
	int is_parameter;

	enum symbol_t type;
	void* attr;

	struct scope* parent_scope;
	struct symbol* next;
};

/**
 * Members:
 *   scope_id: Unique numeric ID of scope
 *   table_root: First symbol in list of symbols in scope
 *   next: Pointer to next scope in stack
 *   prev: Pointer to previous (parent) scope in stack
**/
struct scope {
	struct symbol* table_root;
	int scope_id;

	struct scope* parent_scope;

	struct scope* child_scopes_b;
	struct scope* child_scopes_t;

	struct scope* next;
};

extern struct scope* global_scope;
extern struct scope* current_scope;

extern int new_scope_flag;

/**
 * Allocates a new scope and pushes it to the scope stack
**/
void push_scope();

/**
 * Removes top-most scope from stack
 * 
 * Returns a pointer to the removed scope
**/
struct scope* pop_scope();

/**
 * Adds a symbol to the symbol table of a scope
 *
 * Parameters:
 *   s: Scope to add symbol to
 *   id: String identifier of symbol
 *   type: Type of symbol
 *
 * Returns a pointer to the new symbol on success, and NULL on failure
**/
struct symbol* add_to_symbol_table(char* id, enum symbol_t type);

struct symbol* find_symbol_in_scope(char* id);

#endif
