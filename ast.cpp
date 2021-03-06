#include "ast.h"
#include <iostream>
#include <sstream>
#include <set>
#include "asm.h"

int globalStackPointer = 0;
int arraySize = 0;
extern Asm assemblyFile;

class VariableInfo{
    public:
        VariableInfo(int offset, bool isArray, bool isParameter, Type type){
            this->offset = offset;
            this->isArray = isArray;
            this->isParameter = isParameter;
            this->type = type;
        }
        int offset;
        bool isArray;
        bool isParameter;
        Type type;
};

map<string, VariableInfo *> codeGenerationVars;

const char * intTemps[] = {"$t0","$t1", "$t2","$t3","$t4","$t5","$t6","$t7","$t8","$t9" };
const char * floatTemps[] = {"$f0",
                            "$f1",
                            "$f2",
                            "$f3",
                            "$f4",
                            "$f5",
                            "$f6",
                            "$f7",
                            "$f8",
                            "$f9",
                            "$f10",
                            "$f11",
                            "$f12",
                            "$f13",
                            "$f14",
                            "$f15",
                            "$f16",
                            "$f17",
                            "$f18",
                            "$f19",
                            "$f20",
                            "$f21",
                            "$f22",
                            "$f23",
                            "$f24",
                            "$f25",
                            "$f26",
                            "$f27",
                            "$f28",
                            "$f29",
                            "$f30",
                            "$f31"
                        };

#define INT_TEMP_COUNT 10
#define FLOAT_TEMP_COUNT 32
set<string> intTempMap;
set<string> floatTempMap;


string getIntTemp(){
    for (int i = 0; i < INT_TEMP_COUNT; i++)
    {
        if(intTempMap.find(intTemps[i]) == intTempMap.end()){
            intTempMap.insert(intTemps[i]);
            return string(intTemps[i]);
        }
    }
    cout<<"No more int registers!"<<endl;
    //exit(1);
    return "";
}

string getFloatTemp(){
    for (int i = 0; i < FLOAT_TEMP_COUNT; i++)
    {
        if(floatTempMap.find(floatTemps[i]) == floatTempMap.end()){
            floatTempMap.insert(floatTemps[i]);
            return string(floatTemps[i]);
        }
    }
    cout<<"No more float registers!"<<endl;
    //exit(1);
    return "";
}

void releaseIntTemp(string temp){
    intTempMap.erase(temp);
}

void releaseFloatTemp(string temp){
    floatTempMap.erase(temp);
}

void releaseRegister(string temp){
    releaseIntTemp(temp);
    releaseFloatTemp(temp);
}

int labelCounter = 0;

string getNewLabel(string prefix){
    stringstream ss;
    ss<<prefix << labelCounter;
    labelCounter++;
    return ss.str();
}

string saveState(){
    set<string>::iterator it = floatTempMap.begin();
    stringstream ss;
    ss<<"sw $ra, " <<globalStackPointer<< "($sp)\n";
    globalStackPointer+=4;
    return ss.str();
}

string retrieveState(string state){
    std::string::size_type n = 0;
    string s = "sw";
    while ( ( n = state.find( s, n ) ) != std::string::npos )
    {
    state.replace( n, s.size(), "lw" );
    n += 2;
    globalStackPointer-=4;
    }
    return state;
}


class ContextStack{
    public:
        struct ContextStack* prev;
        map<string, Type> variables;
};

class FunctionInfo{
    public:
        Type returnType;
        bool hasMultipleReturnTypes;
        list<Type> returnTypeList;
        list<Parameter *> parameters;
};

map<string, Type> globalVariables = {};
map<string, Type> variables;
map<string, FunctionInfo*> methods;

map<string, Type> resultTypes ={
    {"INT,INT", INT},
    {"FLOAT32,FLOAT32", FLOAT32},
    {"INT,FLOAT32", FLOAT32},
    {"FLOAT32,INT", FLOAT32},
    {"INT_ARRAY,INT_ARRAY", INT},
    {"FLOAT_ARRAY,FLOAT_ARRAY", FLOAT32},
    {"FLOAT_ARRAY,INT_ARRAY", FLOAT32},
    {"INT_ARRAY,FLOAT_ARRAY", FLOAT32},
    {"INT_ARRAY,INT", INT},
    {"INT_ARRAY,FLOAT32", FLOAT32},
    {"INT,FLOAT_ARRAY", FLOAT32},
    {"INT,INT_ARRAY", INT},
    {"FLOAT_ARRAY,FLOAT32", FLOAT32},
    {"FLOAT32,FLOAT_ARRAY",FLOAT32},
    {"FLOAT32,INT_ARRAY", FLOAT32}
};

map<string, Type> booleanResultTypes ={
    {"INT,INT", BOOL},
    {"FLOAT32,FLOAT32", BOOL},
    {"INT,FLOAT32", BOOL},
    {"FLOAT32,INT", BOOL},
    {"INT_ARRAY,INT_ARRAY", BOOL},
    {"FLOAT_ARRAY,FLOAT_ARRAY", BOOL},
    {"FLOAT_ARRAY,INT_ARRAY", BOOL},
    {"INT_ARRAY,FLOAT_ARRAY", BOOL},
    {"INT_ARRAY,INT", BOOL},
    {"INT_ARRAY,FLOAT32", BOOL},
    {"INT,FLOAT_ARRAY", BOOL},
    {"INT,INT_ARRAY", BOOL},
    {"FLOAT_ARRAY,FLOAT32", BOOL},
    {"FLOAT32,FLOAT_ARRAY",BOOL},
    {"FLOAT32,INT_ARRAY", BOOL},
    {"BOOL,BOOL", BOOL},
};


string getTypeName(Type type){
    switch(type){
        case INT:
            return "INT";
        case FLOAT32:
            return "FLOAT32";
        case VOID:
            return "VOID";
        case INT_ARRAY:
            return "INT";
        case FLOAT_ARRAY:
            return "FLOAT32";
        case BOOL:
            return "BOOL";
    }

    cout<<"Unknown type"<<endl;
    exit(0);
}

ContextStack * context = NULL;

void pushContext(){
    variables.clear();
    ContextStack * c = new ContextStack();
    c->variables = variables;
    c->prev = context;
    context = c;
}

void popContext(){
    if(context != NULL){
        ContextStack * previous = context->prev;
        free(context);
        context = previous;
    }
}

Type getLocalVariableType(string id){
    ContextStack * currContext = context;
    while(currContext != NULL){
        if(currContext->variables[id] != 0){
            return currContext->variables[id];
        }
        currContext = currContext->prev;
    }

    if(!context->variables.empty())
        return context->variables[id];
    return INVALID;
}


Type getVariableType(string id){
    if(!globalVariables.empty())
        return globalVariables[id];
    return INVALID;
}

Type variableExists(string id){
  Type value;
  if(context != NULL){
    value = getLocalVariableType(id);
    //context->variables[id] != 0
    if(value != 0)
      return value;
  }
  if( globalVariables[id] != 0 ){
      return globalVariables[id];
  }
  return INVALID;
}

