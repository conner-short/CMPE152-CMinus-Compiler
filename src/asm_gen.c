/**
 * C-Minus symbol table generator
 *
 * Conner Short
 * December 2017
 * CMPE 152
 *
 * asm_gen.c: Assembly code generation routines
**/
#include <stdio.h>

#include "nt.h"
#include "symbol.h"

void generate_expr_asm(struct expr* e);
void generate_compound_stmt_asm(char* f_name, struct statement* s);

int generate_label_num() {
	static int label_num = 0;
	return label_num++;
}

void generate_return_stmt_asm(char* func_name, struct expr* e) {
	generate_expr_asm(e);

	printf("\tmovl %%ecx, %%eax\n\tjmp .%sret\n", func_name);
}

void generate_var_read_asm(struct var_ref* r) {
	if(r->offset_expr != NULL) {
		generate_expr_asm(r->offset_expr);
	} else {
		printf("\tmovl $0, %%ecx\n");
	}

	if(r->id->type == S_INT_ARRAY && r->id->is_parameter == 1) {
		/* Pointer */
		printf("\tmovl %d(%%ebp), %%ebx\n", r->id->ebp_offset);
		printf("\tmovl (%%ebx, %%ecx, 4), %%ecx\n");
	} else if(r->id->parent_scope == global_scope && r->id->is_parameter == 0) {
		/* Value in global */
		printf("\tmovl %s@GOT(%%eax), %%ebx\n", r->id->id);
		printf("\tmovl (%%ebx, %%ecx, 4), %%ecx\n");
	} else {
		/* Value on stack */
		printf("\taddl $%d, %%ecx\n", r->id->ebp_offset);
		printf("\tmovl (%%ebp, %%ecx), %%ecx\n");
	}
}

void generate_var_write_asm(struct var_ref* r) {
	printf("\tpushl %%ecx\n");

	if(r->offset_expr != NULL) {
		generate_expr_asm(r->offset_expr);
	} else {
		printf("\tmovl $0, %%ecx\n");
	}

	printf("\tpopl %%edx\n");

	if(r->id->type == S_INT_ARRAY && r->id->is_parameter == 1) {
		/* Pointer */
		printf("\tmovl %d(%%ebp), %%ebx\n", r->id->ebp_offset);
		printf("\tmovl %%edx, (%%ebx, %%ecx, 4)\n");
	} else if(r->id->parent_scope == global_scope && r->id->is_parameter == 0) {
		/* Value in global */
		printf("\tmovl %s@GOT(%%eax), %%ebx\n", r->id->id);
		printf("\tmovl %%edx, (%%ebx, %%ecx, 4)\n");
	} else {
		/* Value on stack */
		printf("\taddl $%d, %%ecx\n", r->id->ebp_offset);
		printf("\tmovl %%edx, (%%ebp, %%ecx)\n");
	}
}

void generate_call_asm(struct call* c) {
	printf("\tpushl %%eax\n");

	struct arg_list* i = c->args;
	int num_args = 0;

	while(i != NULL) {
		if(i->type == S_INT_ARRAY) {
			/* Pointer argument */
			struct symbol* arg_symbol = i->e->s_expr->r->r->r->v->id;
			if(arg_symbol->is_parameter == 0) {
				/* Global */
				printf("\tpushl %s@GOT(%%eax)\n", arg_symbol->id);
			} else {
				/* Parameter */
				printf("\tmovl %d(%%ebp), %%ebx\n", arg_symbol->ebp_offset);
				printf("\tpushl %%ebx\n");
			}
		} else {
			/* Integer argument */
			generate_expr_asm(i->e);
			printf("\tpushl %%ecx\n");
		}

		num_args++;
		i = i->prev;
	}

	printf("\tcall %s\n\taddl $%d, %%esp\n",
		c->id->id,
		num_args * 4);

	printf("\tmovl %%eax, %%ecx\n\tpopl %%eax\n");
}

void generate_factor_asm(struct factor* f) {
	switch(f->type) {
	case F_EXPR:
		generate_expr_asm(f->e);
		break;

	case F_VAR:
		generate_var_read_asm(f->v);
		break;

	case F_CALL:
		generate_call_asm(f->c);
		break;

	case F_NUM:
		printf("\tmovl $%d, %%ecx\n", f->n);
		break;
	}
}

void generate_term_asm(struct term* e) {
	switch(e->op) {
	case OP_NOP:
		generate_factor_asm(e->r);
		break;

	case OP_MUL:
		generate_term_asm(e->l);
		printf("\tmovl %%ecx, %%edx\n");
		generate_factor_asm(e->r);
		printf("\tpushl %%eax\n\tmovl %%edx, %%eax\n\timull %%ecx\n");
		printf("\tmovl %%eax, %%ecx\n\tpopl %%eax\n");
		break;

	case OP_DIV:
		generate_term_asm(e->l);
		printf("\tmovl %%ecx, %%edx\n");
		generate_factor_asm(e->r);
		printf("\tpushl %%eax\n\tmovl %%edx, %%eax\n\tcdq\n\tidivl %%ecx\n");
		printf("\tmovl %%eax, %%ecx\n\tpopl %%eax\n");
		break;

	default: break;
	}
}

