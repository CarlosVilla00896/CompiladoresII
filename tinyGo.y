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

package_declaration: KW_PACKAGE ID import_declaration
    ;

import_declaration : import_declaration KW_IMPORT STRING_LITERAL
    | KW_IMPORT STRING_LITERAL program 
    ;

program: program external_declarations
    | external_declarations
    ;

external_declarations: function_definition
    | var_declaration
    ;

function_definition: KW_FUNC ID '(' parameters_list ')' '(' return_type_list ')' block_statement
    | KW_FUNC ID '(' parameters_list ')' '[' ']'type block_statement
    | KW_FUNC ID '(' parameters_list ')' type block_statement
    | KW_FUNC ID '(' parameters_list ')'  block_statement
    | KW_FUNC ID '(' ')'  block_statement
    ;

parameters_list: parameters_list ',' parameter_declaration
    | parameter_declaration
    ;

parameter_declaration: ID type
    | ID '[' ']' type 
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

var_declaration: KW_VAR id_list type 
    | KW_VAR id_list type '=' initializer
    | KW_VAR id_list '=' initializer
    ;

id_list: id_list ',' logical_or_expression
    | logical_or_expression
    ;

initializer: initializer ',' logical_or_expression
    |'[' logical_or_expression ']' type '{' array_values '}'
    |'[' ']' type '{' array_values '}'
    | logical_or_expression
    ;

array_values: array_values ',' logical_or_expression
    | logical_or_expression
    ;

block_statement: '{' statement_list '}'
    | '{' '}'
    ;

statement_list: statement_list statement
    | statement
    ;

statement: if_statement 
    | for_statement
    | return_statement
    | jump_statement
    | expression_statement
    | external_declarations
    ;

if_statement: KW_IF expression_list block_statement 
    | KW_IF expression_list block_statement KW_ELSE block_statement
    | KW_IF expression_list block_statement KW_ELSE if_statement
    ;

expression_list: expression_list ';' expression
    | expression
    ;

for_statement: KW_FOR for_clause block_statement
    ;

for_clause: expression ';' expression ';' expression
    | expression
    |
    ;

return_statement: KW_RETURN return_expression_list
    ;

return_expression_list: return_expression_list ',' logical_or_expression
    | logical_or_expression
    ;

jump_statement: KW_CONTINUE 
    | KW_BREAK
    ;

expression_statement: expression
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

logical_or_expression: logical_or_expression OR logical_and_expression
    | logical_and_expression
    ;

logical_and_expression: logical_and_expression AND equality_expression
    | equality_expression
    ;

equality_expression: equality_expression EQUAL relational_expression
    | equality_expression  NOT_EQUAL relational_expression
    | relational_expression
    ;
    
relational_expression: relational_expression '>' term_expression 
    | relational_expression '<' term_expression 
    | relational_expression GREATER_OR_EQUAL term_expression 
    | relational_expression LESS_OR_EQUAL term_expression 
    | term_expression
    ;

term_expression: term_expression '+' factor_expression
    | term_expression '-' factor_expression
    | factor_expression
    ;

factor_expression: factor_expression '*' pow_expression
    | factor_expression '/' pow_expression
    | factor_expression '%' pow_expression
    | pow_expression
    ;

pow_expression: pow_expression '^' unary_expression
    | unary_expression
    ;

unary_expression: INCREASE unary_expression 
    | DECREASE unary_expression 
    | NOT unary_expression  
    | postfix_expression 
    ;

postfix_expression: postfix_expression INCREASE 
    | postfix_expression DECREASE 
    | postfix_expression '(' argument_expression_list ')'
    | postfix_expression '(' ')' 
    | postfix_expression '[' expression ']'
    | primary_expression
    ;

argument_expression_list: logical_or_expression ',' logical_or_expression 
    | logical_or_expression
    ; 

primary_expression: FLOAT_CONSTANT
    | ID
    | '(' expression ')'
    | ID '.'primary_expression
    | INT_CONSTANT
    | KW_TRUE
    | KW_FALSE
    | STRING_LITERAL
    ;

%%