void AuxVarDeclaration(DeclaratorList declaratorsList, InitializerList initializerList, Type type, int line, bool isGlobalDeclaration){
    list<Declarator *>::iterator declaratorIt = declaratorsList.begin();
    list<Initializer *>::iterator initializerIt = initializerList.begin();

    if( !initializerList.empty()){
        if( declaratorsList.size() != initializerList.size()){
            cout<<"Error: Initializers list must match declarators list. Line "<<line<<endl;
            exit(0);
        }

        if(type != NULLTYPE){
            while(initializerIt != initializerList.end() ){
                Initializer * init = (*initializerIt);
                if(init->initializer->getType() != type){
                    cout<<"Error in line "<<line<<". Value of type "<<getTypeName(init->initializer->getType())<<" cannot be assigned to "<<getTypeName(type)<<" type."<<endl;
                    exit(0);
                }
                initializerIt++;
            }
        }
    }
    int declaratorItIndex = 0;
    while(declaratorIt != declaratorsList.end()){
        Declarator * declarator = (*declaratorIt);
        if(declarator->isArray){
            if(declarator->arraySize == NULL ){
                cout<<"Error: storage size of: "<<declarator->id<< " array is unknown. Line: "<<line<<endl;\
                exit(0);
            }

            if(declarator->arraySize->getType() != INT){
                cout<<"Error: storage size of '"<<declarator->id<<"' must be an int value"<< ". Line: "<<line<<endl;
                exit(0);
            }
        }

       if(!variableExists(declarator->id)){
            if(type == NULLTYPE){
                //Esto es para asignacion dinamica de tipo
                list<Initializer *>::iterator initializerIt2 = initializerList.begin();
                advance(initializerIt2, declaratorItIndex);
                Initializer * initializer  = (*initializerIt2);

                if(isGlobalDeclaration){
                    globalVariables[declarator->id] = initializer->initializer->getType();
                }else{
                    context->variables[declarator->id] = initializer->initializer->getType();
                }
                
            }else{
                if(isGlobalDeclaration){
                    globalVariables[declarator->id] = type;
                }else{
                    context->variables[declarator->id] = type;
                }
            }
        }else{
            cout<<"Error: redefinition of variable '"<< declarator->id<< "'. Line: "<<line <<endl;
            exit(0);
        }
       
        declaratorIt++;
        declaratorItIndex++;
    }
}


int VarDeclaration::evaluateSemantic(){
    AuxVarDeclaration(this->declaratorsList, this->initializerList, this->type, this->line, false);
    return 1;
}

string VarDeclaration::genCode(){
    stringstream code;
    list<Declarator *>::iterator it = this->declaratorsList.begin();

    int declaratorItIndex = 0;
    while( it != this->declaratorsList.end()){
        Declarator * declarator = (*it);

        list<Initializer *>::iterator initializerIt = initializerList.begin();
        advance(initializerIt, declaratorItIndex);
        Initializer * initializer  = (*initializerIt);

        if(!declarator->isArray){
             codeGenerationVars[declarator->id] = new VariableInfo(globalStackPointer, false, false, this->type);
             globalStackPointer +=4;
        }else{
            
            codeGenerationVars[declarator->id] = new VariableInfo(globalStackPointer, true, false, this->type);
            // if(initializer->initializer == NULL){
                if(declarator->arraySize != NULL){
                    
                    int size = ((IntExpression *)declarator->arraySize)->value;
                    globalStackPointer += (size * 4);
                }
            // }
        }
        if(initializer != NULL){
            int offset = codeGenerationVars[declarator->id]->offset;
            Code exprCode;
            initializer->initializer->genCode(exprCode);
            code << exprCode.code <<endl;
            if(exprCode.type == INT){
                code << "sw " << exprCode.place <<", "<< offset << "($sp)"<<endl;
            }else if(exprCode.type == FLOAT32){
                code << "s.s " << exprCode.place <<", "<< offset << "($sp)"<<endl;
            }
            releaseRegister(exprCode.place);
            // if (declarator->isArray)
            // {
            //     globalStackPointer+=4;
            //     offset += 4;
            // }
            
        }
        it++;
    }

    return code.str();
}

int GlobalDeclaration::evaluateSemantic(){
    VarDeclaration * varDeclaration = this->declarationStatement;
    AuxVarDeclaration(varDeclaration->declaratorsList, varDeclaration->initializerList, varDeclaration->type, varDeclaration->line, true);
    return 2;
}

string GlobalDeclaration::genCode(){

    return "";
}

int BlockStatement::evaluateSemantic(){
    
    list<Statement *>::iterator itd = this->statements.begin();
    while( itd != this->statements.end()){
        Statement * statement = *itd;
        if( statement!= NULL){
            statement->evaluateSemantic();
        }

        itd++;
    }
    return 3;
}

string BlockStatement::genCode(){
    stringstream ss;

    list<Statement *>::iterator its = this->statements.begin();
    while (its != this->statements.end())
    {
        Statement * stmt = *its;
        if(stmt != NULL){
            ss<<stmt->genCode()<<endl;
        }

        its++;
    }

    return ss.str();
}

int PrintStatement::evaluateSemantic(){
    list<Expression *>::iterator argsIt = this->args.begin();

    while(argsIt != this->args.end()){
        (*argsIt)->getType();
        argsIt++;
    }

    return 0;
}



string PrintStatement::genCode(){
    list<Expression *>::iterator it = this->args.begin();
    list<Code> codes;
    stringstream ss;
    Code argCode;
    while (it != this->args.end())
    {
        (*it)->genCode(argCode);
        ss << argCode.code <<endl;
        codes.push_back(argCode);
        it++;
    }

    int codesIndex = 0;
    list<Code>::iterator placesIt = codes.begin();
    list<Expression *>::iterator argsIt = this->args.begin();

     while (placesIt != codes.end()){

        releaseRegister((*placesIt).place);
        if((*placesIt).type == INT){
            advance(argsIt, codesIndex);
            if(codeGenerationVars.find(((IdExpression*)(*argsIt))->value) != codeGenerationVars.end()){
                // string intTemp = getIntTemp();
                IdExpression * id = (IdExpression*)(*argsIt);
                if( codeGenerationVars[id->value]->isArray){
                    int c = 0;
                    int offset = 0;
                    while( c < arraySize ){
                        ss << "lw "<<"$t9"<<", "<<offset<<"("<<(*placesIt).place<<")"<<endl
                        <<"move $a0, "<< "$t9"<<endl
                        << "li $v0, 1"<<endl
                        << "syscall"<<endl;

                        c++;
                        offset = offset + 4;
                    }
                }else{
                    ss <<"move $a0, "<< (*placesIt).place<<endl
                    << "li $v0, 1"<<endl
                    << "syscall"<<endl;
                }
                // releaseRegister(intTemp);
            }else{
                ss <<"move $a0, "<< (*placesIt).place<<endl
                << "li $v0, 1"<<endl
                << "syscall"<<endl;
            }
            
        }else if((*placesIt).type == FLOAT32){
            ss << "mov.s $f12, "<< (*placesIt).place<<endl
            << "li $v0, 2"<<endl
            << "syscall"<<endl;
        }else if((*placesIt).type == STRING){
            ss << "la $a0, "<< (*placesIt).place<<endl
            << "li $v0, 4"<<endl
            << "syscall"<<endl;
        }else if((*placesIt).type == BOOL){
            ss << "move $a0, "<<(*placesIt).place<<endl
            << "li $v0, 1"<<endl
            << "syscall"<<endl;
        }

        codesIndex++;
        placesIt++;
    }

   
    return ss.str();
}

int Parameter::evaluateSemantic(){
    if(!variableExists(this->id)){
        context->variables[this->id] = this->type;
    }else{
        cout<<"Error in line: Redefinition of variable '"<<this->id<<"'. Line "<<this->line<<endl;
        exit(0);
    }
    return 0;
}

void addFunctionDeclaration(string id, ParametersList params, TypeList returnTypeList, Type returnType, bool hasMultipleReturnTypes, int line){
    if(methods[id] != 0){
        cout<<"Error in line "<<line<<": Redefinition of function '"<<id<<"'."<<endl;
        exit(0);
    }
    methods[id] = new FunctionInfo();
    methods[id]->returnType = returnType;
    methods[id]->parameters = params;
    methods[id]->returnTypeList = returnTypeList;
    if( hasMultipleReturnTypes ){
        methods[id]->hasMultipleReturnTypes = true;
    }else{
        methods[id]->hasMultipleReturnTypes = false;
    }
}

int FunctionDefinition::evaluateSemantic(){
    if(this->params.size() > 4){
        cout<<"Error in line "<<this->line<<": Function '"<<this->id<<"' can't have more than 4 paramaters."<<endl;
        exit(0);
    }
    if(this->typeList.empty()){
        addFunctionDeclaration(this->id, this->params, this->typeList, this->type, false, this->line);
    }else{
        addFunctionDeclaration(this->id, this->params, this->typeList, this->type, true, this->line);
    }

    pushContext();

    list<Parameter* >::iterator it = this->params.begin();
    while(it != this->params.end()){
        (*it)->evaluateSemantic();
        it++;
    }

    if(this->statement != NULL){
        this->statement->evaluateSemantic();
    }

    popContext();

    return 4;
    
}

