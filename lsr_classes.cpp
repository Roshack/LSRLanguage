#include "lsr_classes.hpp"



void cleanStackSim(stackSim * x) {
    while (x->next != NULL) {
        stackSim * temp = x->next->next;
        delete x->next;
        x->next = temp;
    }
}

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
    objPtr = v.objPtr;
}
LSRValue::LSRValue(ggc_size_t iv) {
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

ggc_size_t LSRValue::getIntVal() const {
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

void * LSRValue::getObjectPointer() {
    return objPtr;
}

void * LSRValue::createInMemory(void *classD) {
    LSRClassTable * classDefs = (LSRClassTable *) classD;
    if (!classDefs->contains(className)) {
        return NULL;
    }
    void * desc = classDefs->getDescriptorPointer(className);
    objPtr = NULL;
    GGC_PUSH_1(objPtr);    
    objPtr = ggggc_malloc((struct GGGGC_Descriptor *) desc);
    return objPtr;
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

int LSRValue::operator<(const LSRValue &rhs) {
    // TODO: add maybe for non int vals
    if (this->isInt() && rhs.isInt()) {
        if (this->intVal < rhs.getIntVal()) {
            return 1;
        } else {
            return 0;
        }
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
    if (parent) {
        ptrScope = parent->ptrScope;
    }
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
        st.add(id,v);
        if(v.isClass()) {
            void * x = st.createInMemory(id,classDefs);
            (*ptrScope)->addPtr(id,x);
        }
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
        LSRValue v = st.get(id);
        if (v.isClass()) {
            v.objPtr = (*ptrScope)->resolve(id);
        }
        return v;
    } else if (!isTopLevel()) {
        return parent->resolve(id);
    } else {
        std::cout << "UndefinedIDError: ID " << id << " has not been declared in this scope." << std::endl;
        return LSRValue(0);
    }
}

LSRValue Scope::resolveMembers(void * ma, void * ct) {
    LSRMemberAccess * access = (LSRMemberAccess*) ma;
    if (st.contains(access->getParent())) {
        LSRValue val = st.get(access->getParent());
        if (!val.isClass()) {
            std::cout <<"Trying to member access variable: " << access->getParent() << " that is of type " << val.getType() << std::endl;
            return LSRValue (0);
        } else {
            LSRClassTable * classTable = (LSRClassTable*) ct;
            std::string className = val.getType();
            std::string memberType = classTable->getType(className,access->getChild());
            long unsigned int offSet = classTable->getOffset(className,access->getChild());
            ggc_size_t * objPointer = (ggc_size_t*) (*ptrScope)->resolve(access->getParent());
            if (!memberType.compare("int")) {
                
                return LSRValue(objPointer[offSet]);
            }
            //TODO: stuff other than ints
        }
    } else if (!isTopLevel()) {
        return parent->resolveMembers(ma,ct);
    } else {
        std::cout << "UndefinedIDError: ID " << access->getParent() << " has not been declared in this scope." << std::endl;
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
    ggc_size_t size = 1; 
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

void * SymbolTable::createInMemory(std::string id,void * classDefs) {
    LSRValue v = symMap[id];
    symMap[id].objPtr = v.createInMemory(classDefs);
    return symMap[id].objPtr;
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

LSRInt::LSRInt(ggc_size_t value) {
    val = LSRValue(value);
}

LSRStr::LSRStr(std::string st) {
    std::string strVal = st;
    strVal.erase(0,1);
    strVal.erase(strVal.size()-1);
    val = LSRValue(strVal, strVal.size());
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


int nodeValue::operator<(const nodeValue &rhs) {
    // TODO: add for more than ints maybe?
    if (this->intVal < rhs.intVal) {
        return 1;
    } else {
        return 0;
    }
}

nodeValue nodeValue::operator+(const nodeValue &rhs) {
    //TODO: add for more than intsm atybye?
    return nodeValue(this->intVal + rhs.intVal);
}

LSRValue nodeValue::toVal(Scope * scope, LSRClassTable * classDefs) {
    if (type==NVINT) {
        return LSRValue(intVal);
    } else if (type==NVSTR) {
        return (LSRValue(st,strLen));
    } else if (type==NVVAR) {
        return scope->resolve(varName);
    } else if (type==NVMEMB) {
        LSRMemberAccess m = LSRMemberAccess(varName,memberName);
        return scope->resolveMembers((void *) (&m), (void *) classDefs);
    } else {
        std::cout <<"type not defined for nodevalue" << std::endl;
        std::cout <<"type is " << type << std::endl;
        return LSRValue(0);
    }
}

nodeValue binaryExprNode::execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions){
    nodeValue ret = nodeValue();
    switch(op) {
    //TODO: implement other operators
    case PLUS_OP:
        ret = lhs.execute(scope,classDefs,functions) + rhs.execute(scope,classDefs,functions);
        ret.type = NVINT;
        break;
    }
    return ret;
}



nodeValue condNode::execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions){
    nodeValue ret = nodeValue();    
    nodeValue l = lhs.execute(scope,classDefs,functions);
    nodeValue r = rhs.execute(scope,classDefs,functions);
    switch (op) {
    //TODO: implement other operators.
    case LT_OP:
        if (l < r) {
            ret.intVal = 1;
        } else {
            ret.intVal = 0;
        }
        break;
    }
    return ret;
}


nodeValue varNode::execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions){  
    LSRValue v = scope->resolve(varName);
    nodeValue ret = nodeValue();
    // TODO: add for not just ints.
    if (!v.getType().compare("int")) {
        ret = nodeValue(v.intVal);
        ret.type = NVINT;
    } else if (!v.getType().compare("str")) {
        ret.st = v.getStrVal();
        ret.strLen = 0;
        ret.type = NVSTR;
    } else {
        ret.objPtr = v.getObjectPointer();
        ret.varName = varName;
        ret.type = NVVAR;
    }
    ret.varName = varName;
    return ret;

}

nodeValue memberNode::execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions){
    LSRMemberAccess *  ma = new LSRMemberAccess(varName,memberName);    
    LSRValue v = scope->resolveMembers((void*) ma, (void *) classDefs);
    nodeValue ret = nodeValue();
    if (!v.getType().compare("int")) {
        ret = nodeValue(v.intVal);
        ret.type = NVINT;
    } else if (!v.getType().compare("str")) {
        ret.st = v.getStrVal();
        ret.strLen = 0;
        ret.type = NVSTR;
    }
    ret.varName = varName;
    ret.memberName = memberName;
    delete ma;
    return ret;
}

nodeValue assignNode::execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions){
    nodeValue l = lhs.execute(scope, classDefs,functions);
    nodeValue r = rhs.execute(scope,classDefs,functions);  
    if (lhs.isLValue()) {
        if (lhs.isMember()) {         
            scope->memberAssign(l.varName,l.memberName,r.toVal(scope,classDefs),(void *) classDefs);
        } else {
            scope->assign(l.varName,r.toVal(scope,classDefs),(void *) classDefs);
        }
    } else {
        return nodeValue(0);
    }
    return nodeValue(0);
}

nodeValue printNode::execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions){
    nodeValue temp = expr.execute(scope,classDefs,functions);
    LSRValue val = temp.toVal(scope,classDefs);  
    std::cout << val.toString();
    if(printLine) {
        std::cout << std::endl;
    }
    return nodeValue(0);
}

nodeValue declNode::execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions){
    nodeValue ret = nodeValue(0);
    scope->decl(varName, type, (void *) classDefs);
    ret.varName = varName;
    ret.type = NVVAR;
    return ret;
}

