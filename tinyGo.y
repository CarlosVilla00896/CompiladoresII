%code requires{
    #include "ast.h"
}

%{
//https://golang.org/ref/spec
    #include <cstdio>
    using namespace std;
    int yylex();
    extern int yylineno;
    void yyerror(const char * s){
        fprintf(stderr, "Line: %d, error: %s\n", yylineno, s);
    }

    #define YYERROR_VERBOSE 1
    #define YYDEBUG 1
    #define EQUAL 1
    #define PLUSEQUAL 2
    #define MINUSEQUAL 3
    #define MULTEQUAL 4
    #define DIVEQUAL 5
    #define MODEQUAL 6
    #define POWEQUAL 7
    #define ANDEQUAL 8
    #define OREQUAL 9
    #define COLONEQUAL 10
    #define KWBREAK 11
    #define KWCONTINUE 12
    
%}

%union{
    const char * string_t;
    int int_t;
    float float_t;
    bool bool_t;

    Expression * expression_t;
    ExpressionList * expression_list_t;
    Statement * statement_t;
    StatementList * statement_list_t;
    Parameter * param_t;
    ParametersList * params_list_t;
    Initializer * initializer_t;
    InitializerList * initializer_list_t;
    Declarator * declarator_t;
    DeclaratorList * declarator_list_t;
    DeclarationList * declaration_list_t;
    VarDeclaration * var_declaration_t;
    TypeList * type_list_t;
    ArgumentsList * argument_list_t;
    ReturnExpressionList * return_expression_list_t;

}

%token <string_t> STRING_LITERAL ID
%token <int_t> INT_CONSTANT
%token <float_t> FLOAT_CONSTANT
%token <bool_t> KW_TRUE KW_FALSE

%token INT_TYPE FLOAT32_TYPE BOOL_TYPE

%token KW_BREAK KW_FUNC KW_PACKAGE KW_CONTINUE KW_FOR KW_IMPORT KW_RETURN KW_VAR 
%token KW_IF KW_ELSE

%token EOL

%token PLUS_EQUAL MINUS_EQUAL MULT_EQUAL DIV_EQUAL MOD_EQUAL POW_EQUAL AND_EQUAL OR_EQUAL COLON_EQUAL
%token TK_INCREASE TK_DECREASE
%token OR AND TK_NOT

%token GREATER_OR_EQUAL LESS_OR_EQUAL TK_EQUAL NOT_EQUAL

%type<statement_list_t> package_declaration import_declaration program statement_list
%type<statement_t> external_declarations function_definition block_statement statement
%type<var_declaration_t> var_declaration
%type<expression_t> expression assignment_expression logical_or_expression logical_and_expression equality_expression relational_expression 
%type<expression_t> term_expression factor_expression pow_expression unary_expression postfix_expression primary_expression
%type<params_list_t> parameters_list
%type<param_t> parameter_declaration
%type<int_t> type assignment_operator
%type<type_list_t> return_type_list
%type<declarator_list_t> id_list
%type<declarator_t> declarator
%type<initializer_list_t> initializer
%type<expression_list_t> array_values assignment_list
%type<return_expression_list_t> return_expression_list
%type<statement_t> if_statement for_statement return_statement jump_statement expression_statement
%type<argument_list_t> argument_expression_list

%%

start: package_declaration { 
    list<Statement *>::iterator it = $1->begin();
    while(it != $1->end()){
        printf("semantic result: %d \n",(*it)->evaluateSemantic());
        it++;
    }
}
    ;

package_declaration: KW_PACKAGE ID import_declaration { $$ = $3; }
    ;

import_declaration : KW_IMPORT STRING_LITERAL import_declaration  { $$ = $3; }
    | KW_IMPORT STRING_LITERAL program  { $$ = $3;}
    ;

program: program external_declarations {$$ = $1; $$->push_back($2);}
    | external_declarations {$$ = new StatementList; $$->push_back($1);}
    ;

external_declarations: function_definition { $$ = $1; }
    | var_declaration { $$ = new GlobalDeclaration($1); }
    ;