string FunctionDefinition::genCode(){

    if(this->statement == NULL)
        return "";

    int stackPointer = 4;
    globalStackPointer = 0;
    stringstream code;
    code << this->id<<": "<<endl;
    string state = saveState();
    code <<state<<endl;
    if(this->params.size() > 0){
        list<Parameter *>::iterator it = this->params.begin();
        for(int i = 0; i< this->params.size(); i++){
            code << "sw $a"<<i<<", "<< stackPointer<<"($sp)"<<endl;
            codeGenerationVars[(*it)->id] = new VariableInfo(stackPointer, false, true, (*it)->type);
            stackPointer +=4;
            globalStackPointer +=4;
            it++;
        }
    }

    code<< this->statement->genCode()<<endl;
    stringstream sp;
    int currentStackPointer = globalStackPointer;
    sp << endl<<"addiu $sp, $sp, -"<<currentStackPointer<<endl;
    code << retrieveState(state);
    code << "addiu $sp, $sp, "<<currentStackPointer<<endl;
    code <<"jr $ra"<<endl;
    codeGenerationVars.clear();
    string result = code.str();
    result.insert(id.size() + 2, sp.str());
    return result;
}

int IfStatement::evaluateSemantic(){
    
    if( this->initExpression != NULL ){
        this->initExpression->getType();
    }

    if(this->conditionalExpression->getType() != BOOL){
        cout<<"Error in line "<<this->line<<": Expression for if conditional statement must be boolean"<<endl;
    }
    pushContext();
    this->trueStatement->evaluateSemantic();
    popContext();
    return 0;
}

string IfStatement::genCode(){
    string endIfLabel = getNewLabel("endif");
    Code exprCode;
    this->conditionalExpression->genCode(exprCode);
    stringstream code;
    code << exprCode.code << endl;
    if(exprCode.type == INT){
        code<< "beqz "<< exprCode.place << ", " << endIfLabel <<endl;
    }else{
        code << "bc1f "<< endIfLabel <<endl;
    }
    code<< this->trueStatement->genCode() << endl
    << endIfLabel <<" :"<< endl;
    releaseRegister(exprCode.place);
    
    return code.str();
}

int ElseStatement::evaluateSemantic(){

    if( this->initExpression != NULL ){
        this->initExpression->getType();
    }

    if(this->conditionalExpression->getType() != BOOL){
        cout<<"Error in line "<<this->line<<": Expression for if conditional statement must be boolean"<<endl;
    }
    pushContext();
    this->trueStatement->evaluateSemantic();
    popContext();
    if(this->falseStatement != NULL)
        this->falseStatement->evaluateSemantic();
    popContext();

    return 0;
}

string ElseStatement::genCode(){
    string elseLabel = getNewLabel("else");
    string endIfLabel = getNewLabel("endif");
    Code exprCode;
    this->conditionalExpression->genCode(exprCode);
    stringstream code;
    code << exprCode.code << endl;
    if(exprCode.type == INT){
        code<< "beqz "<< exprCode.place << ", " << elseLabel <<endl;
    }else{
        code << "bc1f "<< elseLabel <<endl;
    }
    code << this->trueStatement->genCode() << endl
    << "j " <<endIfLabel << endl
    << elseLabel <<": " <<endl
    << this->falseStatement->genCode() <<endl
    << endIfLabel <<" :"<< endl;
    releaseRegister(exprCode.place);
    return code.str();
}

int ForStatement::evaluateSemantic(){

    if(this->initExpr != NULL){
        this->initExpr->getType();
    }

    if(this->conditionalExpr != NULL){
        if(this->conditionalExpr->getType() != BOOL){
            cout<<"Error in line "<<this->line<<": Conditional expression in for statement must be boolean."<<endl;
        } 
    }

    if(this->postExpr != NULL){
        this->postExpr->getType();
    }

    pushContext();
    if(this->statement != NULL){
        this->statement->evaluateSemantic();
    }
    popContext();
    return 0;
}


string ForStatement::genCode(){
    Code initExprCode;
    Code conditionalExprCode;
    Code postExprCode;
    string forLabel = getNewLabel("for");
    string endForLabel = getNewLabel("endFor");
    stringstream ss;

    if(this->initExpr != NULL){
        this->initExpr->genCode(initExprCode);
        ss<<initExprCode.code<<endl;
        releaseRegister(initExprCode.place);
    }

    ss<<forLabel<<": "<<endl;
    
    if(this->conditionalExpr != NULL){
        this->conditionalExpr->genCode(conditionalExprCode);
        ss<<conditionalExprCode.code<<endl;
        releaseRegister(conditionalExprCode.place);
        if(conditionalExprCode.type == INT){
            ss<<"beqz "<<conditionalExprCode.place<<", "<<endForLabel<<endl;
        }else if(conditionalExprCode.type == FLOAT32){
            ss<<"bc1f "<<endForLabel<<endl;
        }   

    }

    ss<<this->statement->genCode()<<endl;

    if( this->postExpr != NULL){
        this->postExpr->genCode(postExprCode);
        ss<<postExprCode.code<<endl;
    }

    ss<<"j "<< forLabel<<endl
    <<endForLabel<<": "<<endl;

    return ss.str();
}

int ReturnStatement::evaluateSemantic(){
    list<Expression *>::iterator iterator = this->expressionList.begin();

    while(iterator != this->expressionList.end()){
        (*iterator)->getType();
        iterator++;
    }
    return 0;
}

string ReturnStatement::genCode(){
    list<Expression *>::iterator expressionIt = this->expressionList.begin();
    Code exprCode;
    stringstream ss;
    list<Code> codes;

    while( expressionIt != this->expressionList.end()){
        (*expressionIt)->genCode(exprCode);
        ss << exprCode.code <<endl;
        codes.push_back(exprCode);
        
        expressionIt++;
    }
    
    int i = 0;
    list<Code>::iterator placesIt = codes.begin();
    while (placesIt != codes.end())
    {
        releaseRegister((*placesIt).place);
        if(exprCode.type == INT)
            ss<< "move $v"<<i<<", " << (*placesIt).place <<endl;
        else
            ss<< "mfc1 $v"<<i<<", "<<(*placesIt).place<<endl;
        i++;
        placesIt++;
    }

    
    return ss.str();
}

int JumpStatement::evaluateSemantic(){
    //nada
    return 0;
}

string JumpStatement::genCode(){
    stringstream ss;
    ss<< "li $v0, 10"<<endl
    << "syscall"<<endl;
    return ss.str();
}

int ExpressionStatement::evaluateSemantic(){
    return this->expression->getType();
}

string ExpressionStatement::genCode(){
    Code exprCode;
    this->expression->genCode(exprCode);
    releaseRegister(exprCode.place);
    return exprCode.code;
}

Type IntExpression::getType(){
    return INT;
}

void IntExpression::genCode(Code &code){
    stringstream ss;
    string intTemp = getIntTemp();
    ss << "li " << intTemp << ", "<< this->value <<endl;
    code.place = intTemp;
    code.type = INT;
    code.code = ss.str();
}

Type FloatExpression::getType(){
    return FLOAT32;
}

void FloatExpression::genCode(Code &code){
    stringstream ss;
    string floatTemp = getFloatTemp();
    ss << "li.s " << floatTemp << ", "<< this->value <<endl;
    code.place = floatTemp;
    code.type = FLOAT32;
    code.code = ss.str();
}

Type BoolExpression::getType(){
    return BOOL;
}

void BoolExpression::genCode(Code &code){
    stringstream ss;
    string intTemp = getIntTemp();
    if(this->value){
        ss<< "addi "<<intTemp<< ", $zero, 1"<<endl;
    }else{
        ss << "move " << intTemp << ", $zero"<<endl;
    }
    code.place = intTemp;
    code.type = BOOL;
    code.code = ss.str();
}

Type StringExpression::getType(){
    return STRING;
}

void StringExpression::genCode(Code &code){
    string strLabel = getNewLabel("string");
    stringstream ss;
    ss << strLabel <<": .asciiz" << this->value << ""<<endl;
    assemblyFile.data += ss.str(); 
    code.code = "";
    code.place = strLabel;
    code.type = STRING;
}

