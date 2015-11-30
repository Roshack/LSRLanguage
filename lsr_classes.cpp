#include "lsr_classes.hpp"


std::string LSRValue::toString() const {
    if (isInt()) {
        std::stringstream ss;
        ss << intVal;
        return ss.str();
    } else if (isStr()) {
        return strVal;
    } else {
        // Fill with actaul code...
        return "";
    }
}

LSRValue::LSRValue(const LSRValue &v) {
    intVal = v.intVal;
    strVal = v.strVal;
    className = v.className;
    type = v.type;
}
LSRValue::LSRValue(long long iv) {
    intVal = iv;
    type = 0; // 0 is int.
}

LSRValue::LSRValue(std::string st) {
    strVal = st;
    type = 1; // 1 i str.
}

long long LSRValue::getIntVal() const {
    return intVal;
}

std::string LSRValue::getStrVal() const {
    return strVal;
}

int LSRValue::isInt() const {
    return type == 0;
}
int LSRValue::isStr()  const{
    return type == 1;
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

void Scope::decl(std::string id, std::string type) {
    if (st.contains(id)) {
        std::cout << "RepeatIDError: ID " << id << " has already been declared in this scope." << std::endl;
    } else {
        if (!type.compare("int")) {
            LSRValue v = LSRValue(0);
            st.add(id, v);
        } else if (!type.compare("str")) {
            LSRValue v = LSRValue("\"\"");
            st.add(id,v);
        }
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

LSRExpr::LSRExpr(const LSRExpr &expr) {
    val = expr.val;
}

LSRExpr LSRExpr::operator+(const LSRExpr &rhs) {
    LSRValue retval;
    if (this->val.isInt()) {
        if (rhs.val.isStr()) {
            std::cout << "Trying to add string and int, ignored" << std::endl;
        } else if (rhs.val.isInt()) {
            retval = LSRValue(this->val.intVal + rhs.val.intVal);
        }
    } else if (this->val.isStr()) {
        if (rhs.val.isInt()) {
            std::cout << "Trying to add string and int, ignored" << std::endl;
        } else if (rhs.val.isStr()) {
            std::string retstr = this->val.toString() + rhs.val.toString();
            retval = LSRValue(retstr);
        }
    }
    LSRExpr ret = LSRExpr(retval);
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

LSRStr::LSRStr(std::string st) {
    std::string strVal = st;
    strVal.erase(0,1);
    strVal.erase(strVal.size()-1);
    val = LSRValue(strVal);
}

