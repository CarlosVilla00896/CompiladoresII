#include <string>
#include <list>
#include <map>
#include "type.h" //Remember to remove this from here when doing code generation

using namespace std;

class Expression;
class InitDeclarator;
class Declaration;
class Parameter;
class Statement;
class Declarator;
class Initializer;

typedef list<Declarator *> DeclaratorList;
typedef list<Initializer *> InitializerList;
typedef list<Parameter* > ParametersList;
typedef list<Type>TypeList;
typedef list<Statement *> StatementList;
typedef list<Expression *> ExpressionList;
typedef list<Expression *> ArgumentsList;
typedef list<Expression *> ReturnExpressionList;
typedef list<Expression *> AssignmentList;


enum StatementKind{
    FOR_STATEMENT,
    IF_STATEMENT,
    EXPRESSION_STATEMENT,
    RETURN_STATEMENT,
    CONTINUE_STATEMENT,
    BREAK_STATEMENT,
    BLOCK_STATEMENT,
    FUNCTION_DEFINITION_STATEMENT,
    GLOBAL_DECLARATION_STATEMENT,
    ELSE_STATEMENT,
    JUMP_STATEMENT,
    VAR_DECLARATION_STATEMENT
};

enum UnaryType{
    INCREMENT,
    DECREMENT,
    NOT
};

class Expression {
    public:
        int line;
        virtual Type getType() = 0;
};

class Statement{
    public:
        int line;
        virtual int evaluateSemantic() = 0;
        virtual StatementKind getKind() = 0;
};

class Initializer{
    public:
        Initializer(Expression * initializer, ExpressionList arrayValues, bool isArrayInitializer, Type type, int line){
            this->initializer = initializer;
            this->arrayValues = arrayValues;
            this->type = type;
            this->isArrayInitializer = isArrayInitializer;
            this->line = line;
        }

        Expression * initializer;
        ExpressionList arrayValues;
        Type type;
        bool isArrayInitializer;
        int line;
};

class Declarator{
    public:
        Declarator(string id, Expression * arraySize, bool isArray, int line){
            this->id = id;
            this->arraySize = arraySize;
            this->isArray = isArray;
            this->line = line;
        }

        string id;
        Expression * arraySize;
        bool isArray;
        int line;
};

//Lista de las variables de la declaracion, probablemente no necesito esto.
class DeclarationList{
    public:
        DeclarationList(DeclaratorList declarators, int line){
            this->declarators = declarators;
            this->line = line;
        }

        DeclaratorList declarators;
        int line;
};

class VarDeclaration : public Statement{
    public:
        VarDeclaration(DeclaratorList declaratorsList, InitializerList initializerList, Type type, int line ) {
            this->declaratorsList = declaratorsList;
            this->initializerList = initializerList;
            this->type = type;
            this->line = line - 1;
        }

        DeclaratorList declaratorsList;
        InitializerList initializerList;
        Type type;
        int line;

        int evaluateSemantic();
        StatementKind getKind(){
            return VAR_DECLARATION_STATEMENT;
        }
};


class GlobalDeclaration : public Statement{
    public:
        GlobalDeclaration(Statement * declarationStatement){
            this->declarationStatement = declarationStatement;
        }

        Statement * declarationStatement;

        int evaluateSemantic();
        StatementKind getKind(){
            return GLOBAL_DECLARATION_STATEMENT;
        }
};

class Parameter{
    public:
        Parameter(string id, Type type, bool isArray, int line){
            this->id = id;
            this->type = type;
            this->isArray = isArray;
            this->line = line;
        }
        
        string id;
        Type type;
        bool isArray;
        int line;
};

class BlockStatement : public Statement {
    public:
        BlockStatement(StatementList statements, int line){
            this->statements = statements;
            this->line = line;
        }

        StatementList statements;
        int line;

        int evaluateSemantic();
        StatementKind getKind(){
            return BLOCK_STATEMENT;
        }
};

class FunctionDefinition : public Statement {
    public: 
        FunctionDefinition( string id, ParametersList params, Type type, TypeList typeList, string functionKind, Statement * statement, int line){
            this->id = id;
            this->parametersList = parametersList;
            this->type = type;
            this->functionKind = functionKind;
            this->statement = statement;
            this->line = line;
        }

        string id;
        ParametersList parametersList;
        Type type;
        TypeList typeList;
        string functionKind;
        Statement * statement;
        int line;

        int evaluateSemantic();
        StatementKind getKind(){
            return FUNCTION_DEFINITION_STATEMENT;
        }
};


