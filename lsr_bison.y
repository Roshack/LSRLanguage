%{
#include "lsr_classes.hpp"
#include <stdio.h>
#include "gc/ggggc/gc.h"

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
LSRScope *ptrScope;
LSRScope *fnPtrScope;
Scope *fnScope;
int fnCallCount;
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
    accessList *accesslist;
    StmtList *stmtlist;
    LSRParam * param;
    argList *args;
    std::vector<LSRParam*> * paramlist;
};


%token <string> TID TINT TINTTYPE TSTRTYPE TSTRINGLIT
%token <token> TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE
%token <token> TPLUS TMINUS TMUL TDIV
%token <token> TLT TGT
%token <token> TPRINT TPRINTLN
%token <token> TVOIDTYPE TFNDEF TMAIN TSEMICOLON TCLASS TDOT TWHILE TCOMMA TFNMAIN

%type <expr> expr numeric strliteral
%type <stmt> stmt decl print assign fncall
%type <ident> ident
%type <member> memberaccess
%type <accesslist> accesses
%type <block> program maindef block
%type <string> vartype cvartype
%type <stmtnode> deferstmt deferprint deferassign while deferdecl
%type <exprnode> cond deferexpr deferident defernumeric defermemberaccess deferstrlit
%type <paramlist> params
%type <param> param
%type <args> arglist
%type <stmtlist> deferblock deferstmts


/* temp until symbol table shenanigans */
%token ID

/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV

%%

program:{
            curScope = new Scope(NULL); 
            ptrScope = new LSRScope(NULL);
            curScope->ptrScope = (&ptrScope);
            fnPtrScope = new LSRScope(NULL);
            fnScope = new Scope(NULL);
            fnScope->ptrScope = (&fnPtrScope);
            fnScope->fnCallCount = 0;
            setLSRScope(&ptrScope);
            setFnScope(&fnPtrScope);
            classes = new LSRClassTable();
            functions = new LSRFunctionTable;
        } 
        fndefs classdefs maindef 
        {
            programBlock = $4; 
            delete curScope; 
            delete classes;
            delete ptrScope;
        };

fndefs : fndef
       | fndefs fndef
       |
       ;

fndef   : TFNDEF TVOIDTYPE ident TLPAREN params TRPAREN deferblock
        {
            StmtList *temp = new StmtList;
            StmtList::iterator it = $7->begin();
            while (it != $7->end()) {
                temp->push_back(*it);
                it++;
            }
            functions->add($3->getName(),$5,temp);
        }
        ;

params  : param {$$ = new std::vector<LSRParam*>(); $$->push_back($1); }
        | params TCOMMA param {$$ = $1; $$->push_back($3);}
        | {$$ = new std::vector<LSRParam*>();}
        ;

param   : vartype ident {std::string temp = $2->getName();$$ = new LSRParam(temp, *$1);} ;

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

cdecl   : cvartype ident {classes->addVar(currentClass,$2->getName(),*$1);};

cvartype : TINTTYPE {$$ = &intStr;}
         | ident    {std::string temp = $1->getName(); $$ = new std::string(temp);};
         ;

maindef: TFNMAIN TVOIDTYPE TMAIN TLPAREN TRPAREN block {$$ = $6;};

vartype : TINTTYPE {$$ = &intStr;}
        | TSTRTYPE {$$ = &strStr;}
        | ident    {std::string temp = $1->getName(); $$ = new std::string(temp);};
        ;

block: {curScope = new Scope(curScope); ptrScope = new LSRScope(ptrScope); setLSRScope(&ptrScope); } TLBRACE stmts TRBRACE {Scope *temp = curScope; curScope = curScope->getParent(); delete temp;
         LSRScope *t = ptrScope; ptrScope = ptrScope->getParent(); delete t;};

stmts: stmt TSEMICOLON
     | stmts stmt TSEMICOLON
     ;

stmt: decl
    | print
    | assign
    | while {$1->execute(curScope,classes,functions);delete $1;}
    | fncall
    ;

fncall : ident TLPAREN arglist TRPAREN
       {
            std::vector<LSRParam*> params = functions->fnParams[$1->getName()];
            fnCallCount++;
            fnScope = new Scope(fnScope);
            fnScope->fnCallCount = fnCallCount;
            std::vector<LSRParam*>::iterator it = params.begin();
            argList::iterator at = $3->begin();
            while ( it != params.end()) {
                if (at == $3->end()) {
                    std::cout << "mismatched nubmer of params and args for function " << $1->getName() << std::endl;
                }
                fnScope->decl((*it)->varName,(*it)->varType, classes);
                fnScope->assign((*it)->varName, (*at)->getVal(),classes);
                it++;
                at++;
            }
            // done setting up scopes...
            StmtList::iterator st = functions->fns[$1->getName()]->begin();
            while (st != functions->fns[$1->getName()]->end()) {
                (*st)->execute(fnScope,classes,functions);
                st++;
            }
            Scope * temp = fnScope;
            fnScope = fnScope->getParent();
            delete temp;
       }
       ;

