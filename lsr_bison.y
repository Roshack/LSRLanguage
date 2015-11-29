%{
#include "lsr_classes.hpp"
#include <stdio.h>
extern int yylex();
void yyerror(char *s) {
  fprintf(stderr, "%s\n", s);
}

Scope *curScope;
LSRBlock *programBlock;

%}


%union {
    LSRStmt *stmt;
    LSRExpr *expr;
    LSRVarDecl *var_decl;
    LSRIdent *ident;
    LSRBlock *block;
    std::string *string;
    int token;
    int var;
};


%token <string> TID TINT
%token <token> TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE
%token <token> TPLUS TMINUS TMUL TDIV
%token <token> TPRINT
%token <token> TVOIDTYPE TINTTYPE TFNDEF TMAIN TSEMICOLON

%type <expr> expr numeric
%type <stmt> stmt decl print assign
%type <ident> ident
%type <block> program maindef block

/* temp until symbol table shenanigans */
%token ID

/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV

%%

program: {Scope temp = Scope(NULL); curScope = &temp;} maindef {programBlock = $2;};

maindef: TFNDEF TVOIDTYPE TMAIN TLPAREN TRPAREN block {$$ = $6;};

vartype: TINTTYPE;

block: {Scope temp = Scope(curScope); curScope = &temp;} TLBRACE stmts TRBRACE {curScope = curScope->getParent();};

stmts: stmt TSEMICOLON
     | stmts stmt TSEMICOLON
     ;

stmt: decl
    | print
    | assign
    ;

decl: vartype ident {curScope->decl($2->getName());} ;

assign: ident TEQUAL expr 
    { 
    curScope->assign($1->getName(), $3->getVal()); 
    } ;

print: TPRINT TLPAREN expr TRPAREN {std::cout<< $3->getString()<<std::endl;};

expr: ident   {$$ = new LSRExpr(curScope->resolve($1->getName())); }
    | numeric 
    | expr TPLUS expr 
    {
        LSRExpr retval = *$1 + *$3; $$ = &retval;
    }
    | TLPAREN expr TRPAREN {$$ = $2;}
    ;

numeric : TINT { $$ = new LSRInt(atol($1->c_str())); delete $1; } ;

ident : TID { $$ = new LSRIdent(*$1); delete $1; }




