%{
#include <string>
#include "lsr_classes.hpp"
#include "lsr_bison.hpp"
#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)
extern "C" int yywrap() { }
%}

%%

[ tn]                 ;
"print"                 return TOKEN(TPRINT);
"fn"                    return TOKEN(TFNDEF);
"int"                   return TOKEN(TINTTYPE);
"str"                   return TOKEN(TSTRTYPE);
"void"                  return TOKEN(TVOIDTYPE);
"main"                  return TOKEN(TMAIN);
"class"                 return TOKEN(TCLASS);
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return TID;
[0-9]+                  SAVE_TOKEN; return TINT;
"="                     return TOKEN(TEQUAL);
"+"                     return TOKEN(TPLUS);
"-"                     return TOKEN(TMINUS);
"*"                     return TOKEN(TMUL);
"/"                     return TOKEN(TDIV);
"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"{"                     return TOKEN(TLBRACE);
"}"                     return TOKEN(TRBRACE);
";"                     return TOKEN(TSEMICOLON);
"."                     return TOKEN(TDOT);
L?\"(\\.|[^\\"])*\"	    SAVE_TOKEN; return TSTRINGLIT;


.                       printf("Unknown token!n"); yyterminate();

%%
