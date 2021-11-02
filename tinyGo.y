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

%token INT_TYPE FLOAT32_TYPE BOOL_TYPE ID

%token KW_BREAK KW_FUNC KW_PACKAGE KW_CONTINUE KW_FOR KW_IMPORT KW_RETURN KW_VAR 
%token KW_TRUE KW_FALSE
%token KW_IF KW_ELSE


%token PLUS_EQUAL MINUS_EQUAL MULT_EQUAL DIV_EQUAL MOD_EQUAL POW_EQUAL AND_EQUAL OR_EQUAL COLON_EQUAL
%token INCREASE DECREASE
%token OR AND

%token GREATER_OR_EQUAL LESS_OR_EQUAL EQUAL NOT_EQUAL 



%%

expressionlist: expressionlist expression
    | /* epsilon */
    ;

expression: expression '+' factor
    | expression '-' factor 
    | factor 
    ;

factor: factor '*' term 
    | factor '/' term 
    | term 
    ;

term: '|' term 
    | '(' expression ')' 
    | INT_CONSTANT 
    ;

%%