arglist : expr {$$ = new argList; $$->push_back($1);}
        | arglist TCOMMA expr {$$ = $1; $$->push_back($3);}
        | {$$ = new argList;}
        ;

decl: vartype ident     
    {
        curScope->decl($2->getName(), *$1, (void *)classes);
        LSRValue v = curScope->resolve($2->getName());
        if (v.isClass()) {
            ptrScope->addPtr($2->getName(),v.getObjectPointer());
        }

    }
    | vartype ident TEQUAL expr 
    {
        curScope->decl($2->getName(), *$1, (void *)classes);
        curScope->assign($2->getName(), $4->getVal(), (void *)classes);
        // this decl is used for decls in the main scope only... these objects
        // should never die until the program is over... because the way scope
        // works is not enough to push where their objectptr is saved in the scope.
        LSRValue v = curScope->resolve($2->getName());
        if (v.isClass()) {
            ptrScope->addPtr($2->getName(),v.getObjectPointer());
        }
    }    
    ;

assign: ident TEQUAL expr { curScope->assign($1->getName(), $3->getVal(),(void *)classes);}
      | memberaccess TEQUAL expr 
        {
            curScope->memberAssign($1, $3->getVal(), (void *)classes);
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
          | deferstmts deferstmt TSEMICOLON {$1->push_back($<stmtnode>2); }
          ;

deferstmt: deferprint {$$ = $1;}
         | deferassign {$$ = $1;}
         | deferdecl {$$ = $1;}
         | while {$$=$1;}
         ;

deferprint: TPRINT TLPAREN deferexpr TRPAREN {$$ = new printNode(*$3,0); /*delete $3;*/}
          | TPRINTLN TLPAREN deferexpr TRPAREN {$$ = new printNode(*$3,1); /*delete $3;*/}
          ;

deferassign : deferident TEQUAL deferexpr {$$ = new assignNode(*$1,*$3);}
            | defermemberaccess TEQUAL deferexpr {$$ = new assignNode(*$1,*$3);}
            ;

deferdecl   : vartype ident 
            {std::string vname = $2->getName(); $$ = new declNode(vname, *$1); 
            };

deferexpr: deferident {$$ = $1;}
         | defermemberaccess {$$ = $1;}
         | defernumeric {$$ = $1;}
         | deferexpr TPLUS deferexpr {$$ = new binaryExprNode(*$1, PLUS_OP, *$3); /*delete $1; delete $3;*/}
         | deferstrlit {$$ = $1;}
         | TLPAREN deferexpr TRPAREN {$$ = $2;}
         ;

deferident : TID {$$ = new varNode(*$1); /*delete $1;*/};

defermemberaccess : memberaccess
                    {
                        $$ = new memberNode($1);
                    }
                   ; 

defernumeric : TINT {$$ = new intNode(atol($1->c_str())); /*delete $1;*/}; 

deferstrlit : TSTRINGLIT { $$ = new strNode(*$1);};

expr: ident   {$$ = new LSRExpr(curScope->resolve($1->getName())); }
    | memberaccess {$$ = new LSRExpr(curScope->resolveMembers((void*) $1,(void *) classes));}
    | numeric 
    | strliteral
    | expr TPLUS expr {$$ = new LSRExpr(*$1 + *$3); delete $1; delete $3;}
    | TLPAREN expr TRPAREN {$$ = $2;}
    ;

numeric : TINT { $$ = new LSRInt(atol($1->c_str())); delete $1; } ;

strliteral: TSTRINGLIT { $$ = new LSRStr(*$1); delete $1; } ;

ident : TID { $$ = new LSRIdent(*$1); delete $1; };

memberaccess : ident TDOT ident 
             {
                $$ = new LSRMemberAccess($1->getName(), $3->getName());
                $$->list = NULL;
                delete $1; 
                delete $3;
             }
             | ident TDOT ident accesses
             {
                $$ = new LSRMemberAccess($1->getName(), $3->getName());
                delete $1; 
                delete $3;
                $$->list = $4;
             }
             ;

accesses    : TDOT ident
            {
                $$ = new accessList;
                $$->id = $2->getName();
                $$->next = NULL;
                delete $2;
            }
            | TDOT ident accesses
            {
                $$ = new accessList;
                $$->id = $2->getName();
                delete $2;
                $$->next = $3;
            }
            ;




