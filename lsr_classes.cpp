#include "lsr_classes.hpp"


std::string LSRValue::toString() {
    std::stringstream ss;
    ss << intVal;
    return ss.str();
}

LSRValue::LSRValue(const LSRValue &v) {
    intVal = v.intVal;
}

long long LSRValue::getVal() const {
    return intVal;
}

Scope::Scope(Scope *p) {
    parent = p;
}

int Scope::isTopLevel() {
    return parent == NULL;
}

Scope *Scope::getParent() {
    return parent;
}

void Scope::decl(std::string id) {
    if (st.contains(id)) {
        std::cout << "RepeatIDError: ID " << id << " has already been declared in this scope." << std::endl;
    } else {
        LSRValue v = LSRValue(0);
        st.add(id, v);
    }
}

void Scope::assign(std::string id, LSRValue val) {
    if (st.contains(id)) {
        st.set(id,val);
    } else if (!isTopLevel()) {
        parent->assign(id,val);
    } else {
        std::cout << "UndefinedIDError: ID " << id << " has not been declared in this scope." << std::endl;
    }
}

LSRValue Scope::resolve(std::string id) {
    if (st.contains(id)) {
        return st.get(id);
    } else if (!isTopLevel()) {
        return parent->resolve(id);
    } else {
        std::cout << "UndefinedIDError: ID " << id << " has not been declared in this scope." << std::endl;
        return LSRValue(0);
    }
}

int SymbolTable::contains(std::string id) {
    std::map<std::string, LSRValue>::iterator search = symMap.find(id);
    return search!=symMap.end();
}
void SymbolTable::add(std::string id, LSRValue val) {
    symMap[id] = val;
}
LSRValue SymbolTable::get(std::string id) {
    std::map<std::string, LSRValue>::iterator search = symMap.find(id);
    return search->second;
}
void SymbolTable::set(std::string id, LSRValue val) {
    symMap[id] = val;
}

std::string LSRIdent::getName() {
    return name;
}


LSRExpr::LSRExpr(const LSRValue &v){
    val = v;
}
/*
LSRExpr LSRExpr::operator+(const LSRExpr &rhs) {
    std::cout << "This val is " << this->val.intVal << std::endl;
    std::cout << "rhs val is " << rhs.val.intVal << std::endl;
    LSRValue v = LSRValue(this->val.intVal + rhs.val.intVal);
    LSRExpr ret = LSRExpr(v);
    return ret;
}*/
LSRExpr operator+(const LSRExpr& lhs, const LSRExpr& rhs) {
    LSRValue v=  LSRValue(lhs.val.getVal() + rhs.val.getVal());
    LSRExpr ret = LSRExpr(v);
    return ret;
}

LSRValue LSRExpr::getVal() {
    return val;
}

std::string LSRExpr::getString() {
    return val.toString();
}

LSRInt::LSRInt(long long value) {
    val = LSRValue(value);
}

