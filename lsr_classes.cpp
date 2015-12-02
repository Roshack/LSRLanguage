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
        return "Objtype: " + className;
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

LSRValue::LSRValue(std::string st, int size) {
    strVal = st;
    type = 1; // 1 i str.
}

// Literally the only reason for int marker is to discern this function from
// the string one.
LSRValue::LSRValue(std::string cName) {
    className = cName;
    type = 2;
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
int LSRValue::isClass() const {
    return type ==2;
}

void LSRValue::createInMemory(void *classD) {
    LSRClassTable * classDefs = (LSRClassTable *) classD;
    if (!classDefs->contains(className)) {
        std::cout << "Trying to malloc non-existant class " << className << " This should never happen" <<std::endl;
        return;
    }
    void * desc = classDefs->getDescriptorPointer(className);
    objPtr = NULL;
    std::cout <<"Gonna try and do some pushing..." <<std::endl;
    GGC_PUSH_1(objPtr);    
    std::cout <<" gonna do some mallocing with desc pointer " << desc << std::endl;
    
    objPtr = ggggc_malloc((struct GGGGC_Descriptor *) desc);
    std::cout <<"Done that mallocing" << std::endl;
}

std::string LSRValue::getType() {
    if (isInt()) {
        return "int";
    } else if (isStr()) {
        return "str";
    } else {
        return className;
    }
}


LSRValue getInitializedVal(std::string type) {
    LSRValue v = LSRValue(0);
    if (!type.compare("int")) {
        v = LSRValue(0);
    } else if (!type.compare("str")) {
        v = LSRValue("\"\"", 0);
    } else {
        v = LSRValue(type);
    }

    return v;
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

void Scope::decl(std::string id, std::string type,void *classDefs) {
    if (st.contains(id)) {
        std::cout << "RepeatIDError: ID " << id << " has already been declared in this scope." << std::endl;
    } else {
        LSRValue v = getInitializedVal(type);
        std::cout << "gonna try and create in memory" << std::endl;
        if(v.isClass()) {
            v.createInMemory(classDefs);
        }
        std::cout << "finished creating in memory" << std::endl;
        st.add(id,v);
        
    }
}

void Scope::assign(std::string id, LSRValue val,void *classDefs) {
    if (st.contains(id)) {
        st.set(id,val);
    } else if (!isTopLevel()) {
        parent->assign(id,val,classDefs);
    } else {
        std::cout << "UndefinedIDError: ID " << id << " has not been declared in this scope." << std::endl;
    }
}

void Scope::memberAssign(std::string parent, std::string child, LSRValue val,void *classDefs) {
    LSRValue v = resolve(parent);
    LSRClassTable * classes = (LSRClassTable *) classDefs;
    if (!classes->contains(v.className)) {
        std::cout << "Class : " << parent << " not yet defined in member assign" << std::endl;
        return;
    }
    long unsigned int offSet = classes->getOffset(v.className,child);
    std::string childType = classes->getType(v.className,child);
    if (offSet) {
        if (childType != val.getType()) {
            std::cout << "Trying to assign invalid types in member assign" << std::endl;
        }
        ggc_size_t * objectPointer = (ggc_size_t *)v.objPtr;
        objectPointer = objectPointer + offSet;
        if (val.isInt()) {
            (*objectPointer) = (ggc_size_t) val.getIntVal();
        }
        // TODO: Add string and object setting
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

void SymbolTable::setDescriptorPointer() {
    ggc_size_t size = 0; 
    ggc_size_t pointers = 0;
    std::map<std::string, LSRValue>::iterator iter = symMap.begin();
    while (iter != symMap.end()) {
        size++;
        if (!iter->second.isInt()) {
            // Anything but an int is a pointer in this language !!!!!!!
            pointers = pointers | 1<<size;
        }
        iter++;
    }
    if (pointers) {
        // If pointers is no longer 0 we set at least one pointer,
        // need to set the lowest order bit to 1 to denote that we have pointers.
        pointers = pointers | 1;
    }
    descriptorPointer = NULL;
    GGC_PUSH_1(descriptorPointer);
    GGC_GLOBALIZE();
    descriptorPointer = (void *) ggggc_allocateDescriptor(size,pointers);
}

long unsigned int SymbolTable::getIndex(std::string member) {
    int size = 0;
    std::map<std::string, LSRValue>::iterator iter = symMap.begin();
    while(iter != symMap.end()) {
        size++;
        if (!iter->first.compare(member)) {
            return size;
        }   
        iter++;
    }
    return size;
}

std::string SymbolTable::getMemberType(std::string member) {
    std::map<std::string, LSRValue>::iterator search = symMap.find(member);
    if (search != symMap.end()) {
        return search->second.getType();
    } else {
        std::cout << "Trying to get member type of member that doesn't exist" << std::endl;
    }
}

void * SymbolTable::getDescriptorPointer() {
    return descriptorPointer;
}

std::string LSRIdent::getName() {
    return name;
}

LSRMemberAccess::LSRMemberAccess(const std::string& p, const std::string& c) {
    parent = p;
    child = c;
}

std::string LSRMemberAccess::getParent() {
    return parent;
}
std::string LSRMemberAccess::getChild() {
    return child;
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

int LSRClassTable::contains(std::string className) {
    std::map<std::string, SymbolTable>::iterator search = classDefs.find(className);
    return search!=classDefs.end();
}

void LSRClassTable::add(std::string className) {
    if (contains(className)) {
        std::cout << "Classname " << className << " is already defined!" << std::endl;
        return;
    } else {
        SymbolTable temp = SymbolTable();
        classDefs[className] = temp;
        return;
    }
}

void LSRClassTable::addVar(std::string className,std::string varName, std::string type) {
    if (!contains(className)) {
        std::cout << "Trying to add a variable to non-existant class " << className << " This should never happen" <<std::endl;
    } else {
        LSRValue v = getInitializedVal(type);
        classDefs[className].add(varName,v);
    }
}

void LSRClassTable::setDescriptorPointer(std::string className) {
    if (!contains(className)) {
        std::cout << "Trying to set desc pointer for non-existant class " << className << " This should never happen" <<std::endl;
    } else {
        classDefs[className].setDescriptorPointer();
    }
}

void* LSRClassTable::getDescriptorPointer(std::string className) {
    if (!contains(className)) {
        std::cout << "Trying to get desc pointer for non-existant class " << className << " This should never happen" <<std::endl;
        return NULL;
    } else {
        return classDefs[className].getDescriptorPointer();
    }
}
long unsigned int LSRClassTable::getOffset(std::string classname, std::string membername) {
    long unsigned int ret = 0;
    std::map<std::string, SymbolTable>::iterator search = classDefs.find(classname);
    if (search != classDefs.end()) {
        ret = search->second.getIndex(membername);
    }
    return ret;
}

std::string LSRClassTable::getType(std::string classname, std::string member) {
    std::string ret = "UNKNOWN";
    std::map<std::string, SymbolTable>::iterator search = classDefs.find(classname);
    if (search != classDefs.end()) {
        ret = search->second.getMemberType(member);
    }
    return ret;
}

