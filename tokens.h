/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_TOKENS_H_INCLUDED
# define YY_YY_TOKENS_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    STRING_LITERAL = 258,
    INT_CONSTANT = 259,
    FLOAT_CONSTANT = 260,
    INT_TYPE = 261,
    FLOAT32_TYPE = 262,
    BOOL_TYPE = 263,
    STRING_TYPE = 264,
    ID = 265,
    KW_BREAK = 266,
    KW_FUNC = 267,
    KW_PACKAGE = 268,
    KW_CONTINUE = 269,
    KW_FOR = 270,
    KW_IMPORT = 271,
    KW_RETURN = 272,
    KW_VAR = 273,
    KW_TRUE = 274,
    KW_FALSE = 275,
    KW_IF = 276,
    KW_ELSE = 277,
    EOL = 278,
    PLUS_EQUAL = 279,
    MINUS_EQUAL = 280,
    MULT_EQUAL = 281,
    DIV_EQUAL = 282,
    MOD_EQUAL = 283,
    POW_EQUAL = 284,
    AND_EQUAL = 285,
    OR_EQUAL = 286,
    COLON_EQUAL = 287,
    INCREASE = 288,
    DECREASE = 289,
    OR = 290,
    AND = 291,
    NOT = 292,
    GREATER_OR_EQUAL = 293,
    LESS_OR_EQUAL = 294,
    EQUAL = 295,
    NOT_EQUAL = 296
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 15 "tinyGo.y"

    const char * string_t;
    int int_t;
    float float_t;

#line 105 "tokens.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_TOKENS_H_INCLUDED  */
