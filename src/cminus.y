/**
 * C-Minus symbol table generator
 *
 * Conner Short
 * December 2017
 * CMPE 152
 *
 * cminus.y: YACC parser and symbol table generator
**/
%{
#include <stdio.h>
#include <stdlib.h>

#include "asm_gen.h"
#include "nt.h"
#include "support.h"
#include "symbol.h"

int yylex();
int yyerror(char *);

struct declaration* program_decl_list = NULL;
struct scope* popped_scope;
struct function* f_tmp;

%}

%token ELSE
%token IF
%token INT
%token RETURN
%token VOID
%token WHILE

%token ID
%token NUM

%token LTE
%token GTE
%token EQUAL
%token NOTEQUAL

%union {
	char* string;
	int integer;
	enum data_t data_type;
	enum opr op_type;
	struct statement* stmt;
	struct expr* c_expr;
	struct simple_expr* s_expr;
	struct add_expr* add_expr;
	struct term* term_expr;
	struct factor* factor_expr;
	struct symbol* sy;
	struct param* p;
	struct function* f;
	struct declaration* decl;
	struct selection_stmt* sel;
	struct iteration_stmt* iter;
	struct arg_list* arg;
	struct call* c;
	struct var_ref* v;
}

%token <string>
%token <integer>
%token <data_type>
%token <op_type>
%token <stmt>
%token <c_expr>
%token <s_expr>
%token <add_expr>
%token <term>
%token <factor>
%token <sy>
%token <p>
%token <f>
%token <decl>
%token <sel>
%token <iter>
%token <arg>
%token <c>
%token <v>

%type <string> ID
%type <integer> NUM
%type <data_type> type_specifier
%type <op_type> addop mulop relop
%type <stmt> statement statement_list compound_stmt
%type <c_expr> expression expression_stmt return_stmt
%type <s_expr> simple_expression
%type <add_expr> additive_expression
%type <term_expr> term
%type <factor_expr> factor
%type <sy> var_declaration
%type <p> param param_list params
%type <f> fun_declaration
%type <decl> declaration declaration_list
%type <sel> selection_stmt
%type <iter> iteration_stmt
%type <arg> args arg_list
%type <c> call
%type <v> var

%%

program : declaration_list {program_decl_list = $1;};

declaration_list : declaration_list declaration {
	struct declaration* i = $1;

	while(i->next != NULL) {i = i->next;}

	i->next = $2;
	$$ = $1;
};

declaration_list : declaration {$$ = $1;};

declaration : var_declaration {
	$$ = malloc(sizeof(struct declaration));
	if($$ == NULL) {
		perror("declaration : var_declaration");
		die();
	}

	$$->type = VAR_DECL;
	$$->var = $1;
	$$->next = NULL;
};

declaration : fun_declaration {
	$$ = malloc(sizeof(struct declaration));
	if($$ == NULL) {
		perror("declaration : fun_declaration");
		die();
	}

	$$->type = FUNC_DECL;
	$$->func = $1;
	$$->next = NULL;
};

var_declaration : type_specifier ID ';' {
	/* Add variable to symbol table */
	$$ = add_to_symbol_table($2, S_INT);

	if($$ == NULL) {die();}
	else {
		$$->size_bytes = 4;
	}
};

var_declaration : type_specifier ID '[' NUM ']' ';' {
	$$ = add_to_symbol_table($2, S_INT_ARRAY);
	if($$ == NULL) {
		perror("var_declaration : type_specifier ID '[' NUM ']' ';'");
		die();
	}

	$$->size_bytes = $4 * 4;
};

type_specifier : INT {$$ = INT_TYPE;};
type_specifier : VOID {$$ = VOID_TYPE;};

