%option noyywrap
%option c++

%{
#include "tokens.h"

#ifdef _WIN32
#define fileno _fileno
#endif
%}

%%
"QUOTE"												{ return QUOTE; }
"CAR"										{ return CAR; }
"CDR"											{ return CDR; }
"("												{ return LPAREN; }
")"												{ return RPAREN; }
"CONS"											{ return CONS; }
"ATOM"										{ return ATOM; }
"EQUAL"												{ return EQUAL; }
"ADD"				{ return ADD; }
"SUB"										{ return SUB; }
"MUL"											{ return MUL; }
"DIVE"											{ return DIVE; }
"REM"	{ return REM; }
"LEQ"											{ return LEQ; }
"COND"											{ return COND; }
"LAMBDA"										{ return LAMBDA; }
"LET"							{ return LET; }
"LETREC"										{ return LETREC; }
"LET"							{ return LET; }
[0-9]+											{ return INTEGER; }
[a-zA-Z][a-zA-Z0-9_]*(?:\.[a-zA-Z0-9_]+)?		{ return IDENTIFIER; }
\n												{ /* ignore newlines */ }
" "												{ /* ignore space */ }
.												{ return UNKNOWN; }
<<EOF>>											{ return END_OF_FILE; }
%%