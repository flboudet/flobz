typedef union {
    int i;
    char str[1024];
} YYSTYPE;
#define	INTEGER	257
#define	STRING	258
#define	BACKGROUND	259
#define	VECTOR	260
#define	FOREGROUND	261
#define	MUSIC	262
#define	WAIT	263
#define	SEC	264
#define	LOOP	265
#define	TEXTAREA	266
#define	NOTEXTAREA	267
#define	COMMA	268
#define	SAY	269


extern YYSTYPE yylval;