Type IdExpression::getType(){
    Type value;
    if(context != NULL){
        value = variableExists(this->value);
        if(value != INVALID)
            return value;
    }

    value = getVariableType(this->value);

    if(value == INVALID){
        cout<<"Error in line "<<this->line<<": Variable '"<<this->value<<"' was not declared in this scope."<<endl;
        exit(0);
    }
    return INVALID;
}

void IdExpression::genCode(Code &code){
    code.type = globalVariables[this->value];
    if(codeGenerationVars.find(this->value) == codeGenerationVars.end()){
        if(globalVariables[this->value] == INT_ARRAY || globalVariables[this->value] == FLOAT_ARRAY){
            string intTemp = getIntTemp();
            code.code = "la " + intTemp+ ", "+ this->value + "\n";
            code.place = intTemp;
        }
        if(globalVariables[this->value] == FLOAT32){
            string floatTemp = getFloatTemp();
            code.place = floatTemp;
            code.code = "l.s "+ floatTemp + ", "+ this->value + "\n";
        }else{
            string intTemp = getIntTemp();
            code.place = intTemp;
            code.code = "lw "+ intTemp + ", "+ this->value + "\n";
        }
    }
   else{
        
        code.type = codeGenerationVars[this->value]->type;
        if(codeGenerationVars[this->value]->type == FLOAT32 && !codeGenerationVars[this->value]->isArray){
            string floatTemp = getFloatTemp();
            code.place = floatTemp;
            code.code = "l.s "+ floatTemp + ", " +to_string(codeGenerationVars[this->value]->offset) +"($sp)\n";
        }else if(codeGenerationVars[this->value]->type == INT && !codeGenerationVars[this->value]->isArray){
            string intTemp = getIntTemp();
            code.place = intTemp;
            code.code = "lw "+ intTemp + ", " + to_string(codeGenerationVars[this->value]->offset) +"($sp)\n";
        }else if(codeGenerationVars[this->value]->isArray){
            string intTemp = getIntTemp();
            code.code = "la "+ intTemp + ", " + to_string(codeGenerationVars[this->value]->offset)+"($sp)\n";
            code.place = intTemp;
        }
    }
}

Type getUnaryType(Type expressionType, int unaryOperation, int line){
    switch(unaryOperation){
        case INCREMENT:
        case DECREMENT:
            if(expressionType == INT || expressionType == FLOAT32)
                return expressionType;
        case NOT:
            if(expressionType == BOOL)
                return BOOL;
    }

    cerr<<"Error in line "<<line<<": Invalid type"<<endl;
    exit(0);
}

Type UnaryExpression::getType(){
    //Tambien hay error para las unary cuando hay dos unary seguido y el primero es un bool
    Type expressionType = this->expression->getType();
    return getUnaryType(expressionType, this->type, this->line);
}

void UnaryExpression::genCode(Code &code){

    
}

Type PostIncrementExpression::getType(){
    
    if(this->expression->getType() != INT && this->expression->getType() != FLOAT32){
        cerr<<"Error in line "<<this->line<<": Invalid type"<<endl;
        exit(0);
    }

    return this->expression->getType();
}

void PostIncrementExpression::genCode(Code &code){
    Code exprCode;
    stringstream ss;

    this->expression->genCode(exprCode);
    ss<<exprCode.code<<endl;

    if(exprCode.type == INT){
        ss<<"addi "<<exprCode.place<<", "<<exprCode.place<<", "<<"1"<<endl
        <<"sw "<<exprCode.place<<", "<<to_string(codeGenerationVars[((IdExpression*)this->expression)->value]->offset)<<"($sp)"<<endl;
        code.place = exprCode.place;
        code.type = INT;
    }else if(exprCode.type == FLOAT32){
        string floatTemp = getFloatTemp();
        ss<<"li.s "<<floatTemp<<", "<<"1.0"<<endl
        <<"add.s "<<exprCode.place<<", "<<exprCode.place<<", "<<floatTemp<<endl
        <<"s.s "<<exprCode.place<<", "<<to_string(codeGenerationVars[((IdExpression*)this->expression)->value]->offset)<<"($sp)"<<endl;
        releaseRegister(floatTemp);
        code.place = exprCode.place;
        code.type = FLOAT32;
    }else if( exprCode.type == INT_ARRAY){
        string intTemp = getIntTemp();
        ss << "lw "<<intTemp<<", ("<<exprCode.place<<")"<<endl
        <<"addi "<<intTemp<<", "<<intTemp<<", "<<"1"<<endl
        <<"sw "<<intTemp<<", "<<to_string(codeGenerationVars[((IdExpression*)this->expression)->value]->offset)<<"($sp)"<<endl;
        releaseRegister(exprCode.place);
        code.place = intTemp;
        code.type = INT_ARRAY;
    }else if( exprCode.type == FLOAT_ARRAY ){
        string floatTemp = getFloatTemp();
        ss << "l.s "<<floatTemp<<", ("<<exprCode.place<<")"<<endl
        <<"s.s "<<floatTemp<<", "<<to_string(codeGenerationVars[((IdExpression*)this->expression)->value]->offset)<<"($sp)"<<endl;
        releaseRegister(exprCode.place);
        code.place = floatTemp;
        code.type = FLOAT_ARRAY;
    }

    code.code = ss.str();
}

Type PostDecrementExpression::getType(){
    
   if(this->expression->getType() != INT && this->expression->getType() != FLOAT32){
        cerr<<"Error in line "<<this->line<<": Invalid type"<<endl;
        exit(0);
    }

    return this->expression->getType();
}

void PostDecrementExpression::genCode(Code &code){
    Code exprCode;
    stringstream ss;

    this->expression->genCode(exprCode);
    ss<<exprCode.code<<endl;

    if(exprCode.type == INT){
        ss<<"addi "<<exprCode.place<<", "<<exprCode.place<<", "<<"-1"<<endl;
        code.place = exprCode.place;
        code.type = INT;
    }else if(exprCode.type == FLOAT32){
        string floatTemp = getFloatTemp();
        ss<<"li.s "<<floatTemp<<", "<<"1.0"<<endl
        <<"sub.s "<<exprCode.place<<", "<<exprCode.place<<", "<<floatTemp<<endl;
        releaseRegister(floatTemp);
        code.place = exprCode.place;
        code.type = FLOAT32;
    }else if( exprCode.type == INT_ARRAY){
        string intTemp = getIntTemp();
        ss << "lw "<<intTemp<<", ("<<exprCode.place<<")"<<endl
        <<"addi "<<intTemp<<", "<<intTemp<<", "<<"-1"<<endl
        <<"sw "<<intTemp<<", "<<to_string(codeGenerationVars[((IdExpression*)this->expression)->value]->offset)<<"($sp)"<<endl;
        releaseRegister(exprCode.place);
        code.place = intTemp;
        code.type = INT_ARRAY;
    }else if( exprCode.type == FLOAT_ARRAY ){
        string floatTemp = getFloatTemp();
        string floatTemp2 = getFloatTemp();
        ss<<"li.s "<<floatTemp2<<", "<<"1.0"<<endl
        << "l.s "<<floatTemp<<", ("<<exprCode.place<<")"<<endl
        <<"sub.s "<<floatTemp<<", "<<floatTemp<<", "<<floatTemp2<<endl
        <<"s.s "<<floatTemp<<", "<<to_string(codeGenerationVars[((IdExpression*)this->expression)->value]->offset)<<"($sp)"<<endl;
        releaseRegister(exprCode.place);
        releaseRegister(floatTemp2);
        code.place = floatTemp;
        code.type = FLOAT_ARRAY;
    }

    code.code = ss.str();
}

Type ArrayExpression::getType(){
    if(this->expression->getType() != INT){
        cout<<"Error in line "<<this->line<<": Storage size of '"<<this->id->value<<"' must be an int value."<<endl;
        exit(0);
    }
    return this->id->getType();
}

