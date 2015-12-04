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
LSRFunctionTable *functions;
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
    exprNode *exprnode;
    stmtNode *stmtnode;
    whileNode *whilenode;
    StmtList *stmtlist;
};


%token <string> TID TINT TINTTYPE TSTRTYPE TSTRINGLIT
%token <token> TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE
%token <token> TPLUS TMINUS TMUL TDIV
%token <token> TLT TGT
%token <token> TPRINT TPRINTLN
%token <token> TVOIDTYPE TFNDEF TMAIN TSEMICOLON TCLASS TDOT TWHILE

%type <expr> expr numeric strliteral
%type <stmt> stmt decl print assign
%type <ident> ident
%type <member> memberaccess
%type <block> program maindef block
%type <string> vartype
%type <stmtnode> deferstmt deferprint deferassign while
%type <exprnode> cond deferexpr deferident defernumeric
%type <stmtlist> deferblock deferstmts


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
    | while {$1->execute(curScope,classes,functions);delete $1;}
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

while: TWHILE TLPAREN cond TRPAREN deferblock
        {
            whileNode *temp = new whileNode(*$3);
            StmtList::iterator it = $5->begin();
            while (it != $5->end()) {
                temp->stmts.push_back(*it);
                it++;
            }
            $$ = temp;
        }
        ;

cond: deferexpr TLT deferexpr {$$= new condNode(*$1, LT_OP, *$3);};

deferblock: TLBRACE deferstmts TRBRACE {$$ = $2;};

deferstmts: deferstmt TSEMICOLON {$$ = new StmtList(); $$->push_back($<stmtnode>1);}
          | deferstmts deferstmt TSEMICOLON {$1->push_back($<stmtnode>2);}
          ;

deferstmt: deferprint {$$ = $1;}
         | deferassign {$$ = $1;;}
         ;

deferprint: TPRINT TLPAREN deferexpr TRPAREN {$$ = new printNode(*$3,0); /*delete $3;*/}
          | TPRINTLN TLPAREN deferexpr TRPAREN {$$ = new printNode(*$3,1); /*delete $3;*/}
          ;

deferassign: deferident TEQUAL deferexpr {$$ = new assignNode(*$1,*$3);/* delete $1; delete $3;*/};

deferexpr: deferident {$$ = $1;}
         | defernumeric {$$ = $1;}
         | deferexpr TPLUS deferexpr {$$ = new binaryExprNode(*$1, PLUS_OP, *$3); /*delete $1; delete $3;*/}
         ;

deferident : TID {$$ = new varNode(*$1); /*delete $1;*/};

defernumeric : TINT {$$ = new intNode(atol($1->c_str())); /*delete $1;*/}; 

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




