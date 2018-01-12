/**
 * C-Minus symbol table generator
 *
 * Conner Short
 * December 2017
 * CMPE 152
 *
 * scope.c: Scope / scope stack definitions and functions
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "support.h"
#include "symbol.h"

struct scope* global_scope = NULL;
struct scope* current_scope = NULL;

int new_scope_flag = 1;

void push_scope() {
	static int scope_count = 0;

	struct scope* tmp = malloc(sizeof(struct scope));
	if(tmp == NULL) {
		perror("push_scope");
		die();
	}

	tmp->scope_id = scope_count;
	tmp->table_root = NULL;
	tmp->next = NULL;

	tmp->child_scopes_b = NULL;
	tmp->child_scopes_t = NULL;

	if(current_scope == NULL) {
		/* Global scope */
		tmp->parent_scope = NULL;

		global_scope = tmp;
		current_scope = tmp;
	} else {
		tmp->parent_scope = current_scope;

		if(current_scope->child_scopes_t == NULL) {
			/* First child */
			current_scope->child_scopes_b = tmp;
			current_scope->child_scopes_t = tmp;
		} else {
			/* Other child */
			current_scope->child_scopes_t->next = tmp;
			current_scope->child_scopes_t = tmp;
		}
	}

	current_scope = tmp;
	scope_count++;
}

struct scope* pop_scope() {
	struct scope* tmp = current_scope;

	if(current_scope != NULL) {
		current_scope = current_scope->parent_scope;
	}

	return tmp;
}

struct symbol* find_symbol_in_scope(char* id) {
	struct scope* scope_tmp = current_scope;
	struct symbol* symbol_tmp;

	while(scope_tmp != NULL) {
		symbol_tmp = scope_tmp->table_root;

		while(symbol_tmp != NULL) {
			if(strcmp(symbol_tmp->id, id) == 0) {return symbol_tmp;}
			symbol_tmp = symbol_tmp->next;
		}

		/* Look in parent scope if not found in current scope */
		scope_tmp = scope_tmp->parent_scope;
	}

	return NULL;
}

struct symbol* add_to_symbol_table(char* id, enum symbol_t type) {
	/* Check if symbol is already declared in scope */

	struct symbol* match = find_symbol_in_scope(id);

	if(match != NULL) {
		fprintf(stderr,
			"error: symbol \"%s\" in scope %d already declared in scope %d\n",
			id,
			current_scope->scope_id,
			match->parent_scope->scope_id);

			return NULL;
	}

	struct symbol* tmp = malloc(sizeof(struct symbol));
	if(tmp == NULL) {
		perror("add_to_symbol_table");
		die();
	}

	/* Set symbol attributes */

	tmp->id = strdup(id);
	if(tmp->id == NULL) {
		perror("add_to_symbol_table");
		die();
	}

	tmp->type = type;
	tmp->ebp_offset = 0;
	tmp->parent_scope = current_scope;
	tmp->is_parameter = 0;

	/* Add symbol to table */

	if(current_scope->table_root == NULL) {
		current_scope->table_root = tmp;
	} else {
		struct symbol* i = current_scope->table_root;
		while(i->next != NULL) {i = i->next;}

		i->next = tmp;
	}

	tmp->next = NULL;

	return tmp;
}
