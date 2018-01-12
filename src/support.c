/**
 * C-Minus symbol table generator
 *
 * Conner Short
 * December 2017
 * CMPE 152
 *
 * support.c: Miscellaneous support functions
**/
#include <stdio.h>
#include <stdlib.h>

#include "nt.h"
#include "symbol.h"

void die() {
	fprintf(stderr, "compilation terminated\n");
	exit(EXIT_FAILURE);
}

int set_scope_ebp_offsets(struct scope* s, int offset) {
	struct symbol* symbol_i = s->table_root;

	while(symbol_i != NULL) {
		offset -= symbol_i->size_bytes;
		symbol_i->ebp_offset = offset;
		symbol_i = symbol_i->next;
	}

	struct scope* scope_i = s->child_scopes_b;

	while(scope_i != NULL) {
		offset = set_scope_ebp_offsets(scope_i, offset);
		scope_i = scope_i->next;
	}

	return offset;
}

void set_ebp_offsets(struct declaration* decl_list) {
	/* Handle offsets for function parameters */
	struct declaration* d = decl_list;

	while(d != NULL) {
		if(d->type == FUNC_DECL) {
			int offset = 8;

			struct param* param_i = d->func->params;

			while(param_i != NULL) {
				param_i->symbol->ebp_offset = offset;

				offset += param_i->symbol->size_bytes;
				param_i = param_i->next;
			}

			d->func->local_bytes = -1 * set_scope_ebp_offsets(d->func->sc, -4) - 4;
		}

		d = d->next;
	}
}