fun_declaration: type_specifier ID '(' params ')' {
	/* Add function to symbol table */
	struct symbol* s = add_to_symbol_table($2, S_FUNC);

	if(s == NULL) {die();} 
	else {
		f_tmp = malloc(sizeof(struct function));
		if(f_tmp == NULL) {
			perror("fun_declaration: type_specifier ID '(' params ')' compound_stmt");
			die();
		}

		s->attr = f_tmp;
		f_tmp->id = s;

		f_tmp->params = $4;
		f_tmp->return_type = $1;
	}
} compound_stmt {
	$$ = f_tmp;
	$$->body = $<stmt>7;
	$$->sc = popped_scope;
};

params : param_list {$$ = $1;};
params : VOID {$$ = NULL;};

param_list : param_list ',' param {
	struct param* i = $1;

	while(i->next != NULL) {i = i->next;}

	i->next = $3;
	$$ = $1;
};

param_list : param {$$ = $1;};

param : type_specifier ID {
	/* Add parameter to symbol table */
	struct symbol* s = add_to_symbol_table($2, S_INT);

	if(s == NULL) {die();}
	else {
		s->size_bytes = 4;
		s->is_parameter = 1;

		$$ = malloc(sizeof(struct param));
		if($$ == NULL) {die();}
		else {
			$$->symbol = s;
			$$->next = NULL;
		}
	}
};

param : type_specifier ID '[' ']' {
	/* Array pointer parameter */
	struct symbol* s = add_to_symbol_table($2, S_INT_ARRAY);
	if(s == NULL) {
		perror("param : type_specifier ID '[' ']'");
		die();
	}

	s->size_bytes = 4;
	s->is_parameter = 1;

	$$ = malloc(sizeof(struct param));
	if($$ == NULL) {
		perror("param : type_specifier ID '[' ']'");
		die();
	}

	$$->symbol = s;
	$$->next = NULL;
};

compound_stmt : '{' local_declarations statement_list '}' {
	popped_scope = pop_scope();
	$$ = $3;
};

local_declarations : local_declarations var_declaration {
	if(new_scope_flag) {
		push_scope();
		new_scope_flag = 0;
	}
};

local_declarations : %empty {
	if(new_scope_flag) {
		push_scope();
		new_scope_flag = 0;
	}
};

statement_list : statement_list statement {
	new_scope_flag = 1;

	struct statement* i = $1;

	while(i->next != NULL) {i = i->next;}

	i->next = $2;
	$$ = $1;
};

statement_list : %empty {
	new_scope_flag = 1;

	$$ = malloc(sizeof(struct statement));
	if($$ == NULL) {
		perror("statement_list : %empty");
		die();
	}

	$$->type = EMPTY_STMT;
	$$->stmt_list = NULL;
	$$->next = NULL;
};

statement : expression_stmt {
	$$ = malloc(sizeof(struct statement));
	if($$ == NULL) {
		perror("statement : expression_stmt");
		die();
	}

	$$->type = EXPR_STMT;
	$$->e = $1;
	$$->next = NULL;
};

statement : compound_stmt {
	$$ = malloc(sizeof(struct statement));
	if($$ == NULL) {
		perror("statement : compound_stmt");
		die();
	}

	$$->type = COMPOUND_STMT;
	$$->stmt_list = $1;
	$$->next = NULL;
};

statement : selection_stmt {
	$$ = malloc(sizeof(struct statement));
	if($$ == NULL) {
		perror("statement : selection_stmt");
		die();
	}

	$$->type = SEL_STMT;
	$$->sel = $1;
	$$->next = NULL;
};

statement : iteration_stmt {
	$$ = malloc(sizeof(struct statement));
	if($$ == NULL) {
		perror("statement : return_stmt");
		die();
	}

	$$->type = ITER_STMT;
	$$->iter = $1;
	$$->next = NULL;
};

statement : return_stmt {
	$$ = malloc(sizeof(struct statement));
	if($$ == NULL) {
		perror("statement : return_stmt");
		die();
	}

	$$->type = RETURN_STMT;
	$$->e = $1;
	$$->next = NULL;
};

