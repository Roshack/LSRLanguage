#ifndef LSR_CLASSES_H
#define LSR_CLASSES_H
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <list>
#include "gc/ggggc/gc.h"

using namespace std;


class LSRStmt;
class LSRExpr;
class LSRVarDecl;
class SymbolTable;
class LSRBlock;
class LSRIdent;
class LSRInt;
class deferNode;
class stmtNode;
class whileNode;
class condNode;
class exprNode;
class declNode;
class varNode;
class memberNode;
class printNode;
class assignNode;
class LSRFunctionTable;
class LSRClassTable;
class Scope;
class nodeValue;

class Scope;
typedef std::vector<stmtNode*> StmtList;
typedef std::vector<exprNode*> ExprList;
typedef std::vector<declNode*> VarList;

enum BinaryOp {
PLUS_OP,
MINUS_OP,
MOD_OP,
MUL_OP,
DIV_OP
};

enum CondOp {
LT_OP,
LEQ_OP,
EQ_OP,
GT_OP,
GEQ_OP
};


class LSRValue {
public:
    ggc_size_t intVal;
    std::string strVal;
    std::string className;
    int type;
    void * objPtr;
    LSRValue(ggc_size_t iv);
    LSRValue(const LSRValue &v);
    LSRValue() : intVal(0) {}
    LSRValue(std::string st, int size);
    LSRValue(std::string className);
    void *createInMemory(void *classD);
    ggc_size_t getIntVal() const;
    std::string getStrVal() const;
    int isStr() const;
    int isInt() const;
    int isClass() const;
    std::string toString() const;
    std::string getType();
    void * getObjectPointer();
    int operator<(const LSRValue &rhs);

};

class Node {
public:
    virtual ~Node() {}
};

class LSRBlock : public Node {

};

class LSRStmt : public Node {
    
};

class LSRExpr : public Node {
public:
    LSRValue val;
    LSRExpr(const LSRValue &v);
    LSRExpr(const LSRExpr &expr);
    LSRExpr(std::string st) : val(st) {}
    LSRExpr() : val(0) {}
    LSRValue getVal();
    std::string getString();
    LSRExpr operator+(const LSRExpr &rhs);
};


class LSRVarDecl : public LSRStmt {

};

class LSRIdent : public LSRExpr {
public:
    std::string name;
    LSRIdent(const std::string& n) : name(n) { }
    std::string getName();
};

class LSRMemberAccess : public LSRExpr {
public:
    std::string parent;
    std::string child;
    LSRMemberAccess(const std::string& p, const std::string& c);
    std::string getParent();
    std::string getChild();
};

class LSRInt : public LSRExpr {
public:
    LSRInt(ggc_size_t value);
    
};

class LSRStr : public LSRExpr {
public:
    LSRStr(std::string st);
};


class SymbolTable {
public:
    std::map<std::string, LSRValue> symMap;
    int contains(std::string id);
    void add(std::string id, LSRValue val);
    void set(std::string id, LSRValue val);
    void * descriptorPointer;
    void setDescriptorPointer();
    LSRValue get(std::string id);
    long unsigned int getIndex(std::string member);
    std::string getMemberType(std::string member);
    void * createInMemory(std::string id,void * classDefs);
    void * getDescriptorPointer();
};


class Scope {
public:
    Scope(Scope *p);
    int isTopLevel();
    Scope *getParent();
    void decl(std::string id, std::string type, void *classDefs);
    void assign(std::string id, LSRValue val, void *classDefs);
    void memberAssign(std::string parent, std::string child, LSRValue val,void *classDefs);
    LSRValue resolveMembers(void * ma, void * ct);
    LSRValue resolve(std::string id);

private:
    Scope *parent;
    SymbolTable st;
};

class LSRClassTable {
public:
    std::map<std::string, SymbolTable> classDefs;
    void add(std::string className);
    int contains(std::string className);
    void addVar(std::string className,std::string varName, std::string type);
    void setDescriptorPointer(std::string className);
    void* getDescriptorPointer(std::string className);
    long unsigned int getOffset(std::string classname, std::string membername);
    std::string getType(std::string classname, std::string member);
};

class LSRFunctionTable {

};

/*
*
*
*       DEFERRAL STUFF PAST HERE.
*       THIS IS STUFF FOR FNS, WHILE LOOPS, ETC.
*       SHOULD HAVE DONE IT THIS WAY FOR EVERYTHING BUT
*       FORGOT HOW TO WRITE AN INTERPRETER TIL NOW
*
*
*/

enum nodeValueType{
NVINT,
NVSTR,
NVVAR,
NVMEMB
};

class nodeValue {
public:
    std::string varName;
    std::string memberName;
    ggc_size_t intVal;
    ggc_size_t objPtr;
    std::string st;
    int strLen;
    nodeValueType type;
    int operator<(const nodeValue &rhs);
    nodeValue operator+(const nodeValue &rhs);
    nodeValue() {}
    nodeValue(ggc_size_t x) : intVal(x) {}
    LSRValue toVal(Scope * scope, LSRClassTable * classDefs);
    ~nodeValue() {}
};

class deferNode {
public:
    virtual ~deferNode() {}
    virtual nodeValue execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions) {};
};

class exprNode : public deferNode {
public:
    virtual int isLValue() {return 0;}
    virtual int isMember() {return 0;}
};

class stmtNode : public deferNode {

};


class binaryExprNode : public exprNode {
public:
    BinaryOp op;
    exprNode& lhs;
    exprNode& rhs;
    binaryExprNode(exprNode& lhs, BinaryOp op, exprNode& rhs) : lhs(lhs), rhs(rhs), op(op) {}
    nodeValue execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions);
};

class condNode : public exprNode {
public:
    CondOp op;
    exprNode& lhs;
    exprNode& rhs;
    condNode(exprNode& lhs, CondOp op, exprNode& rhs) : lhs(lhs), rhs(rhs), op(op) {}
    nodeValue execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions);
};


class varNode : public exprNode {
public:
    std::string varName;
    int isLValue() {return 1;}
    int isMember() {return 0;}
    varNode(std::string v) : varName(v) {}
    nodeValue execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions);
};

class memberNode : public exprNode {
public:
    std::string varName;
    std::string memberName;
    int isLValue() {return 1;}
    int isMember() {return 1;}
    memberNode(std::string v, std::string m) : varName(v), memberName(m) {}
    nodeValue execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions);
};

class intNode : public exprNode {
public:
    ggc_size_t intVal;
    intNode(ggc_size_t x) : intVal(x) {}
    nodeValue execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions);
};

class declNode : public stmtNode {
public:
    std::string varName;
    std::string type;
    declNode(std::string& v,  std::string& t) : varName(v), type(t) {}
    nodeValue execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions);
};

class assignNode : public stmtNode {
public:
    exprNode& lhs;
    exprNode& rhs;
    assignNode(exprNode& lhs, exprNode& rhs) : lhs(lhs), rhs(rhs) {}
    nodeValue execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions);
};

class printNode : public stmtNode {
public:
    exprNode& expr;
    int printLine;
    printNode(exprNode& e, int p) : expr(e), printLine(p) {}
    nodeValue execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions);
};

class whileNode : public stmtNode {
public:
    exprNode& cond;
    StmtList stmts;
    whileNode(exprNode& c) : cond(c) {}
    ~whileNode();
    nodeValue execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions);
};

LSRValue getInitializedVal(std::string type);;


#endif

