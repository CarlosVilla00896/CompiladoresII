%option noyywrap
%option yylineno
%option caseless
%x comment

%{
    #include <stdio.h>
    #include "tokens.h"
%}

DIGIT [0-9]

%%


[\t \r \n] { /* nada */ }
"/*" {BEGIN(comment);}
"//"[^\n]* {/* nada */}



"break" { return KW_BREAK; }
"func" { return KW_FUNC; }
"if" { return KW_IF; }
"else" { return KW_ELSE; }
"package" { return KW_PACKAGE; }
"continue" { return KW_CONTINUE; }
"for" { return KW_FOR; }
"import" { return KW_IMPORT; }
"return" { return KW_RETURN; }
"var" { return KW_VAR; }
"true" { return KW_TRUE; }
"false" { return KW_FALSE; }
"fmt.Println" { return KW_PRINTLN; }


"int" { return INT_TYPE; }
"float32" { return FLOAT32_TYPE; }
"bool" { return BOOL_TYPE; }


\".*\" { yylval.string_t = strdup(yytext); return STRING_LITERAL; }
{DIGIT}+ {  yylval.int_t = atoi(yytext); return INT_CONSTANT; }
{DIGIT}+"."{DIGIT}+ {  yylval.float_t = atof(yytext); return FLOAT_CONSTANT; }
[a-zA-Z]({DIGIT}|[a-zA-Z_0-9]|_)* { yylval.string_t = strdup(yytext); return ID; }

"+=" { return PLUS_EQUAL; }
"-=" { return MINUS_EQUAL; }
"++" { return TK_INCREASE; }
"--" { return TK_DECREASE; }
"||" { return OR; }
"&&" { return AND; }
"&=" { return AND_EQUAL; }
"|=" { return OR_EQUAL; }
"*=" { return MULT_EQUAL; }
"^=" { return POW_EQUAL; }
"/=" { return DIV_EQUAL; }
":=" { return COLON_EQUAL; }
"%=" { return MOD_EQUAL; }
"==" { return TK_EQUAL; }
"!=" { return NOT_EQUAL; }
">=" { return GREATER_OR_EQUAL; }
"<=" { return LESS_OR_EQUAL; }

"!" {return TK_NOT; }
";" {return ';'; }
"," {return ','; }
":" {return ':'; }
"+" {return '+'; }
"-" {return '-'; }
"/" {return '/'; }
"*" {return '*'; }
"%" {return '%'; }
"^" {return '^'; }
"=" {return '='; }
"[" { return '['; }
"]" { return ']'; }
"{" { return '{'; }
"}" { return '}'; }
">" { return '>'; }
"<" { return '<'; }
"(" { return '('; }
")" { return ')'; }
"." { return '.'; }

[\n] { return EOL; }

. {printf("caracter '%c' invalido en la linea %d\n", yytext[0], yylineno);}

<comment>"*/" {BEGIN(INITIAL);}
<comment>.|\n {/*nada*/}
<comment><<EOF>> {printf("comentario incompleto en la linea %d\n", yylineno); return 0;}

%%