class IntExpression : public Expression{
    public:
        IntExpression(int value, int line){
            this->value = value;
            this->line = line;
        }

        int value;
        int line;

        Type getType();
};


class FloatExpression : public Expression{
    public:
        FloatExpression(float value, int line){
            this->value = value;
            this->line = line;
        }

        float value;
        int line;

        Type getType();
};

class BoolExpression : public Expression{
    public:
        BoolExpression(bool value, int line){
            this->value = value;
            this->line = line;
        }

        bool value;
        int line;

        Type getType();
};

class StringExpression : public Expression{
    public:
        StringExpression(string value, int line){
            this->value = value;
            this->line = line;
        }
        string value;
        int line;

        Type getType();
};

class IdExpression : public Expression{
    public:
        IdExpression(string id, int line){
            this->value = value;
            this->line = line;
        }

        string value;
        int line;

        Type getType();
};

class BinaryExpression : public Expression{
    public:
        BinaryExpression(Expression * leftExpression, Expression * rightExpression, int line){
            this->leftExpression = leftExpression;
            this->rightExpression = rightExpression;
            this->line = line;
        }

        Expression * leftExpression;
        Expression * rightExpression;
        int line;
};

class BinaryAssignExpression : public Expression{
    public:
        BinaryAssignExpression(ExpressionList leftExpressionList, InitializerList rightExpressionList, int line){
            this->leftExpressionList = leftExpressionList;
            this->rightExpressionList = rightExpressionList;
            this->line = line;
        }

        ExpressionList leftExpressionList;
        InitializerList rightExpressionList;
        int line;
};

#define IMPLEMENT_BINARY_EXPRESSION(name) \
class name##Expression : public BinaryExpression{\
    public: \
        name##Expression(Expression * leftExpression, Expression * rightExpression, int line) : BinaryExpression(leftExpression, rightExpression, line){}\
        Type getType(); \
};

#define IMPLEMENT_BINARY_ASSIGN_EXPRESSION(name) \
class name##Expression : public BinaryAssignExpression{\
    public: \
        name##Expression(ExpressionList leftExpressionList, InitializerList rightExpressionList, int line) : BinaryAssignExpression(leftExpressionList, rightExpressionList, line){}\
        Type getType(); \
};

class UnaryExpression : public Expression{
    public:
        UnaryExpression(int type, Expression* expression, int line){
            this->type = type;
            this->expression = expression;
            this->line = line;
        }
        int type;
        Expression* expression;
        int line;

        Type getType();
};

class PostIncrementExpression: public Expression{
    public:
        PostIncrementExpression(Expression * expression, int line){
            this->expression = expression;
            this->line = line;
        }
        Expression * expression;
        int line;

        Type getType();
};

class PostDecrementExpression: public Expression{
    public:
        PostDecrementExpression(IdExpression * id, int line){
            this->id = id;
            this->line = line;
        }
        IdExpression * id;
        int line;

        Type getType();
};

class ArrayExpression : public Expression{
    public:
        ArrayExpression(IdExpression * id, Expression * expression, int line){
            this->id = id;
            this->expression = expression;
            this->line = line;
        }
        IdExpression * id;
        Expression * expression;
        int line;

        Type getType();
};

class FunctionCallExpression : public Expression{
    public:
        FunctionCallExpression(IdExpression * id, ArgumentsList args, int line){
            this->id = id;
            this->args = args;
            this->line = line;
        }
        IdExpression * id;
        ArgumentsList args;
        int line;

        Type getType();

};

class FunctionInvocationExpression : public Expression{
    public:
        FunctionInvocationExpression(IdExpression * id, Expression * expression, int line){
            this->id = id;
            this->expression = expression;
            this->line = line;
        }

        IdExpression * id;
        Expression * expression;
        int line;

        Type getType();
};

class ElseStatement : public Statement{
    public:
        ElseStatement(Expression * initExpression, Expression * conditionalExpression, Statement * trueStatement, Statement * falseStatement, int line){
            this->initExpression = initExpression;
            this->conditionalExpression = conditionalExpression;
            this->trueStatement = trueStatement;
            this->falseStatement = falseStatement;
            this->line = line;
        }

        Expression * initExpression;
        Expression * conditionalExpression;
        Statement * trueStatement;
        Statement * falseStatement;
        int line;

        int evaluateSemantic();
        StatementKind getKind(){
            return ELSE_STATEMENT;
        }
};

