#ifndef LSR_CLASSES_H
#define LSR_CLASSES_H
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include "gc/ggggc/gc.h"

using namespace std;


class LSRStmt;
class LSRExpr;
class LSRVarDecl;
class SymbolTable;
class LSRBlock;
class LSRIdent;
class LSRInt;

class Scope;
typedef std::vector<LSRStmt*> StmtList;
typedef std::vector<LSRExpr*> ExprList;
typedef std::vector<LSRVarDecl*> VarList;


class LSRValue {
public:
    long long intVal;
    std::string strVal;
    std::string className;
    int type;
    LSRValue(long long iv);
    LSRValue(const LSRValue &v);
    LSRValue() : intVal(0) {}
    LSRValue(std::string st);
    long long getIntVal() const;
    std::string getStrVal() const;
    int isStr() const;
    int isInt() const;
    std::string toString() const;

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

class LSRInt : public LSRExpr {
public:
    LSRInt(long long value);
    
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
    LSRValue get(std::string id);
};


class Scope {
public:
    Scope(Scope *p);
    int isTopLevel();
    Scope *getParent();
    void decl(std::string id, std::string type);
    void assign(std::string id, LSRValue val);
    LSRValue resolve(std::string id);

private:
    Scope *parent;
    SymbolTable st;
};

LSRExpr LSRAdd(LSRExpr lhs, LSRExpr rhs);


#endif

