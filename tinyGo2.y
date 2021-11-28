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
%}

%union{
    const char * string_t;
    int int_t;
    float float_t;
}

%token <string_t> STRING_LITERAL
%token <int_t> INT_CONSTANT
%token <float_t> FLOAT_CONSTANT

%token INT_TYPE FLOAT32_TYPE BOOL_TYPE  STRING_TYPE ID

%token KW_BREAK KW_FUNC KW_PACKAGE KW_CONTINUE KW_FOR KW_IMPORT KW_RETURN KW_VAR 
%token KW_TRUE KW_FALSE
%token KW_IF KW_ELSE

%token EOL

%token PLUS_EQUAL MINUS_EQUAL MULT_EQUAL DIV_EQUAL MOD_EQUAL POW_EQUAL AND_EQUAL OR_EQUAL COLON_EQUAL
%token INCREASE DECREASE
%token OR AND NOT

%token GREATER_OR_EQUAL LESS_OR_EQUAL EQUAL NOT_EQUAL



%%


start: package_declaration
    ;

package_declaration : KW_PACKAGE ID import_declaration
    ;

import_declaration : import_declaration KW_IMPORT '"' ID '"' EOL
    | KW_IMPORT '"' ID '"' EOL program
    ;

program: program external_declarations
    | external_declarations

external_declarations: function_definition
    | var_declaration
    ;

function_definition: KW_FUNC ID '(' parameters_list ')' '(' return_type_list ')' block_statement
    | KW_FUNC ID '(' parameters_list ')' '[' ']'type block_statement
    | KW_FUNC ID '(' parameters_list ')' type block_statement
    | KW_FUNC ID '(' parameters_list ')'  block_statement
    | KW_FUNC ID '(' ')'  block_statement
    ;

type: INT_TYPE
    | BOOL_TYPE
    | FLOAT32_TYPE
    | STRING_TYPE
    ;

return_type_list: return_type_list ',' type
    | type
    | '[' ']' type
    ;

parameters_list: parameters_list ',' parameter_declaration
    | parameter_declaration
    ;

parameter_declaration: ID type
    | ID '[' ']' type 
    ;

var_declaration: KW_VAR id_list type 
    | KW_VAR id_list type '=' initializer
    ;

id_list: id_list ',' id_declarator
    | id_declarator
    ;

id_declarator: ID
    | ID '[' expression ']'
    ;

initializer: initializer ',' expression
    | expression
    ;

block_statement: '{' statement_list '}'
    | '{' external_declarations statement_list '}'
    | '{' '}'
    ;

statement_list: statement_list statement
    | statement
    ;

statement: if_statement
    | for_statement
    | expression_statement
    | return_statement
    | jump_statement    
    ;

if_statement:
    ;

for_statement:
    ;

expression_statement: expression
    ;

return_statement:
    ;

jump_statement:
    ;

expression: assignment_expression
    ;

assignment_expression: id_list assignment_operator initializer
    | logical_or_expression
    ;

assignment_operator: '='
    | PLUS_EQUAL
    | MINUS_EQUAL
    | AND_EQUAL
    | OR_EQUAL
    | MULT_EQUAL
    | POW_EQUAL
    | DIV_EQUAL
    | COLON_EQUAL
    | MOD_EQUAL
    ;


logical_or_expression: FLOAT_CONSTANT
    | ID
    | '(' expression ')'
    | ID '.'logical_or_expression
    | INT_CONSTANT
    | KW_TRUE
    | KW_FALSE
    | STRING_LITERAL
    ;
%%