expression_stmt : expression ';' {$$ = $1;};
expression_stmt : ';' {$$ = NULL;};

selection_stmt : IF '(' expression ')' statement {
	$$ = malloc(sizeof(struct selection_stmt));
	if($$ == NULL) {
		perror("selection_stmt : IF '(' expression ')' statement");
		die();
	}

	$$->e = $3;
	$$->t = $5;
	$$->f = NULL;
};

selection_stmt : IF '(' expression ')' statement ELSE statement {
	$$ = malloc(sizeof(struct selection_stmt));
	if($$ == NULL) {
		perror("selection_stmt : IF '(' expression ')' statement ELSE statement");
		die();
	}

	$$->e = $3;
	$$->t = $5;
	$$->f = $7;
};

iteration_stmt : WHILE '(' expression ')' statement {
	$$ = malloc(sizeof(struct iteration_stmt));
	if($$ == NULL) {
		perror("iteration_stmt : WHILE '(' expression ')' statement");
		die();
	}

	$$->e = $3;
	$$->body = $5;
};

return_stmt : RETURN ';' {$$ = NULL;};
return_stmt : RETURN expression ';' {$$ = $2;};

expression : var '=' expression {
	$$ = malloc(sizeof(struct expr));
	if($$ == NULL) {
		perror("expression : var \'=\' expression");
		die();
	}

	$$->dest_var = $1;
	$$->c_expr = $3;
};

expression : simple_expression {
	$$ = malloc(sizeof(struct expr));
	if($$ == NULL) {
		perror("expression : simple_expression");
		die();
	}

	$$->dest_var = NULL;
	$$->s_expr = $1;
};

var : ID {
	struct symbol* id = find_symbol_in_scope($1);
	if(id == NULL) {
		fprintf(stderr, "error: symbol \'%s\' not found in scope\n", $1);
		die();
	}

	$$ = malloc(sizeof(struct var_ref));
	if($$ == NULL) {
		perror("var : ID");
		die();
	}

	$$->id = id;
	$$->offset_expr = NULL;
};

var : ID '[' expression ']' {
	struct symbol* id = find_symbol_in_scope($1);
	if(id == NULL) {
		fprintf(stderr, "error: symbol \'%s\' not found in scope\n", $1);
		die();
	}

	$$ = malloc(sizeof(struct var_ref));
	if($$ == NULL) {
		perror("var : ID");
		die();
	}

	$$->id = id;
	$$->offset_expr = $3;
};

simple_expression : additive_expression relop additive_expression {
	$$ = malloc(sizeof(struct simple_expr));
	if($$ == NULL) {
		perror("simple_expression : additive_expression relop additive_expression");
		die();
	}

	$$->op = $2;
	$$->l = $1;
	$$->r = $3;
};

simple_expression : additive_expression {
	$$ = malloc(sizeof(struct simple_expr));
	if($$ == NULL) {
		perror("simple_expression : additive_expression");
		die();
	}

	$$->op = OP_NOP;
	$$->l = NULL;
	$$->r = $1;
};

relop : LTE      {$$ = OP_LTE;};
relop : '<'      {$$ = OP_LT;};
relop : '>'      {$$ = OP_GT;};
relop : GTE      {$$ = OP_GTE;};
relop : EQUAL    {$$ = OP_EQ;};
relop : NOTEQUAL {$$ = OP_NEQ;};

additive_expression : additive_expression addop term {
	$$ = malloc(sizeof(struct add_expr));
	if($$ == NULL) {
		perror("additive_expression : additive_expression addop term");
		die();
	}

	$$->op = $2;
	$$->l = $1;
	$$->r = $3;
};

additive_expression : term {
	$$ = malloc(sizeof(struct add_expr));
	if($$ == NULL) {
		perror("additive_expression : term");
		die();
	}

	$$->op = OP_NOP;
	$$->l = NULL;
	$$->r = $1;
};

