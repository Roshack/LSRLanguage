#ifndef LSR_CLASSES_H
#define LSR_CLASSES_H
#include <iostream>
#include <vector>
#include <map>
#include <set>
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
    void * objPtr;
    LSRValue(long long iv);
    LSRValue(const LSRValue &v);
    LSRValue() : intVal(0) {}
    LSRValue(std::string st, int size);
    LSRValue(std::string className);
    void *createInMemory(void *classD);
    long long getIntVal() const;
    std::string getStrVal() const;
    int isStr() const;
    int isInt() const;
    int isClass() const;
    std::string toString() const;
    std::string getType();
    void * getObjectPointer();

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

LSRValue getInitializedVal(std::string type);


#endif