void generate_add_expr_asm(struct add_expr* e) {
	switch(e->op) {
	case OP_NOP:
		generate_term_asm(e->r);
		break;

	case OP_ADD:
		generate_add_expr_asm(e->l);
		printf("\tmovl %%ecx, %%edx\n");
		generate_term_asm(e->r);
		printf("\taddl %%edx, %%ecx\n");
		break;

	case OP_SUB:
		generate_add_expr_asm(e->l);
		printf("\tmovl %%ecx, %%edx\n");
		generate_term_asm(e->r);
		printf("\tsubl %%ecx, %%edx\n\tmovl %%edx, %%ecx\n");
		break;

	default: break;
	}
}

void generate_simple_expr_asm(struct simple_expr* e) {
	if(e->op == OP_NOP) {
		generate_add_expr_asm(e->r);
	} else {
		/* Put left side in EDX and right side in ECX */
		generate_add_expr_asm(e->l);
		printf("\tmovl %%ecx, %%edx\n");
		generate_add_expr_asm(e->r);
		printf("\tcmpl %%ecx, %%edx\n");

		int label = generate_label_num();

		switch(e->op) {
		case OP_LT:  printf("\tjl .l%dt\n",  label); break;
		case OP_LTE: printf("\tjle .l%dt\n", label); break;
		case OP_GT:  printf("\tjg .l%dt\n",  label); break;
		case OP_GTE: printf("\tjge .l%dt\n", label); break;
		case OP_EQ:  printf("\tje .l%dt\n",  label); break;
		case OP_NEQ: printf("\tjne .l%dt\n", label); break;
		default: break;
		}

		printf("\tmovl $0, %%ecx\n\tjmp .l%dp\n", label);
		printf(".l%dt:\n\tmovl $1, %%ecx\n.l%dp:\n", label, label);
	}
}

void generate_expr_asm(struct expr* e) {
	if(e != NULL) {
		if(e->dest_var != NULL) {
			generate_expr_asm(e->c_expr);
			generate_var_write_asm(e->dest_var);
		} else {
			generate_simple_expr_asm(e->s_expr);
		}
	}
}

void generate_selection_stmt_asm(char* f_name, struct selection_stmt* sel) {
	int label = generate_label_num();

	generate_expr_asm(sel->e);
	printf("\ttestl %%ecx, %%ecx\n\tjz .l%df\n", label);
	generate_compound_stmt_asm(f_name, sel->t);
	printf("\tjmp .l%dp\n.l%df:\n", label, label);
	generate_compound_stmt_asm(f_name, sel->f);
	printf(".l%dp:\n", label);
}

void generate_iteration_stmt_asm(char* f_name, struct iteration_stmt* iter) {
	int label = generate_label_num();

	printf(".l%dh:\n", label);
	generate_expr_asm(iter->e);
	printf("\ttestl %%ecx, %%ecx\n\tjz .l%dp\n", label);
	generate_compound_stmt_asm(f_name, iter->body);
	printf("\tjmp .l%dh\n.l%dp:\n", label, label);
}

void generate_compound_stmt_asm(char* f_name, struct statement* s) {
	while(s != NULL) {
		switch(s->type) {
		case RETURN_STMT:
			generate_return_stmt_asm(f_name, s->e);
			break;

		case EXPR_STMT:
			generate_expr_asm(s->e);
			break;

		case COMPOUND_STMT:
			generate_compound_stmt_asm(f_name, s->stmt_list);
			break;

		case SEL_STMT:
			generate_selection_stmt_asm(f_name, s->sel);
			break;

		case ITER_STMT:
			generate_iteration_stmt_asm(f_name, s->iter);
			break;

		case EMPTY_STMT:
		default:
			break;
		}

		s = s->next;
	}
}

void generate_asm(struct declaration* decl_list) {
	struct declaration* d = decl_list;

	while(d != NULL) {
		if(d->type == VAR_DECL) {
			printf("\t.comm %s,%d\n", d->var->id, d->var->size_bytes);
		} else {
			char* f_name = d->func->id->id;

			printf("\t.globl %s\n", f_name);
			printf("\t.type %s, @function\n", f_name);
			printf("%s:\n", f_name);
			printf("\tpushl %%ebp\n\tmovl %%esp, %%ebp\n\tpushl %%ebx\n");
			printf("\tsubl $%d, %%esp\n", d->func->local_bytes);
			printf("\tcall __x86.get_pc_thunk.ax\n");
			printf("\taddl $_GLOBAL_OFFSET_TABLE_, %%eax\n");

			generate_compound_stmt_asm(f_name, d->func->body);

			printf(".%sret:\n\tmovl -4(%%ebp), %%ebx\n\tleave\n\tret\n", f_name);
		}

		d = d->next;
	}
}
