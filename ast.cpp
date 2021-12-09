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
    //Parece que esta validacion no es la correcta
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
  //deberia retornar false? Al parecer no checa el primer contexto
  return false;
}

int VarDeclaration::evaluateSemantic(){
    list<Declarator *>::iterator declaratorIt = this->declaratorsList.begin();
    list<Initializer *>::iterator initializerIt = this->initializerList.begin();

    if( !this->initializerList.empty()){
        if( this->declaratorsList.size() != this->initializerList.size()){
            cout<<"Error: Initializers list must match declarators list. Line: "<<this->line<<endl;
            // exit(0);
        }

        if(this->type != NULLTYPE){
            while(initializerIt != this->initializerList.end() ){
                Initializer * init = (*initializerIt);
                if(init->initializer->getType() != this->type){
                    cout<<"Value of type "<<getTypeName(init->initializer->getType())<<" cannot be assigned to "<<getTypeName(this->type)<<" type. Line: "<<this->line<<endl;
                    // exit(0);
                }

                initializerIt++;
            }
        }
    }

    while(declaratorIt != this->declaratorsList.end()){
        Declarator * declarator = (*declaratorIt);
        if(declarator->isArray){
            if(declarator->arraySize == NULL ){
                cout<<"error: storage size of: "<<declarator->id<< " array is unknown. Line: "<<this->line<<endl;\
                exit(0);
            }

            if(declarator->arraySize->getType() != INT){
                cout<<"error: storage size must be an int value"<< ". Line: "<<this->line<<endl;
                exit(0);
            }
        }
        declaratorIt++;
    }

    
    return 0;
}

int GlobalDeclaration::evaluateSemantic(){
    this->declarationStatement->evaluateSemantic();
    return 0;
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
    return 0;
}

int FunctionDefinition::evaluateSemantic(){
    
    return 0;
}

int IfStatement::evaluateSemantic(){

    return 0;
}

int ElseStatement::evaluateSemantic(){

    return 0;
}

int ForStatement::evaluateSemantic(){

    return 0;
}

int ReturnStatement::evaluateSemantic(){
    
    return 0;
}

int JumpStatement::evaluateSemantic(){
    
    return 0;
}

int ExpressionStatement::evaluateSemantic(){
    
    return 0;
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