function_definition: KW_FUNC ID '(' parameters_list ')' '(' return_type_list ')' block_statement { 
        $$ = new FunctionDefinition($2, *$4, NULLTYPE, *$7, "lista", $9, yylineno );
    }
    | KW_FUNC ID '(' parameters_list ')' '[' ']'type block_statement{
        TypeList * typeList = new TypeList;
        $$ = new FunctionDefinition($2, *$4, (Type)$8, *typeList, "tipo", $9, yylineno);
        delete typeList;
    }
    | KW_FUNC ID '(' parameters_list ')' type block_statement{
        TypeList * typeList = new TypeList;
        $$ = new FunctionDefinition($2, *$4, (Type)$6, *typeList, "tipo", $7, yylineno);
        delete typeList;
    }
    | KW_FUNC ID '(' parameters_list ')'  block_statement{
        TypeList * typeList = new TypeList;
        $$ = new FunctionDefinition($2, *$4, VOID, *typeList, "nada", $6, yylineno);
        delete typeList;
    }
    | KW_FUNC ID '(' ')' type block_statement{
        ParametersList * pm = new ParametersList();
        TypeList * typeList = new TypeList;
        $$ = new FunctionDefinition($2, *pm, (Type)$5, *typeList, "nada", $6, yylineno);
        delete typeList;
        delete pm;
    }
    | KW_FUNC ID '(' ')'  block_statement{
        ParametersList * pm = new ParametersList();
        TypeList * typeList = new TypeList;
        $$ = new FunctionDefinition($2, *pm, VOID, *typeList, "nada", $5, yylineno);
        delete typeList;
        delete pm;
    }
    ;

parameters_list: parameters_list ',' parameter_declaration { $$ = $1; $$->push_back($3); }
    | parameter_declaration { $$ = new ParametersList(); $$->push_back($1); }
    ;

parameter_declaration: ID type { $$ = new Parameter($1, (Type)$2, false, yylineno); }
    | ID '[' ']' type  { $$ = new Parameter($1, (Type)$4, true, yylineno); }
    ;

type: INT_TYPE { $$ = INT; }
    | BOOL_TYPE { $$ = BOOL; }
    | FLOAT32_TYPE { $$ = FLOAT32; }
    ;

return_type_list: return_type_list ',' type { $$ = $1; $$->push_back((Type)$3); }
    | type { $$ = new TypeList; $$->push_back((Type)$1); }
    | '[' ']' type { $$ = new TypeList; $$->push_back((Type)$3); }
    ;

var_declaration: KW_VAR id_list type { 
        InitializerList * initializerList = new InitializerList; 
        $$ = new VarDeclaration(*$2, *initializerList, (Type)$3, yylineno ); 
    }
    | KW_VAR id_list type '=' initializer { $$ = new VarDeclaration(*$2, *$5, (Type)$3, yylineno ); }
    | KW_VAR id_list '=' initializer { $$ = new VarDeclaration(*$2, *$4, NULLTYPE, yylineno ); }
    ;

id_list: id_list ',' declarator { $$ = $1; $$->push_back($3); }
    | declarator { $$ = new DeclaratorList; $$->push_back($1); }
    ;

declarator: ID { $$ = new Declarator($1, NULL, false, yylineno); }
    | ID '[' logical_or_expression ']' { $$ = new Declarator($1, $3, true, yylineno); }
    ;
    /* Regresar el InitializerList a una lista de Expressions y hacer que estas expressiones de arreglos y todo vayan al postfix o algo, solo dejar logical_or */
    
initializer: initializer ',' logical_or_expression { 
        $$ = $1; 
        ExpressionList * expressionList = new ExpressionList();
        $$->push_back(new Initializer( $3, *expressionList, false, NULLTYPE, yylineno ));
        delete expressionList; 
    }
    |'[' logical_or_expression ']' type '{' array_values '}' {
         $$ = new InitializerList; 
         $$->push_back( new Initializer( $2, *$6, true, (Type)$4, yylineno ) );
    }
    |'[' ']' type '{' array_values '}' {
        $$ = new InitializerList; 
        $$->push_back( new Initializer( NULL, *$5, true, (Type)$3, yylineno ) );
    }
    | logical_or_expression{
        $$ = new InitializerList;
        ExpressionList * expressionList = new ExpressionList(); 
        $$->push_back( new Initializer( $1, *expressionList, false, NULLTYPE, yylineno ) );
        delete expressionList;
    }
    ;

array_values: array_values ',' logical_or_expression { $$ = $1; $$->push_back($3); }
    | logical_or_expression { $$ = new ExpressionList; $$->push_back($1); }
    ;

block_statement: '{' statement_list '}' { $$ = new BlockStatement( *$2, yylineno); }
    | '{' '}' { StatementList * statementList = new StatementList();  $$ = new BlockStatement(*statementList, yylineno); }
    ;

