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
        if(currContext->variables[id] != 0)
            return currContext->variables[id];
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

bool variableExists(string id){
  Type value;
  if(context != NULL){
    value = getLocalVariableType(id);
    //context->variables[id] != 0
    if(value != 0)
      return true;
  }
  if( globalVariables[id] != 0 ){
      return true;
  }
  return false;
}

void AuxVarDeclaration(DeclaratorList declaratorsList, InitializerList initializerList, Type type, int line, bool isGlobalDeclaration){
    list<Declarator *>::iterator declaratorIt = declaratorsList.begin();
    list<Initializer *>::iterator initializerIt = initializerList.begin();

    if( !initializerList.empty()){
        if( declaratorsList.size() != initializerList.size()){
            cout<<"Error: Initializers list must match declarators list. Line "<<line<<endl;
            // exit(0);
        }

        if(type != NULLTYPE){
            while(initializerIt != initializerList.end() ){
                Initializer * init = (*initializerIt);
                if(init->initializer->getType() != type){
                    cout<<"Error in line "<<line<<". Value of type "<<getTypeName(init->initializer->getType())<<" cannot be assigned to "<<getTypeName(type)<<" type."<<endl;
                    // exit(0);
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

    return value;
}

Type UnaryExpression::getType(){
    Type value;

    return value;
}

Type PostIncrementExpression::getType(){
    Type value;

    return value;
}

Type PostDecrementExpression::getType(){
    Type value;

    return value;
}

Type ArrayExpression::getType(){
    Type value;

    return value;
}

Type FunctionCallExpression::getType(){
    Type value;

    return value;
}

Type FunctionInvocationExpression::getType(){
    Type value;

    return value;
}

Type AddExpression::getType(){
    Type value;

    return value;
}

Type SubExpression::getType(){
    Type value;

    return value;
}

Type MultExpression::getType(){
    Type value;

    return value;
}

Type DivExpression::getType(){
    Type value;

    return value;
}

Type ModExpression::getType(){
    Type value;

    return value;
}

Type PowExpression::getType(){
    Type value;

    return value;
}

Type EqualExpression::getType(){
    Type value;

    return value;
}

Type NotEqualExpression::getType(){
    Type value;

    return value;
}

Type GteExpression::getType(){
    Type value;

    return value;
}

Type LteExpression::getType(){
    Type value;

    return value;
}

Type GtExpression::getType(){
    Type value;

    return value;
}

Type LtExpression::getType(){
    Type value;

    return value;
}

Type LogicalAndExpression::getType(){
    Type value;

    return value;
}

Type LogicalOrExpression::getType(){
    Type value;

    return value;
}

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