void ArrayExpression::genCode(Code &code){
    Code arrayCode;
    string name = this->id->value;
    stringstream ss;
    this->expression->genCode(arrayCode);
    //a[1]
    releaseRegister(arrayCode.place);
    if (codeGenerationVars.find(name) == codeGenerationVars.end())
    {
        string temp = getIntTemp();
        string labelAddress = getIntTemp();
        ss << arrayCode.code<<endl
        << "li $a0, 4"<<endl
        << "mult $a0, "<< arrayCode.place<<endl
        <<"mflo "<<temp<<endl
        << "la "<< labelAddress<<", "<< name<<endl
        << "add "<<temp<<", "<<labelAddress<<", "<<temp<<endl;
        releaseRegister(labelAddress);
        if(globalVariables[name] == INT){
           ss <<"lw "<< temp<<", 0("<<temp<<")"<<endl;
           code.place = temp;
        }else{
            string floatTemp = getFloatTemp();
            ss <<"l.s "<< floatTemp<<", 0("<<temp<<")"<<endl;
            code.place = floatTemp;
        }
    }else{
        string temp = getIntTemp();
        string address = getIntTemp();
        ss << arrayCode.code<<endl
        << "li $a0, 4"<<endl
        << "mult $a0, "<< arrayCode.place<<endl
        <<"mflo "<<temp<<endl;
        if(!codeGenerationVars[name]->isParameter)
            ss<< "la " + address +", " <<codeGenerationVars[name]->offset<<"($sp)\n";
        else
            ss << "lw "<<address<<", "<<codeGenerationVars[name]->offset<<"($sp)"<<endl;
        ss<< "add "<<temp<<", "<<address<<", "<<temp<<endl;
        releaseRegister(address);
        if(codeGenerationVars[name]->type == INT){
           ss <<"lw "<< temp<<", 0("<<temp<<")"<<endl;
           code.place = temp;
           code.type = INT;
        }else{
            string floatTemp = getFloatTemp();
            ss <<"l.s "<< floatTemp<<", 0("<<temp<<")"<<endl;
           code.place = floatTemp;
           code.type = FLOAT32;
        }
    }
    code.code = ss.str();
}

Type FunctionCallExpression::getType(){
    FunctionInfo * funcInfo = methods[this->id->value];

    if( funcInfo == NULL){
        cout<<"Error in line "<<this->line<<": Function '"<<this->id->value<<"' not found, line: "<<endl;
        exit(0);
    }

    Type funcType = funcInfo->returnType;

    if(funcInfo->parameters.size() > this->args.size()){
        cout<<"Error in line "<<this->line<<": Too few arguments to function '"<<this->id->value<<"'."<<endl;
        exit(0);
    }
    if(funcInfo->parameters.size() < this->args.size()){
        cout<<"Error in line "<<this->line<<": Too many arguments to function '"<<this->id->value<<"'."<<endl;
        exit(0);
    }

    list<Parameter *>::iterator paramIt = funcInfo->parameters.begin();
    list<Expression *>::iterator argsIt = this->args.begin();
    while(paramIt != funcInfo->parameters.end() && argsIt != this->args.end()){
        string paramType = getTypeName((*paramIt)->type);
        string argType = getTypeName((*argsIt)->getType());
        if( paramType != argType){
            cout<<"Error in line '"<<this->line<<"': Invalid conversion from: "<< argType <<" to " <<paramType<< " line: "<<this->line <<endl;
            exit(0);
        }
        paramIt++;
        argsIt++;
    }

    return funcType;
}

void FunctionCallExpression::genCode(Code &code){
    list<Expression *>::iterator it = this->args.begin();
    list<Code> codes;
    stringstream ss;
    Code argCode;
    while (it != this->args.end())
    {
        (*it)->genCode(argCode);
        ss << argCode.code <<endl;
        codes.push_back(argCode);
        it++;
    }

    int i = 0;
    list<Code>::iterator placesIt = codes.begin();
    while (placesIt != codes.end())
    {
        releaseRegister((*placesIt).place);
        if((*placesIt).type == FLOAT32){
            ss << "mfc1 $a"<<i<<", "<< (*placesIt).place<<endl;
        }else{
            ss << "move $a"<<i<<", "<< (*placesIt).place<<endl;
        }
        i++;
        placesIt++;
    }

    ss<< "jal "<< this->id->value<<endl;
    string reg;
    if(methods[this->id->value]->returnType == FLOAT32){
        reg = getFloatTemp();
        ss << "mtc1 $v0, "<< reg<<endl;
    }else{
        reg = getIntTemp();
        ss << "move "<< reg<<", $v0";
    }
    code.code = ss.str();
    code.place = reg;
    code.type = methods[this->id->value]->returnType;
}

Type FunctionInvocationExpression::getType(){
    this->id->getType();
    

    FunctionInfo * funcInfo = methods[this->expression->value];

    if( funcInfo == NULL){
        cout<<"Error in line "<<this->line<<": Function '"<<this->expression->value<<"' not found, line: "<<endl;
        exit(0);
    }

    list<Expression *>::iterator argsIt = this->args.begin();

    while( argsIt != this->args.end()){
        Expression * expression = (*argsIt);
        expression->getType();

        argsIt++;
    }

    Type funcType = funcInfo->returnType;

    // if(funcInfo->parameters.size() > this->args.size()){
    //     cout<<"Error in line "<<this->line<<": Too few arguments to function '"<<this->id->value<<"'."<<endl;
    //     exit(0);
    // }
    // if(funcInfo->parameters.size() < this->args.size()){
    //     cout<<"Error in line "<<this->line<<": Too many arguments to function '"<<this->id->value<<"'."<<endl;
    //     exit(0);
    // }

    // list<Parameter *>::iterator paramIt = funcInfo->parameters.begin();
    // list<Expression *>::iterator argsIt = this->args.begin();
    // while(paramIt != funcInfo->parameters.end() && argsIt != this->args.end()){
    //     string paramType = getTypeName((*paramIt)->type);
    //     string argType = getTypeName((*argsIt)->getType());
    //     if( paramType != argType){
    //         cout<<"Error in line '"<<this->line<<"': Invalid conversion from: "<< argType <<" to " <<paramType<< " line: "<<this->line <<endl;
    //         exit(0);
    //     }
    //     paramIt++;
    //     argsIt++;
    // }

    return funcType;
}

void FunctionInvocationExpression::genCode(Code &code){
    list<Expression *>::iterator it = this->args.begin();
    list<Code> codes;
    stringstream ss;
    Code argCode;
    while (it != this->args.end())
    {
        (*it)->genCode(argCode);
        ss << argCode.code <<endl;
        codes.push_back(argCode);
        it++;
    }

    int i = 0;
    list<Code>::iterator placesIt = codes.begin();
    while (placesIt != codes.end())
    {
        releaseRegister((*placesIt).place);
        if((*placesIt).type == FLOAT32){
            ss << "mfc1 $a"<<i<<", "<< (*placesIt).place<<endl;
        }else{
            ss << "move $a"<<i<<", "<< (*placesIt).place<<endl;
        }
        i++;
        placesIt++;
    }

    ss<< "jal "<< this->id->value<<endl;
    string reg;
    if(methods[this->id->value]->returnType == FLOAT32){
        reg = getFloatTemp();
        ss << "mtc1 $v0, "<< reg<<endl;
    }else{
        reg = getIntTemp();
        ss << "move "<< reg<<", $v0";
    }
    code.code = ss.str();
    code.place = reg;
    code.type = methods[this->id->value]->returnType;
}

#define IMPLEMENT_BINARY_GET_TYPE(name)\
Type name##Expression::getType(){\
    string leftType = getTypeName(this->leftExpression->getType());\
    string rightType = getTypeName(this->rightExpression->getType());\
    Type resultType = resultTypes[leftType+","+rightType];\
    if(resultType == 0){\
        cerr<< "Error in line "<<this->line<<": Cannot perform arithmetic operation between type "<< leftType <<" and type "<< rightType <<endl;\
        exit(0);\
    }\
    return resultType; \
}\

