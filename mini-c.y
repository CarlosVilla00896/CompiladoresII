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


%token PLUS_EQUAL MINUS_EQUAL MULT_EQUAL DIV_EQUAL MOD_EQUAL POW_EQUAL AND_EQUAL OR_EQUAL COLON_EQUAL
%token INCREASE DECREASE
%token OR AND NOT

%token GREATER_OR_EQUAL LESS_OR_EQUAL EQUAL NOT_EQUAL



%%


start: input

input: input external_declaration
    | external_declaration
    ;

external_declaration: method_definition
            | declaration 
            ;

method_definition: type ID '(' parameters_type_list ')' block_statement 
                 | type ID '(' ')' block_statement
                 | type ID '(' parameters_type_list ')' ';'
                 | type ID '(' ')' block_statement ';'
                ;

declaration_list: declaration_list declaration
                | declaration
                ;

declaration: type init_declarator_list ';'
           ;

init_declarator_list: init_declarator_list ',' init_declarator
                | init_declarator
                ;

init_declarator: declarator
                | declarator '=' initializer
                ;

declarator: ID 
          | ID '[' assignment_expression ']'
          | ID '[' ']'
          ;

parameters_type_list: parameters_type_list ',' parameter_declaration 
                   | parameter_declaration
                   ;

parameter_declaration: type declarator
                     | type
                     | type '[' ']' 
                    ;

initializer: assignment_expression 
           | '{' initializer_list '}'
           ;

initializer_list: initializer_list ',' logical_or_expression
                | logical_or_expression 
                ;

statement: for_statement
        | expression_statement
        | if_statement 
        | jump_statement 
        ;

statement_list: statement_list statement 
              | statement 
              ;

if_statement: KW_IF '(' expression ')' statement 
            | KW_IF '(' expression ')' statement KW_ELSE statement 
            ;

expression_statement: expression ';' 
                    ;

for_statement: KW_FOR '(' expression ')' statement 
               ;

jump_statement: KW_RETURN expression ';' 
              ;

block_statement: '{' statement_list '}' 
               | '{' declaration_list  statement_list'}'
               | '{' '}' 
               ;

type: INT_TYPE 
    | FLOAT32_TYPE
    | BOOL_TYPE
    | STRING_TYPE
    ;

primary_expression: '(' expression ')' 
    | ID 
    | constant
    | STRING_LITERAL 
    ;

assignment_expression: unary_expression assignment_operator assignment_expression
                     | logical_or_expression
                     ;

postfix_expression: primary_expression 
                    | postfix_expression '[' expression ']' 
                    | postfix_expression '(' ')' 
                    | postfix_expression '(' argument_expression_list ')' 
                    | postfix_expression INCREASE 
                    | postfix_expression DECREASE 
                    ;


argument_expression_list: argument_expression_list ',' assignment_expression 
                        | assignment_expression 
                        ;

unary_expression: INCREASE unary_expression 
                | DECREASE unary_expression 
                | NOT unary_expression  
                | postfix_expression 
                ;

multiplicative_expression: multiplicative_expression '*' unary_expression 
      | multiplicative_expression '/' unary_expression 
      | unary_expression 
      ;

additive_expression:  additive_expression '+' multiplicative_expression
                    | additive_expression '-' multiplicative_expression 
                    | multiplicative_expression 
                    ;

relational_expression: relational_expression '>' additive_expression 
                     | relational_expression '<' additive_expression 
                     | relational_expression GREATER_OR_EQUAL additive_expression 
                     | relational_expression LESS_OR_EQUAL additive_expression 
                     | additive_expression 
                     ;

equality_expression:  equality_expression EQUAL relational_expression 
                   | equality_expression NOT_EQUAL relational_expression
                   | relational_expression
                   ;

logical_or_expression: logical_or_expression OR logical_and_expression
                    | logical_and_expression 
                    ;

logical_and_expression: logical_and_expression AND equality_expression 
                      | equality_expression 
                      ;

assignment_operator: '=' 
                   | PLUS_EQUAL 
                   | MINUS_EQUAL 
                   ;

expression: assignment_expression 
          ;

constant: INT_CONSTANT 
        | FLOAT_CONSTANT
        ;
%%