class IfStatement : public Statement{
    public:
        IfStatement(Expression * initExpression, Expression * conditionalExpression, Statement * trueStatement, int line){
            this->initExpression = initExpression;
            this->conditionalExpression = conditionalExpression;
            this->trueStatement = trueStatement;
            this->line = line;
        }

        Expression * initExpression;
        Expression * conditionalExpression;
        Statement * trueStatement;
        int line;

        int evaluateSemantic();
        StatementKind getKind(){
            return IF_STATEMENT;
        }
};

class ForStatement : public Statement{
    public: 
        ForStatement(Expression * initExpr, Expression * conditionalExpr, Expression * postExpr, Statement * statement, int line){
            this->initExpr = initExpr;
            this->conditionalExpr = conditionalExpr;
            this->postExpr = postExpr;
            this->statement = statement;
            this->line = line;
        }

        Expression * initExpr; 
        Expression * conditionalExpr; 
        Expression * postExpr; 
        Statement * statement;
        int line;

        int evaluateSemantic();
        StatementKind getKind(){
            return FOR_STATEMENT;
        }
};

class ReturnStatement : public Statement{
    public:
        ReturnStatement( ReturnExpressionList expressionList, int line ){
            this->expressionList = expressionList;
            this->line = line;
        }

        ReturnExpressionList expressionList;
        int line;

        int evaluateSemantic();
        StatementKind getKind(){
            return RETURN_STATEMENT;
        }
};

class JumpStatement : public Statement{
    public:
        JumpStatement(int jumpKind, int line ){
            this->jumpKind = jumpKind;
            this->line = line;
        }

    int jumpKind;
    int line;

    int evaluateSemantic();
    StatementKind getKind(){
        return JUMP_STATEMENT;
    }
};

class ExpressionStatement : public Statement{
    public:
        ExpressionStatement(Expression * expression, int line){
            this->expression = expression;
            this->line = line;
        }

        Expression * expression;
        int line;

        int evaluateSemantic();
        StatementKind getKind(){
            return EXPRESSION_STATEMENT;
        }
};

IMPLEMENT_BINARY_EXPRESSION(Add);
IMPLEMENT_BINARY_EXPRESSION(Sub);
IMPLEMENT_BINARY_EXPRESSION(Mult);
IMPLEMENT_BINARY_EXPRESSION(Div);
IMPLEMENT_BINARY_EXPRESSION(Mod);
IMPLEMENT_BINARY_EXPRESSION(Pow);
IMPLEMENT_BINARY_EXPRESSION(Equal);
IMPLEMENT_BINARY_EXPRESSION(NotEqual);
IMPLEMENT_BINARY_EXPRESSION(Gte);
IMPLEMENT_BINARY_EXPRESSION(Lte);
IMPLEMENT_BINARY_EXPRESSION(Gt);
IMPLEMENT_BINARY_EXPRESSION(Lt);
IMPLEMENT_BINARY_EXPRESSION(LogicalAnd);
IMPLEMENT_BINARY_EXPRESSION(LogicalOr);

// IMPLEMENT_BINARY_EXPRESSION(Assign);
// IMPLEMENT_BINARY_EXPRESSION(PlusAssign);
// IMPLEMENT_BINARY_EXPRESSION(MinusAssign);
// IMPLEMENT_BINARY_EXPRESSION(MultAssign);
// IMPLEMENT_BINARY_EXPRESSION(DivAssign);
// IMPLEMENT_BINARY_EXPRESSION(ModAssign);
// IMPLEMENT_BINARY_EXPRESSION(PowAssign);
// IMPLEMENT_BINARY_EXPRESSION(AndAssign);
// IMPLEMENT_BINARY_EXPRESSION(OrAssign);
// IMPLEMENT_BINARY_EXPRESSION(ColonAssign);

IMPLEMENT_BINARY_ASSIGN_EXPRESSION(Assign);
IMPLEMENT_BINARY_ASSIGN_EXPRESSION(PlusAssign);
IMPLEMENT_BINARY_ASSIGN_EXPRESSION(MinusAssign);
IMPLEMENT_BINARY_ASSIGN_EXPRESSION(MultAssign);
IMPLEMENT_BINARY_ASSIGN_EXPRESSION(DivAssign);
IMPLEMENT_BINARY_ASSIGN_EXPRESSION(ModAssign);
IMPLEMENT_BINARY_ASSIGN_EXPRESSION(PowAssign);
IMPLEMENT_BINARY_ASSIGN_EXPRESSION(AndAssign);
IMPLEMENT_BINARY_ASSIGN_EXPRESSION(OrAssign);
IMPLEMENT_BINARY_ASSIGN_EXPRESSION(ColonAssign);