void toFloat(Code &code){
    if(code.type == INT){
        string floatTemp = getFloatTemp();
        stringstream ss;
        ss << code.code
        << "mtc1 "<< code.place << ", " << floatTemp <<endl
        << "cvt.s.w " << floatTemp<< ", " << floatTemp <<endl;
        releaseRegister(code.place);
        code.place = floatTemp;
        code.type = FLOAT32;
        code.code =  ss.str();
    }
    else{
        /* nothing */
    }
}

string intArithmetic(Code &leftCode, Code &rightCode, Code &code, char op){
    stringstream ss;
    code.place = getIntTemp();
    switch (op)
    {
        case '+':
            ss << "add "<< code.place<<", "<< leftCode.place <<", "<< rightCode.place;
            break;
        case '-':
            ss << "sub "<< code.place<<", "<< leftCode.place <<", "<< rightCode.place;
            break;
        case '*':
            ss << "mult "<< leftCode.place <<", "<< rightCode.place <<endl
            << "mflo "<< code.place;
            break;
        case '/':
            ss << "div "<< leftCode.place <<", "<< rightCode.place<<endl
            << "mflo "<< code.place;
            break;
        case '%':
            ss << "div "<< leftCode.place <<", "<< rightCode.place<<endl
            << "mfhi "<< code.place;
            break;
        default:
            break;
    }
    return ss.str();
}

string floatArithmetic(Code &leftCode, Code &rightCode, Code &code, char op){
    stringstream ss;
    code.place = getFloatTemp();
    switch (op)
    {
        case '+':
            ss << "add.s "<< code.place<<", "<< leftCode.place <<", "<< rightCode.place;
            break;
        case '-':
            ss << "sub.s "<< code.place<<", "<< leftCode.place <<", "<< rightCode.place;
            break;
        case '*':
            ss << "mul.s "<< code.place<<", "<< leftCode.place <<", "<< rightCode.place;
            break;
        case '/':
            ss << "div.s "<< code.place<<", "<< leftCode.place <<", "<< rightCode.place;
            break;
        default:
            break;
    }
    return ss.str();
}

#define IMPLEMENT_BINARY_ARIT_GEN_CODE(name, op)\
void name##Expression::genCode(Code &code){\
    Code leftCode, rightCode;\
    stringstream ss;\
    this->leftExpression->genCode(leftCode);\
    this->rightExpression->genCode(rightCode);\
    if(leftCode.type == INT && rightCode.type == INT){\
        code.type = INT;\
        releaseRegister(leftCode.place);\
        releaseRegister(rightCode.place);\
        ss<< leftCode.code << endl\
        << rightCode.code <<endl\
        << intArithmetic(leftCode, rightCode, code, op)<< endl;\
    }else{\
        code.type = FLOAT32;\
        toFloat(leftCode);\
        toFloat(rightCode);\
        releaseRegister(leftCode.place);\
        releaseRegister(rightCode.place);\
        ss << leftCode.code << endl\
        << rightCode.code <<endl\
        << floatArithmetic(leftCode, rightCode, code, op)<<endl;\
    }\
    code.code = ss.str();\
}\

#define IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(name)\
Type name##Expression::getType(){\
    string leftType = getTypeName(this->leftExpression->getType());\
    string rightType = getTypeName(this->rightExpression->getType());\
    Type resultType = booleanResultTypes[leftType+","+rightType];\
    if(resultType == 0){\
        cerr<< "Error in line "<<this->line<<": Cannot perform boolean operation between type "<< leftType <<" and type "<< rightType <<endl;\
        exit(0);\
    }\
    return BOOL; \
}\


void EqualExpression::genCode(Code &code){
    Code leftSideCode; 
    Code rightSideCode;
    this->leftExpression->genCode(leftSideCode);
    this->rightExpression->genCode(rightSideCode);
    stringstream ss;
    if (leftSideCode.type == INT && rightSideCode.type == INT)
    {
        code.type = INT;
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        ss<< leftSideCode.code <<endl
        << rightSideCode.code <<endl;
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        string temp = getIntTemp();
        string label = getNewLabel("label");
        string finalLabel = getNewLabel("finalLabel");
        ss << "beq " << leftSideCode.place << ", " << rightSideCode.place << ", " << label + "\n";
        ss << "addi " << temp << ", $zero, 0"<<endl << " j " << finalLabel <<endl;
        ss<< label <<":"<<endl<< "addi " << temp << ", $zero, 1"<<endl<<finalLabel<<":"<<endl;
        code.place = temp;
    }else{
        code.type = FLOAT32;
        toFloat(leftSideCode);
        toFloat(rightSideCode);
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        ss << leftSideCode.code << endl
        << rightSideCode.code <<endl
        << "c.eq.s "<< leftSideCode.place<< ", "<< rightSideCode.place<<endl;
    }
    code.code = ss.str();
}

void NotEqualExpression::genCode(Code &code){
    Code leftSideCode; 
    Code rightSideCode;
    this->leftExpression->genCode(leftSideCode);
    this->rightExpression->genCode(rightSideCode);
    stringstream ss;
    if (leftSideCode.type == INT && rightSideCode.type == INT)
    {
        code.type = INT;
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        ss<< leftSideCode.code <<endl
        << rightSideCode.code <<endl;
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        string temp = getIntTemp();
        string label = getNewLabel("label");
        string finalLabel = getNewLabel("finalLabel");
        ss << "bne " << leftSideCode.place << ", " << rightSideCode.place << ", " << label + "\n";
        ss << "addi " << temp << ", $zero, 1"<<endl 
        << " j " << finalLabel <<endl;
        ss<< label <<":"<<endl
        << "addi " << temp << ", $zero, 0"<<endl
        <<finalLabel<<":"<<endl;
        code.place = temp;
    }else{
        code.type = FLOAT32;
        toFloat(leftSideCode);
        toFloat(rightSideCode);
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        ss << leftSideCode.code << endl
        << rightSideCode.code <<endl
        << "c.eq.s "<< leftSideCode.place<< ", "<< rightSideCode.place<<endl;
    }
    code.code = ss.str();
}

void GteExpression::genCode(Code &code){
    Code leftSideCode; 
    Code rightSideCode;
    stringstream ss;
    this->leftExpression->genCode(leftSideCode);
    this->rightExpression->genCode(rightSideCode);
    if (leftSideCode.type == INT && rightSideCode.type == INT)
    {
        code.type = INT;
        ss << leftSideCode.code <<endl<< rightSideCode.code<<endl;
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        string temp = getIntTemp();
        ss<< "sge "<< temp<< ", "<< leftSideCode.place<< ", "<< rightSideCode.place<<endl;
        code.place = temp;
    }else{
        code.type = FLOAT32;
        toFloat(leftSideCode);
        toFloat(rightSideCode);
        ss << leftSideCode.code <<endl<< rightSideCode.code<<endl;
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        string finalGteLabel = getNewLabel("finalGteLabel");
        string temp = getIntTemp();
        ss << "c.eq.s " << leftSideCode.place << ", "<<rightSideCode.place<<endl
        <<"bc1t " <<finalGteLabel<<endl
        <<"c.lt.s " <<rightSideCode.place<<", "<<leftSideCode.place<<endl
        <<"bc1t " << finalGteLabel <<endl
        <<finalGteLabel<<": "<<endl;
        code.code = temp;
    }
    code.code = ss.str();
}

void LteExpression::genCode(Code &code){
    Code leftSideCode; 
    Code rightSideCode;
    stringstream ss;
    this->leftExpression->genCode(leftSideCode);
    this->rightExpression->genCode(rightSideCode);
    if (leftSideCode.type == INT && rightSideCode.type == INT)
    {
        code.type = INT;
        ss << leftSideCode.code <<endl<< rightSideCode.code<<endl;
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        string temp = getIntTemp();
        ss<< "sle "<< temp<< ", "<< leftSideCode.place<< ", "<< rightSideCode.place<<endl;
        code.place = temp;
    }else{
        code.type = FLOAT32;
        toFloat(leftSideCode);
        toFloat(rightSideCode);
        ss << leftSideCode.code <<endl<< rightSideCode.code<<endl;
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        ss<< "c.le.s "<< leftSideCode.place<< ", "<< rightSideCode.place<<endl;
    }
    code.code = ss.str();
}

