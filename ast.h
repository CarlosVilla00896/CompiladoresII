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


enum StatementKind{
    FOR_STATEMENT,
    IF_STATEMENT,
    EXPRESSION_STATEMENT,
    RETURN_STATEMENT,
    CONTINUE_STATEMENT,
    BREAK_STATEMENT,
    BLOCK_STATEMENT,
    FUNCTION_DEFINITION_STATEMENT,
    VAR_DECLARATION_STATEMENT,
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

class IdList{
    public:
        IdList(){

        }
};