addop : '+' {$$ = OP_ADD;};
addop : '-' {$$ = OP_SUB;};

term : term mulop factor {
	$$ = malloc(sizeof(struct term));
	if($$ == NULL) {
		perror("term : factor");
		die();
	}

	$$->op = $2;
	$$->l = $1;
	$$->r = $3;
};

term : factor {
	$$ = malloc(sizeof(struct term));
	if($$ == NULL) {
		perror("term : factor");
		die();
	}

	$$->op = OP_NOP;
	$$->l = NULL;
	$$->r = $1;
};

mulop : '*' {$$ = OP_MUL;};
mulop : '/' {$$ = OP_DIV;};

factor : '(' expression ')' {
	$$ = malloc(sizeof(struct factor));
	if($$ == NULL) {
		perror("factor : var");
		die();
	}

	$$->type = F_EXPR;
	$$->e = $2;
};

factor : var {
	$$ = malloc(sizeof(struct factor));
	if($$ == NULL) {
		perror("factor : var");
		die();
	}

	$$->type = F_VAR;
	$$->v = $1;
};

factor : call {
	$$ = malloc(sizeof(struct factor));
	if($$ == NULL) {
		perror("factor : call");
		die();
	}

	$$->type = F_CALL;
	$$->c = $1;
};

factor : NUM {
	$$ = malloc(sizeof(struct factor));
	if($$ == NULL) {
		perror("factor : NUM");
		die();
	}

	$$->type = F_NUM;
	$$->n = $1;
};

call : ID '(' args ')' {
	struct symbol* id = find_symbol_in_scope($1);
	if(id == NULL) {
		fprintf(stderr, "error: no function named \"%s\" declared\n", $1);
		die();
	}

	/* Check number of arguments and stack arguments for comparison with
   * parameters */
	struct arg_list** arg_stack = NULL;
	int arg_stack_size = 0;

	struct param* i = ((struct function*)(id->attr))->params;
	struct arg_list* j = $3;

	while(i != NULL && j != NULL) {
		arg_stack = realloc(arg_stack,
			(arg_stack_size + 1) * sizeof(struct arg_list*));
		if(arg_stack == NULL) {
			perror("call : ID '(' args ')'");
			die();
		}

		arg_stack[arg_stack_size] = j;
		arg_stack_size++;

		i = i->next;
		j = j->prev;
	}

	if(i == NULL && j != NULL) {
		fprintf(stderr, "error: too many arguments to function \"%s\"\n", id->id);
		die();
	} else if(i != NULL && j == NULL) {
		fprintf(stderr, "error: too few arguments to function \"%s\"\n", id->id);
		die();
	}

	/* Copy parameter types to corresponding arguments */
	i = ((struct function*)(id->attr))->params;

	while(i != NULL) {
		arg_stack_size--;
		arg_stack[arg_stack_size]->type = i->symbol->type;
		i = i->next;
	}

	free(arg_stack);

	/* Set up call structure */
	$$ = malloc(sizeof(struct call));
	if($$ == NULL) {
		perror("call : ID '(' args ')'");
		die();
	}

	$$->id = id;
	$$->args = $3;
};

args : arg_list {$$ = $1;};
args : %empty {$$ = NULL;};

arg_list : arg_list ',' expression {
	struct arg_list* arg = malloc(sizeof(struct arg_list));
	if(arg == NULL) {
		perror("arg_list : arg_list ',' expression");
		die();
	}
	
	arg->e = $3;
	arg->prev = $1;

	$$ = arg;
};

arg_list : expression {
	$$ = malloc(sizeof(struct arg_list));
	if($$ == NULL) {
		perror("arg_list : expression");
		die();
	}

	$$->e = $1;
	$$->prev = NULL;
};

%%

int main() {
	push_scope();
	yyparse();
	set_ebp_offsets(program_decl_list);
	generate_asm(program_decl_list);
}
