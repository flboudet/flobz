/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INTEGER = 258,
     STRING = 259,
     BACKGROUND = 260,
     VECTOR = 261,
     FOREGROUND = 262,
     MUSIC = 263,
     WAIT = 264,
     SEC = 265,
     LOOP = 266,
     TEXTAREA = 267,
     NOTEXTAREA = 268,
     COMMA = 269,
     SAY = 270
   };
#endif
#define INTEGER 258
#define STRING 259
#define BACKGROUND 260
#define VECTOR 261
#define FOREGROUND 262
#define MUSIC 263
#define WAIT 264
#define SEC 265
#define LOOP 266
#define TEXTAREA 267
#define NOTEXTAREA 268
#define COMMA 269
#define SAY 270




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 116 "scenar.y"
typedef union YYSTYPE {
    int i;
    char str[1024];
} YYSTYPE;
/* Line 1240 of yacc.c.  */
#line 71 "scenar.y.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



