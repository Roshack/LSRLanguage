%{
#include "lsr_classes.hpp"
#include <stdio.h>

extern int yylex();
void yyerror(char *s) {
  fprintf(stderr, "%s\n", s);
}

Scope *curScope;
LSRBlock *programBlock;
LSRClassTable *classes;
std::string intStr = "int";
std::string strStr = "str";
std::string currentClass = "";
%}


%union {
    LSRStmt *stmt;
    LSRExpr *expr;
    LSRVarDecl *var_decl;
    LSRIdent *ident;
    LSRMemberAccess *member;
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
%token <token> TPRINT TPRINTLN
%token <token> TVOIDTYPE TFNDEF TMAIN TSEMICOLON TCLASS TDOT

%type <expr> expr numeric strliteral
%type <stmt> stmt decl print assign
%type <ident> ident
%type <member> memberaccess
%type <block> program maindef block
%type <string> vartype

/* temp until symbol table shenanigans */
%token ID

/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV

%%

program: {curScope = new Scope(NULL); classes = new LSRClassTable();} classdefs maindef {programBlock = $3; delete curScope; delete classes;};

classdefs: classdef
         | classdefs classdef
         |
         ;

classdef: TCLASS ident {classes->add($2->getName()); currentClass = $2->getName();} classblock 
        {
            classes->setDescriptorPointer(currentClass);
            currentClass = "";
            
        };

classblock: TLBRACE cdecls TRBRACE;

cdecls  : cdecl TSEMICOLON
        | cdecls cdecl TSEMICOLON
        ;

cdecl   : vartype ident {classes->addVar(currentClass,$2->getName(),*$1);};

maindef: TFNDEF TVOIDTYPE TMAIN TLPAREN TRPAREN block {$$ = $6;};

vartype : TINTTYPE {$$ = &intStr;}
        | TSTRTYPE {$$ = &strStr;}
        | ident    {std::string temp = $1->getName(); $$ = &temp;};
        ;

block: {curScope = new Scope(curScope);} TLBRACE stmts TRBRACE {Scope *temp = curScope; curScope = curScope->getParent(); delete temp;};

stmts: stmt TSEMICOLON
     | stmts stmt TSEMICOLON
     ;

stmt: decl
    | print
    | assign
    ;

decl: vartype ident {curScope->decl($2->getName(), *$1, (void *)classes);}
    | vartype ident TEQUAL expr 
    {
        curScope->decl($2->getName(), *$1, (void *)classes);
        curScope->assign($2->getName(), $4->getVal(), (void *)classes);
    }    
    ;

assign: ident TEQUAL expr { curScope->assign($1->getName(), $3->getVal(),(void *)classes);}
      | memberaccess TEQUAL expr 
        {
            curScope->memberAssign($1->getParent(),$1->getChild(), $3->getVal(), (void *)classes);
        }
      ;

print: TPRINT TLPAREN expr TRPAREN {std::cout<< $3->getString();}
     | TPRINTLN TLPAREN expr TRPAREN {std::cout<< $3->getString() <<std::endl;}
     ;

expr: ident   {$$ = new LSRExpr(curScope->resolve($1->getName())); }
    | memberaccess {$$ = new LSRExpr(curScope->resolveMembers((void*) $1,(void *) classes));}
    | numeric 
    | strliteral
    | expr TPLUS expr {$$ = new LSRExpr(*$1 + *$3); delete $1; delete $3;}
    | TLPAREN expr TRPAREN {$$ = $2;}
    ;

numeric : TINT { $$ = new LSRInt(atol($1->c_str())); delete $1; } ;

strliteral: TSTRINGLIT { $$ = new LSRStr(*$1); delete $1; } ;

ident : TID { $$ = new LSRIdent(*$1); delete $1; }

memberaccess : ident TDOT ident 
             {
                $$ = new LSRMemberAccess($1->getName(), $3->getName());
                delete $1; 
                delete $3;
             };