statement_list: statement_list statement { $$ = $1; $$->push_back($2); }
    | statement { $$ = new StatementList; $$->push_back($1); }
    ;

statement: if_statement {$$ = $1; }
    | for_statement {$$ = $1; }
    | return_statement {$$ = $1; }
    | jump_statement {$$ = $1; }
    | expression_statement {$$ = $1; }
    | function_definition {$$ = $1; }
    | var_declaration {$$ = $1; }
    ;

if_statement: KW_IF expression ';' logical_or_expression block_statement { $$ = new IfStatement($2, $4, $5, yylineno); }
    | KW_IF expression block_statement { $$ = new IfStatement(NULL, $2, $3, yylineno); }
    | KW_IF expression ';' logical_or_expression block_statement KW_ELSE block_statement { $$ = new ElseStatement($2, $4, $5, $7, yylineno); } 
    | KW_IF expression block_statement KW_ELSE block_statement { $$ = new ElseStatement(NULL, $2, $3, $5, yylineno); }
    | KW_IF expression ';' logical_or_expression block_statement KW_ELSE if_statement { $$ = new ElseStatement($2, $4, $5, $7, yylineno); }
    | KW_IF expression block_statement KW_ELSE if_statement { $$ = new ElseStatement(NULL, $2, $3, $5, yylineno); }
    ;

for_statement: KW_FOR expression ';' expression ';' expression block_statement { $$ = new ForStatement($2, $4, $6, $7, yylineno);}
    | KW_FOR expression block_statement { $$ = new ForStatement(NULL, $2, NULL, $3, yylineno); }
    | KW_FOR block_statement { $$ = new ForStatement(NULL, NULL, NULL, $2, yylineno); }
    ;

return_statement: KW_RETURN return_expression_list { $$ = new ReturnStatement(*$2, yylineno); }
    ;

return_expression_list: return_expression_list ',' logical_or_expression { $$ = $1; $$->push_back($3); }
    | logical_or_expression { $$ = new ReturnExpressionList ; $$->push_back($1); }
    ;

jump_statement: KW_CONTINUE  { $$ = new JumpStatement(KWCONTINUE, yylineno); }
    | KW_BREAK { $$ = new JumpStatement(KWBREAK, yylineno); }
    ;

expression_statement: expression { $$ = new ExpressionStatement( $1, yylineno ); }
    ;

expression: assignment_expression { $$ = $1; }
    ;
/* El initializer es una expressionlist, hay que recorrerla, cuando se analice un initializer arraeglo que se evalue el type con el type de los array values */
assignment_expression: assignment_list assignment_operator initializer { 
        switch($2){
            case EQUAL:
                $$ = new AssignExpression(*$1, *$3, yylineno);
                break;
            case PLUSEQUAL:
                $$ = new PlusAssignExpression(*$1, *$3, yylineno);
                break;
            case MINUSEQUAL:
                $$ = new MinusAssignExpression(*$1, *$3, yylineno);
                break;
            case MULTEQUAL:
                $$ = new MultAssignExpression(*$1, *$3, yylineno);
                break;
            case DIVEQUAL:
                $$ = new DivAssignExpression(*$1, *$3, yylineno);
                break;
            case MODEQUAL:
                $$ = new ModAssignExpression(*$1, *$3, yylineno);
                break;
            case POWEQUAL:
                $$ = new PowAssignExpression(*$1, *$3, yylineno);
                break;
            case ANDEQUAL:
                $$ = new AndAssignExpression(*$1, *$3, yylineno);
                break;
            case OREQUAL:
                $$ = new OrAssignExpression(*$1, *$3, yylineno);
                break;
            case COLONEQUAL:
                $$ = new ColonAssignExpression(*$1, *$3, yylineno);
                break;
            default:
                $$ = new AssignExpression(*$1, *$3, yylineno);
        }
        
    }
    | logical_or_expression { $$ = $1; }
    ;

assignment_list: assignment_list ',' logical_or_expression {$$ = $1; $$->push_back($3);}
    | logical_or_expression { $$ = new ExpressionList; $$->push_back($1); }
    ;

assignment_operator: '=' { $$ = EQUAL; }
    | PLUS_EQUAL { $$ = PLUSEQUAL; }
    | MINUS_EQUAL { $$ = MINUSEQUAL; }
    | AND_EQUAL { $$ = ANDEQUAL; }
    | OR_EQUAL { $$ = OREQUAL; }
    | MULT_EQUAL { $$ = MULTEQUAL; }
    | POW_EQUAL { $$ = POWEQUAL; }
    | DIV_EQUAL { $$ = DIVEQUAL; }
    | COLON_EQUAL { $$ = COLONEQUAL; }
    | MOD_EQUAL { $$ = MODEQUAL; }
    ;

