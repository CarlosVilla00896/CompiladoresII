#include "ast.h"
#include <iostream>
#include <sstream>
#include <set>

class VariableInfo{
    public:
        VariableInfo(bool isArray, bool isParameter, Type type){
            this->isArray = isArray;
            this->isParameter = isParameter;
            this->type = type;
        }
        bool isArray;
        bool isParameter;
        Type type;
};

// map<string, VariableInfo *> codeGenerationVars;

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
    {"BOOL,BOOL", BOOL},
};

map<string, Type> booleanResultTypes ={
    {"INT,INT", BOOL},
    {"FLOAT32,FLOAT32", BOOL},
    {"INT,FLOAT32", BOOL},
    {"FLOAT32,INT", BOOL},
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

int GlobalDeclaration::evaluateSemantic(){
    VarDeclaration * varDeclaration = this->declarationStatement;
    AuxVarDeclaration(varDeclaration->declaratorsList, varDeclaration->initializerList, varDeclaration->type, varDeclaration->line, true);
    return 2;
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

int ForStatement::evaluateSemantic(){

    if(this->initExpr != NULL){
        this->initExpr->getType();
    }

    if(this->conditionalExpr != NULL){
        if(this->conditionalExpr->getType() != BOOL){
            cout<<"Error in line "<<this->line<<": Conditional expression in for statement must be boolean."<<endl;
        } 
    }

    if(this->initExpr != NULL){
        this->initExpr->getType();
    }

    pushContext();
    if(this->statement != NULL){
        this->statement->evaluateSemantic();
    }
    popContext();
    return 0;
}

int ReturnStatement::evaluateSemantic(){
    list<Expression *>::iterator iterator = this->expressionList.begin();

    while(iterator != this->expressionList.end()){
        (*iterator)->getType();
        iterator++;
    }
    return 0;
}

int JumpStatement::evaluateSemantic(){
    //nada
    return 0;
}

int ExpressionStatement::evaluateSemantic(){
    return this->expression->getType();
}

Type IntExpression::getType(){
    return INT;
}

Type FloatExpression::getType(){
    return FLOAT32;
}

Type BoolExpression::getType(){
    return BOOL;
}

Type StringExpression::getType(){
    return STRING;
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

Type PostIncrementExpression::getType(){
    
    if(this->expression->getType() != INT && this->expression->getType() != FLOAT32){
        cerr<<"Error in line "<<this->line<<": Invalid type"<<endl;
        exit(0);
    }

    return this->expression->getType();
}

Type PostDecrementExpression::getType(){
    
   if(this->expression->getType() != INT && this->expression->getType() != FLOAT32){
        cerr<<"Error in line "<<this->line<<": Invalid type"<<endl;
        exit(0);
    }

    return this->expression->getType();
}

Type ArrayExpression::getType(){
    if(this->expression->getType() != INT){
        cout<<"Error in line "<<this->line<<": Storage size of '"<<this->id->value<<"' must be an int value."<<endl;
    }

    return this->id->getType();
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

Type AssignExpression::getType(){
    Type value;

    return value;
}

Type PlusAssignExpression::getType(){
    Type value;

    return value;
}

Type MinusAssignExpression::getType(){
    Type value;

    return value;
}

Type MultAssignExpression::getType(){
    Type value;

    return value;
}

Type DivAssignExpression::getType(){
    Type value;

    return value;
}

Type ModAssignExpression::getType(){
    Type value;

    return value;
}

Type PowAssignExpression::getType(){
    Type value;

    return value;
}

Type AndAssignExpression::getType(){
    Type value;

    return value;
}

Type OrAssignExpression::getType(){
    Type value;

    return value;
}

Type ColonAssignExpression::getType(){
    Type value;

    return value;
}


IMPLEMENT_BINARY_GET_TYPE(Add);
IMPLEMENT_BINARY_GET_TYPE(Sub);
IMPLEMENT_BINARY_GET_TYPE(Mult);
IMPLEMENT_BINARY_GET_TYPE(Div);
IMPLEMENT_BINARY_GET_TYPE(Mod);
IMPLEMENT_BINARY_GET_TYPE(Pow);

IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(Equal);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(NotEqual);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(Gte);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(Lte);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(Gt);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(Lt);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(LogicalAnd);
IMPLEMENT_BINARY_BOOLEAN_GET_TYPE(LogicalOr);

