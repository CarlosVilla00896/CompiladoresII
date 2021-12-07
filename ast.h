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

typedef list<Declarator *> DeclaratorList;
typedef list<Expression *> InitializerList;


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
    ELSE_STATEMENT
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
        Initializer(InitializerList initializerList, bool isArrayInitializer, Type type, int line){
            this->initializerList = initializerList;
            this->type = type;
            this->isArrayInitializer = isArrayInitializer;
            this->line = line;
        }

        InitializerList initializerList;
        Type type;
        bool isArrayInitializer;
        int line;
};

class Declarator{
    public:
        Declarator(string id, Expression * arrayDeclaration, bool isArray, int line){
            this->id = id;
            this->arrayDeclaration = arrayDeclaration;
            this->isArray = isArray;
            this->line = line;
        }

        string id;
        Expression * arrayDeclaration;
        bool isArray;
        int line;
};

//Lista de las variables de la declaracion
class DeclarationList{
    public:
        DeclarationList(DeclaratorList declarators, int line){
            this->declarators = declarators;
            this->line = line;
        }

        DeclaratorList declarators;
        int line;
};

class VarDeclaration{
    public:
        VarDeclaration(DeclaratorList declaratorsList, Initializer * initializer, Type type, int line ) {
            this->declaratorsList = declaratorsList;
            this->initializer = initializer;
            this->type = type;
        }

        DeclaratorList declaratorsList;
        Initializer * initializer;
        Type type;
        int line;

        int evaluateSemantic();
};


class GlobalDeclaration : public Statement{
    public:
        GlobalDeclaration(VarDeclaration * varDeclaration){
            this->varDeclaration = varDeclaration;
        }

        VarDeclaration * varDeclaration;

        int evaluateSemantic();
        StatementKind getKind(){
            return GLOBAL_DECLARATION_STATEMENT;
        }
};