void GtExpression::genCode(Code &code){
    Code leftSideCode; 
    Code rightSideCode;
    stringstream ss;
    this->leftExpression->genCode(leftSideCode);
    this->rightExpression->genCode(rightSideCode);
    if (leftSideCode.type == INT && rightSideCode.type == INT)
    {
        code.type = INT;
        ss << leftSideCode.code <<endl<< rightSideCode.code<<endl;
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        string temp = getIntTemp();
        ss<< "slt "<< temp<< ", "<< rightSideCode.place<< ", "<< leftSideCode.place<<endl;
        code.place = temp;
    }else{
        code.type = FLOAT32;
        toFloat(leftSideCode);
        toFloat(rightSideCode);
        ss << leftSideCode.code <<endl<< rightSideCode.code<<endl;
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        ss<< "c.lt.s "<< leftSideCode.place<< ", "<< rightSideCode.place<<endl;
    }
    code.code = ss.str();
}

void LtExpression::genCode(Code &code){
    Code leftSideCode; 
    Code rightSideCode;
    stringstream ss;
    this->leftExpression->genCode(leftSideCode);
    this->rightExpression->genCode(rightSideCode);
    if (leftSideCode.type == INT && rightSideCode.type == INT)
    {
        code.type = INT;
        ss << leftSideCode.code <<endl<< rightSideCode.code<<endl;
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        string temp = getIntTemp();
        ss<< "slt "<< temp<< ", "<< leftSideCode.place<< ", "<< rightSideCode.place<<endl;
        code.place = temp;
    }else{
        code.type = FLOAT32;
        toFloat(leftSideCode);
        toFloat(rightSideCode);
        ss << leftSideCode.code <<endl<< rightSideCode.code<<endl;
        releaseRegister(leftSideCode.place);
        releaseRegister(rightSideCode.place);
        ss<< "c.lt.s "<< leftSideCode.place<< ", "<< rightSideCode.place<<endl;
    }
    code.code = ss.str();
}

void LogicalAndExpression::genCode(Code &code){
    Code leftSideCode; 
    Code rightSideCode;
    this->leftExpression->genCode(leftSideCode);
    this->rightExpression->genCode(rightSideCode);
    stringstream ss;
    ss<< leftSideCode.code<<endl
    << rightSideCode.code <<endl;
    string temp = getIntTemp();
    string label = getNewLabel("label");
    string finalLabel = getNewLabel("finalLabel");
    ss<< "addi "<<temp<< ", $zero, 0"<<endl;
    ss<< "beq "<< leftSideCode.place<< ", "<<temp<<", "<< finalLabel<<endl;
    ss<< "beq "<< rightSideCode.place<< ", "<<temp<<", "<< finalLabel<<endl;
    ss<< label<< ":"<<endl
    << "addi "<< temp<< ", $zero, 1"<<endl
    <<finalLabel<<":"<<endl;
    releaseRegister(leftSideCode.place);
    releaseRegister(rightSideCode.place);
    code.place = temp;
    code.type = BOOL;
    code.code = ss.str();
}

void LogicalOrExpression::genCode(Code &code){
    Code leftSideCode; 
    Code rightSideCode;
    this->leftExpression->genCode(leftSideCode);
    this->rightExpression->genCode(rightSideCode);
    stringstream ss;
    ss<< leftSideCode.code<<endl<< rightSideCode.code<<endl;
    string temp = getIntTemp();
    string label = getNewLabel("label");
    string finalLabel = getNewLabel("finalLabel");
    ss<< "addi "<<temp<< ", $zero, 1"<<endl;
    ss<< "beq "<< leftSideCode.place<< ", "<<temp <<", " << finalLabel <<endl;
    ss<< "beq "<<rightSideCode.place<< ", "<<temp <<", " << finalLabel <<endl;
    ss<< label <<":"<<endl
    << "addi "<< temp<< ", $zero, 0"<<endl
    <<finalLabel<<":"<<endl;
    releaseRegister(leftSideCode.place);
    releaseRegister(rightSideCode.place);
    code.place = temp;
    code.code = ss.str();
}

void AuxAssignExpre(ExpressionList leftExpressionList, InitializerList rightExpressionList, int line){
    list<Expression *>::iterator leftIt = leftExpressionList.begin();
    list<Initializer *>::iterator rightIt = rightExpressionList.begin();
    if( leftExpressionList.size() != rightExpressionList.size()){
        cerr<<"Error in line "<<line<<": Initializers list must match assignment list."<<endl;\
        exit(0);
    }
    int leftExprIndex = 0;
    while( leftIt != leftExpressionList.end()){
        Expression * leftExpr = (*leftIt);
        list<Initializer *>::iterator rightIt2 = rightExpressionList.begin();
        advance(rightIt2, leftExprIndex);
        Initializer * rightExpr = (*rightIt2);

        if(leftExpr->getType() != rightExpr->initializer->getType()){
            cerr<<"Error in line "<<line<<". Value of type "<<getTypeName(rightExpr->initializer->getType())<<" cannot be assigned to "<<getTypeName(leftExpr->getType())<<" type."<<endl;
            exit(0);
        }
        leftIt++;
        leftExprIndex++;
    }
}


Type AssignExpression::getType(){
    AuxAssignExpre(this->leftExpressionList, this->rightExpressionList, this->line);
    return BOOL;
}

Type PlusAssignExpression::getType(){
    AuxAssignExpre(this->leftExpressionList, this->rightExpressionList, this->line);
    return BOOL;
}

Type MinusAssignExpression::getType(){
    AuxAssignExpre(this->leftExpressionList, this->rightExpressionList, this->line);
    return BOOL;
}

Type MultAssignExpression::getType(){
    AuxAssignExpre(this->leftExpressionList, this->rightExpressionList, this->line);
    return BOOL;
}

Type DivAssignExpression::getType(){
    AuxAssignExpre(this->leftExpressionList, this->rightExpressionList, this->line);
    return BOOL;
}

Type ModAssignExpression::getType(){
    AuxAssignExpre(this->leftExpressionList, this->rightExpressionList, this->line);
    return BOOL;
}

Type PowAssignExpression::getType(){
    AuxAssignExpre(this->leftExpressionList, this->rightExpressionList, this->line);
    return BOOL;
}

Type AndAssignExpression::getType(){
    AuxAssignExpre(this->leftExpressionList, this->rightExpressionList, this->line);
    return BOOL;
}

Type OrAssignExpression::getType(){
    AuxAssignExpre(this->leftExpressionList, this->rightExpressionList, this->line);
    return BOOL;
}

