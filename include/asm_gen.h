/**
 * C-Minus symbol table generator
 *
 * Conner Short
 * December 2017
 * CMPE 152
 *
 * asm_gen.h: Assembly code generation routines
**/
#ifndef _ASM_GEN_H
#define _ASM_GEN_H

#include "nt.h"
#include "symbol.h"

void generate_asm(struct declaration* decl_list);

#endif
