#ifndef BISON_SCENAR_Y_H
# define BISON_SCENAR_Y_H

#ifndef YYSTYPE
typedef union {
    int i;
    char str[1024];
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	INTEGER	257
# define	STRING	258
# define	BACKGROUND	259
# define	VECTOR	260
# define	FOREGROUND	261
# define	MUSIC	262
# define	WAIT	263
# define	SEC	264
# define	LOOP	265
# define	TEXTAREA	266
# define	NOTEXTAREA	267
# define	COMMA	268
# define	SAY	269


extern YYSTYPE yylval;

#endif /* not BISON_SCENAR_Y_H */
