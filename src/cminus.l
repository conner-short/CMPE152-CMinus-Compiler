/**
 * Lex scanner file for C Minus language
 * Conner Short
 * October 24, 2017
**/
%{
#include <stdio.h>
#include <stdlib.h>

#include "nt.h"
#include "symbol.h"
#include "y.tab.h"
%}

%%

"else"                  {return ELSE;}
"if"                    {return IF;}
"int"                   {return INT;}
"return"                {return RETURN;}
"void"                  {return VOID;}
"while"                 {return WHILE;}

[_a-zA-Z][_a-zA-Z0-9]*  {
	yylval.string = strdup(yytext); 
	if(yylval.string == NULL) {
		perror("scanner: ID");
		exit(EXIT_FAILURE);
	}

	return ID;
}

\-?[0-9]+               {yylval.integer = atoi(yytext); return NUM;}

"<="                    {return LTE;}
">="                    {return GTE;}
"=="                    {return EQUAL;}
"!="                    {return NOTEQUAL;}

[ \t\n]+                {}
.                       {return yytext[0];}

%%