logical_or_expression: logical_or_expression OR logical_and_expression { $$ = new LogicalOrExpression($1, $3, yylineno); }
    | logical_and_expression { $$ = $1; }
    ;

logical_and_expression: logical_and_expression AND equality_expression { $$ = new LogicalAndExpression($1, $3, yylineno); }
    | equality_expression { $$ = $1; }
    ;

equality_expression: equality_expression TK_EQUAL relational_expression { $$ = new EqualExpression($1, $3, yylineno); }
    | equality_expression  NOT_EQUAL relational_expression { $$ = new NotEqualExpression($1, $3, yylineno); }
    | relational_expression { $$ = $1; }
    ;
    
relational_expression: relational_expression '>' term_expression { $$ = new GtExpression($1, $3, yylineno); }
    | relational_expression '<' term_expression { $$ = new LtExpression($1, $3, yylineno); }
    | relational_expression GREATER_OR_EQUAL term_expression { $$ = new GteExpression($1, $3, yylineno); }
    | relational_expression LESS_OR_EQUAL term_expression  { $$ = new LteExpression($1, $3, yylineno); }
    | term_expression { $$ = $1; }
    ;

term_expression: term_expression '+' factor_expression { $$ = new AddExpression($1, $3, yylineno); }
    | term_expression '-' factor_expression { $$ = new SubExpression($1, $3, yylineno); }
    | factor_expression { $$ = $1; }
    ;

factor_expression: factor_expression '*' pow_expression { $$ = new MultExpression($1, $3, yylineno); }
    | factor_expression '/' pow_expression { $$ = new DivExpression($1, $3, yylineno); }
    | factor_expression '%' pow_expression { $$ = new ModExpression($1, $3, yylineno); }
    | pow_expression { $$ = $1; }
    ;

pow_expression: pow_expression '^' unary_expression { $$ = new PowExpression($1, $3, yylineno); }
    | unary_expression { $$ = $1; }
    ;

unary_expression: TK_INCREASE unary_expression { $$ = new UnaryExpression(INCREMENT, $2, yylineno); } 
    | TK_DECREASE unary_expression  { $$ = new UnaryExpression(DECREMENT, $2, yylineno); } 
    | TK_NOT unary_expression  { $$ = new UnaryExpression(NOT, $2, yylineno); } 
    | postfix_expression {$$ = $1; }
    ;

postfix_expression: postfix_expression TK_INCREASE { $$ = new PostIncrementExpression((IdExpression*)$1, yylineno); }
    | postfix_expression TK_DECREASE { $$ = new PostDecrementExpression((IdExpression*)$1, yylineno); }
    | postfix_expression '(' argument_expression_list ')' { $$ = new FunctionCallExpression((IdExpression*)$1, *$3, yylineno); }
    | postfix_expression '(' ')' { $$ = new FunctionCallExpression((IdExpression*)$1, *(new ArgumentsList), yylineno); }
    | postfix_expression '[' expression ']' { $$ = new ArrayExpression((IdExpression*)$1, $3, yylineno); }
    | postfix_expression '.'primary_expression '(' ')'{ $$ = new FunctionInvocationExpression((IdExpression*)$1, (IdExpression*)$3, *(new ArgumentsList), yylineno); }
    | postfix_expression '.'primary_expression '(' argument_expression_list ')'{ $$ = new FunctionInvocationExpression((IdExpression*)$1, (IdExpression*)$3, *$5, yylineno); }
    | primary_expression { $$ = $1; }
    ;

argument_expression_list: argument_expression_list ',' logical_or_expression {$$ = $1;  $$->push_back($3);}
    | logical_or_expression { $$ = new ArgumentsList; $$->push_back($1);}
    ; 

primary_expression: FLOAT_CONSTANT { $$ = new FloatExpression($1, yylineno); }
    | ID { $$ = new IdExpression($1, yylineno); }
    | '(' expression ')' { $$ = $2; }
    | INT_CONSTANT { $$ = new IntExpression($1, yylineno); }
    | KW_TRUE  { $$ = new BoolExpression(true, yylineno); }
    | KW_FALSE { $$ = new BoolExpression(false, yylineno); }
    | STRING_LITERAL { $$ = new StringExpression($1, yylineno); }
    ;

%%