void AssignExpression::genCode(Code &code){
    Code initializerCode;
    stringstream ss;

    list<Expression *>::iterator declaratorsIt = this->leftExpressionList.begin();

    int declaratorIndex = 0;
    while( declaratorsIt != this->leftExpressionList.end()){
        Expression * declarator = (*declaratorsIt);
        // IdExpression * declaratorId = (IdExpression*) declarator;

        list<Initializer *>::iterator initializerIt = this->rightExpressionList.begin();
        advance(initializerIt, declaratorIndex);
        Initializer * initializer = (*initializerIt);
        
        initializer->initializer->genCode(initializerCode);
        ss << initializerCode.code <<endl;
        
        if( codeGenerationVars.find(((IdExpression*)declarator)->value) != codeGenerationVars.end() ){
            string declaratorName =  ((IdExpression *)declarator)->value;
            
            if(codeGenerationVars.find(declaratorName) == codeGenerationVars.end()){
                if(initializerCode.type == INT || initializerCode.type == BOOL){
                    ss << "sw "<<initializerCode.place << ", "<<declaratorName <<endl;
                }else if( initializerCode.type == FLOAT32 ){
                    ss << "s.s "<<initializerCode.place << ", "<<declaratorName <<endl;
                }
            }else{
                if(initializerCode.type == INT || initializerCode.type == BOOL)
                    ss<< "sw "<< initializerCode.place <<", "<<codeGenerationVars[declaratorName]->offset<<"($sp)"<<endl;
                else if(initializerCode.type == FLOAT32)
                    ss<< "s.s "<< initializerCode.place <<", "<<codeGenerationVars[declaratorName]->offset<<"($sp)"<<endl;
            }
        }else{
            Code arrayExpr;
            string name = ((ArrayExpression *)declarator)->id->value;
            ((ArrayExpression *)declarator)->expression->genCode(arrayExpr);
            releaseRegister(arrayExpr.place);
            ss << arrayExpr.code<<endl;
            string temp = getIntTemp();
            string address = getIntTemp();
            if(codeGenerationVars.find(name) == codeGenerationVars.end()){
                ss<< "li $a0, 4"<<endl
                << "mult $a0, "<< arrayExpr.place<<endl
                <<"mflo "<<temp<<endl
                << "la " <<address<< ", "<< name<<endl
                <<"add "<< temp << ", "<< temp << ", "<< address<<endl;
                Code rightSideCode;
                initializer->initializer->genCode(rightSideCode);
                ss<< rightSideCode.code <<endl
                << "sw "<< rightSideCode.place << ", 0(" <<temp<< ")"<<endl;
            }else{
                ss << "li $a0, 4"<<endl
                << "mult $a0, "<< arrayExpr.place<<endl
                <<"mflo "<<temp<<endl;
                if(!codeGenerationVars[name]->isParameter)
                    ss<< "la " + address +", " <<codeGenerationVars[name]->offset<<"($sp)"<<endl;
                else
                    ss << "lw "<<address<<", "<<codeGenerationVars[name]->offset<<"($sp)"<<endl;
                ss <<"add "<< temp << ", "<< temp << ", "<< address<<endl;
                Code rightSideCode;
                initializer->initializer->genCode(rightSideCode);
                ss<< rightSideCode.code <<endl;
                ss<< "sw "<< rightSideCode.place << ", 0("<<temp<<")"<<endl;
            }    
            releaseRegister(temp);     
            releaseRegister(address);     
            releaseRegister(initializerCode.place);
            releaseRegister(arrayExpr.place);
        }
        declaratorsIt++;
    }
    code.code = ss.str();
}

void PlusAssignExpression::genCode(Code &code){

}

void MinusAssignExpression::genCode(Code &code){

}

void MultAssignExpression::genCode(Code &code){

}

void DivAssignExpression::genCode(Code &code){

}

void ModAssignExpression::genCode(Code &code){

}

void PowAssignExpression::genCode(Code &code){

}

void AndAssignExpression::genCode(Code &code){

}

void OrAssignExpression::genCode(Code &code){

}

void InitializerIsArray(Initializer * initializer, IdExpression * declarator, int line){
    if( initializer->initializer != NULL ){
        if( initializer->initializer->getType() != INT){
            cout<<"Error in line "<<line<<": Storage size of '"<<declarator->value<<"' must be an int value."<<endl;
            exit(0);
        }
    }

    list<Expression *>::iterator arrayIt = initializer->arrayValues.begin();
    while( arrayIt != initializer->arrayValues.end()){
        Expression * arrayValue = (*arrayIt);
        if(arrayValue->getType() != initializer->type ){
            cout<<"Error in line "<<line<<": Value of type "<<getTypeName(arrayValue->getType())<<" cannot be converted to "<<getTypeName(initializer->type)<<" type."<<endl;
            exit(0);
        }
        arrayIt++;
    }
}

void AuxShortDeclaration(ExpressionList declaratorsList, InitializerList initializerList, int line){
    list<Expression *>::iterator declaratorIt = declaratorsList.begin();
    list<Initializer *>::iterator initializerIt = initializerList.begin();

    if( !initializerList.empty()){
        if( declaratorsList.size() != initializerList.size()){
            cout<<"Error in line "<<line<<": Initializers list must match declarators list."<<endl;
            exit(0);
        }
    }
    int declaratorItIndex = 0;
    while(declaratorIt != declaratorsList.end()){
        Expression * expr = (*declaratorIt);
        IdExpression * declarator = (IdExpression*) expr;
       if(!variableExists(declarator->value)){
            list<Initializer *>::iterator initializerIt2 = initializerList.begin();
            advance(initializerIt2, declaratorItIndex);
            Initializer * initializer  = (*initializerIt2);
            if( initializer->isArrayInitializer ){
                InitializerIsArray(initializer, declarator, line);
                context->variables[declarator->value] = initializer->type;
            }else{
                context->variables[declarator->value] = initializer->initializer->getType();
            }
            
        }else{
            cout<<"Error in line "<<line<<": Redefinition of variable '"<< declarator->value<< "'."<<endl;
            exit(0);
        }
       
        declaratorIt++;
        declaratorItIndex++;
    }
}

Type ColonAssignExpression::getType(){
    AuxShortDeclaration(this->leftExpressionList, this->rightExpressionList, this->line);
    return BOOL;
}

void ColonAssignExpression::genCode(Code &code){
    stringstream ss;
    list<Expression *>::iterator it = this->leftExpressionList.begin();

    int declaratorItIndex = 0;
    while( it != this->leftExpressionList.end()){
        Expression * declarator = (*it);
        IdExpression * declaratorId = (IdExpression*)declarator;
        
        list<Initializer *>::iterator initializerIt = this->rightExpressionList.begin();
        advance(initializerIt, declaratorItIndex);
        Initializer * initializer  = (*initializerIt);
        
        if( !initializer->isArrayInitializer ){
            codeGenerationVars[declaratorId->value] = new VariableInfo(globalStackPointer, false, false, initializer->initializer->getType());
            globalStackPointer +=4;
        }else{
            codeGenerationVars[declaratorId->value] = new VariableInfo(globalStackPointer, true, false, initializer->type);
            // if(initializer->initializer == NULL){
                if(initializer->initializer != NULL){
                    int size = ((IntExpression *)initializer->initializer)->value;
                    globalStackPointer += (size * 4);
                    arraySize = size;
                }
            // }
        }

        if(initializer != NULL){
            list<Expression *>::iterator itExpr = initializer->arrayValues.begin();
            int offset = codeGenerationVars[declaratorId->value]->offset;
            if( !initializer->isArrayInitializer){
                Code exprCode;
                initializer->initializer->genCode(exprCode);
                ss << exprCode.code <<endl;


                if(exprCode.type == INT)
                    ss << "sw " << exprCode.place <<", "<< offset << "($sp)"<<endl;
                else if(exprCode.type == FLOAT32)
                    ss << "s.s " << exprCode.place <<", "<< offset << "($sp)"<<endl;
                releaseRegister(exprCode.place);
                itExpr++;
            }else{
            
                for (int i = 0; i < initializer->arrayValues.size(); i++)
                {
                    Code exprCode;
                    (*itExpr)->genCode(exprCode);
                    ss << exprCode.code <<endl;

                    if(exprCode.type == INT)
                        ss << "sw " << exprCode.place <<", "<< offset << "($sp)"<<endl;
                    else if(exprCode.type == FLOAT32)
                        ss << "s.s " << exprCode.place <<", "<< offset << "($sp)"<<endl;
                    releaseRegister(exprCode.place);
                    itExpr++;
                    globalStackPointer+=4;
                    offset += 4;
                }
            }
        }

        it++;
    }

    code.code = ss.str();
}


IMPLEMENT_BINARY_GET_TYPE(Add);
IMPLEMENT_BINARY_GET_TYPE(Sub);
IMPLEMENT_BINARY_GET_TYPE(Mult);
IMPLEMENT_BINARY_GET_TYPE(Div);
IMPLEMENT_BINARY_GET_TYPE(Mod);
IMPLEMENT_BINARY_GET_TYPE(Pow);

IMPLEMENT_BINARY_ARIT_GEN_CODE(Add, '+');
IMPLEMENT_BINARY_ARIT_GEN_CODE(Sub, '-');
IMPLEMENT_BINARY_ARIT_GEN_CODE(Mult, '*');
IMPLEMENT_BINARY_ARIT_GEN_CODE(Div, '/');
IMPLEMENT_BINARY_ARIT_GEN_CODE(Mod, '%');
IMPLEMENT_BINARY_ARIT_GEN_CODE(Pow, '^');

IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(Equal);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(NotEqual);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(Gte);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(Lte);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(Gt);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(Lt);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(LogicalAnd);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(LogicalOr);

