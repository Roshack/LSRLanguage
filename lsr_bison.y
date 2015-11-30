%{
#include "lsr_classes.hpp"
#include <stdio.h>
extern int yylex();
void yyerror(char *s) {
  fprintf(stderr, "%s\n", s);
}

Scope *curScope;
LSRBlock *programBlock;
std::string intStr = "int";
std::string strStr = "str";
%}


%union {
    LSRStmt *stmt;
    LSRExpr *expr;
    LSRVarDecl *var_decl;
    LSRIdent *ident;
    LSRBlock *block;
    LSRStr *strlit;
    std::string *string;
    int token;
    int var;
};


%token <string> TID TINT TINTTYPE TSTRTYPE TSTRINGLIT
%token <token> TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE
%token <token> TPLUS TMINUS TMUL TDIV
%token <token> TPRINT
%token <token> TVOIDTYPE TFNDEF TMAIN TSEMICOLON TCLASS

%type <expr> expr numeric strliteral
%type <stmt> stmt decl print assign
%type <ident> ident
%type <block> program maindef block //classdef classblock
%type <string> vartype

/* temp until symbol table shenanigans */
%token ID

/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV

%%

program: {curScope = new Scope(NULL); /* ehhh no glboal scope? */} /*classdefs*/ maindef {programBlock = $2; delete curScope;};
/*
classdefs: classdef
         | classdefs classdef
         ;

classdef: TCLASS ident classblock;

classblock: TLBRACE cdecls TRBRACE;

cdecls  : cdecl
        | cdecls cdecl
        ;

cdecl   : vartype ident;
*/
maindef: TFNDEF TVOIDTYPE TMAIN TLPAREN TRPAREN block {$$ = $6;};

vartype : TINTTYPE {$$ = &intStr;}
        | TSTRTYPE {$$ = &strStr;}
        ;

block: {curScope = new Scope(curScope);} TLBRACE stmts TRBRACE {Scope *temp = curScope; curScope = curScope->getParent(); delete temp;};

stmts: stmt TSEMICOLON
     | stmts stmt TSEMICOLON
     ;

stmt: decl
    | print
    | assign
    ;

decl: vartype ident {curScope->decl($2->getName(), *$1);}
    | vartype ident TEQUAL expr 
    {
        curScope->decl($2->getName(), *$1);
        curScope->assign($2->getName(), $4->getVal());
    }    
    ;

assign: ident TEQUAL expr 
    { 
    curScope->assign($1->getName(), $3->getVal()); 
    } ;

print: TPRINT TLPAREN expr TRPAREN {std::cout<< $3->getString()<<std::endl;};

expr: ident   {$$ = new LSRExpr(curScope->resolve($1->getName())); }
    | numeric 
    | strliteral
    | expr TPLUS expr {$$ = new LSRExpr(*$1 + *$3);}
    | TLPAREN expr TRPAREN {$$ = $2;}
    ;

numeric : TINT { $$ = new LSRInt(atol($1->c_str())); delete $1; } ;

strliteral: TSTRINGLIT { $$ = new LSRStr(*$1); delete $1; } ;

ident : TID { $$ = new LSRIdent(*$1); delete $1; }