nodeValue intNode::execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions) {
    nodeValue v = nodeValue(intVal);
    v.type=NVINT;
    return v;
}

strNode::strNode(std::string &s) {
    LSRStr st = LSRStr(s);
    strVal = st.getString();
}

nodeValue strNode::execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions) {
    nodeValue v = nodeValue(0);
    v.type=NVSTR;
    v.st = strVal;
    v.strLen = 0;
    return v;
}

nodeValue whileNode::execute(Scope * scope, LSRClassTable * classDefs, LSRFunctionTable * functions){
    // A cond node when executed returns an LSRValue with an intval of 1 if true, 0 if false.;
    while (cond.execute(scope,classDefs,functions).intVal) {
        scope = new Scope(scope);
        *(scope->ptrScope) = new LSRScope(*(scope->ptrScope));
        StmtList::iterator it = stmts.begin();
        while(it != stmts.end()) {
            (*it)->execute(scope,classDefs,functions);
            it++;
        }
        Scope *temp = scope;
        LSRScope * t = (*scope->ptrScope);
        scope = scope->getParent();
        *(scope->ptrScope) = (*(scope->ptrScope))->getParent();
        delete t;
        delete temp;
    }
    return nodeValue(0);
}

whileNode::~whileNode() {
    while (!stmts.empty()) {
        stmtNode *p = stmts.back();
        stmts.pop_back();
        delete p;
    }
}




