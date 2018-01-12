/**
 * C-Minus symbol table generator
 *
 * Conner Short
 * December 2017
 * CMPE 152
 *
 * support.h: Miscellaneous support functions
**/
#ifndef _SUPPORT_H
#define _SUPPORT_H

#include "nt.h"
#include "symbol.h"

void die();
void set_ebp_offsets(struct declaration* decl_list);

#endif
