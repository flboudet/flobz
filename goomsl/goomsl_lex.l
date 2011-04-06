%{

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "goomsl.h"
#include "goomsl_private.h"
// xcode and automake have different ways
// to prefix yacc files. I have not found
// a proper way to generate it.
#ifdef XCODE_STYLE_YACC
#include "goomsl_yacc.tab.h"
#else
#include "goomsl_yacc.h"
#endif

void yyerror(char *);
void yyparse(void);

GoomSL *currentGoomSL;
static int  string_size;
static char string[1024];

    
#define YY_INPUT(buf,result,max_size) { \
    result = currentGoomSL->read_file_function(currentGoomSL, buf, currentGoomSL->current_file, max_size); \
    }\
    \
    
#define UP_INCLUDE_STACK() \
    if (yy_buffer_stack_top == 0) {\
        yyterminate(); \
    } \
    else { \
    if (currentGoomSL->current_file != NULL) { \
    currentGoomSL->close_file_function(currentGoomSL, currentGoomSL->current_file); \
    gsl_forget_buffer_input_file(currentGoomSL, YY_CURRENT_BUFFER); \
    } \
    yypop_buffer_state(); \
    currentGoomSL->current_file = gsl_get_buffer_input_file(currentGoomSL, YY_CURRENT_BUFFER); \
    }


%}

DIGIT    [0-9]
XDIGIT   [0-9a-f]
ID       [a-zA-Z_@&][a-zA-Z0-9_\.]*

%S C_COMMENT
%S LINE_COMMENT
%S STRING
%S IMPORT

%%

<LINE_COMMENT,C_COMMENT,INITIAL>^[ \t]*\n           { ++currentGoomSL->num_lines; /* Ignore empty lines */ }
<LINE_COMMENT,C_COMMENT,INITIAL>^[ \t]*"//"[^\n]*\n { ++currentGoomSL->num_lines; /* Ignore empty lines */ }

<LINE_COMMENT>\n    { ++currentGoomSL->num_lines; yylval.charValue=*yytext; BEGIN INITIAL; return '\n'; }
<INITIAL>\n         { ++currentGoomSL->num_lines; yylval.charValue=*yytext; return '\n'; }

<C_COMMENT>"*/"                { BEGIN INITIAL; }
<C_COMMENT>\n                  { ++currentGoomSL->num_lines; }
<C_COMMENT,LINE_COMMENT>.      { /* eat up comment */ }

"import"        {  BEGIN IMPORT; }
<IMPORT>[ \t]*  ;
<IMPORT>\<[^<>\n]+\> {
        goomsl_file *newfile = NULL;
        char* fullName = NULL;
        YY_BUFFER_STATE b;
        yytext[strlen(yytext)-1] = 0;

        /* TODO: build the absolute path name */
        fullName = gsl_create_full_filepath( currentGoomSL, (char*) yytext+1 );
        if ( fullName == NULL )
        {
           goto endImport;
        }

        /* open the imported file */
        newfile = currentGoomSL->open_file_function(currentGoomSL, fullName);
        if ( ! newfile )
        {
           fprintf(stderr, "Warning: file not found %s, ignored\n", fullName);
           goto endImport;
        }

        /* free the imported file after use */
        if ( fullName != NULL )
        {
           free( fullName );
        }
        //GPI_LineNumber = (int*)(META_GTI_LineNumber+includeStackPtr);
        //(*GPI_LineNumber) = 1;
        currentGoomSL->current_file = newfile;

        /* changement de buffer d'entree pour le bovin */
        b = yy_create_buffer( NULL, YY_BUF_SIZE );
        gsl_set_buffer_input_file(currentGoomSL, b, newfile);
        yypush_buffer_state(b);

        endImport:
        BEGIN(INITIAL);
    }

<INITIAL>"#RST_LINE#"             { currentGoomSL->num_lines = 0; }
<INITIAL>"#FILE ".*"#"            { currentGoomSL->num_lines = 0; }
<INITIAL>"#"[^\n]*                { /* ignore preprocessor lines */ }

<INITIAL>"/*"                { BEGIN C_COMMENT; }
<INITIAL>"//"                { BEGIN LINE_COMMENT; }
<INITIAL>\"                  { BEGIN STRING; string_size=0; }

<STRING>"\\\\"               { string[string_size++] = '\\'; }
<STRING>"\\n"                { string[string_size++] = '\n'; }
<STRING>"\\\""               { string[string_size++] = '\"'; }
<STRING>\"                   { /* fin de la chaine: on cree le pointeur qui va bien */
                               unsigned int tmp;
                               BEGIN INITIAL;
                               string[string_size]=0;
                               tmp = gsl_malloc(currentGoomSL, string_size+1);
                               strcpy((char*)currentGoomSL->ptrArray[tmp],string);
                               sprintf(yylval.strValue, "0x%08x", tmp);
                               return LTYPE_PTR;
                             }
<STRING>.                    { string[string_size++] = *yytext; }

<INITIAL>"float"                 { return FLOAT_TK; }
<INITIAL>"int"                   { return INT_TK;   }
<INITIAL>"boolean"               { return INT_TK;   }
<INITIAL>"ptr"                   { return PTR_TK;   }
<INITIAL>"string"                { return PTR_TK;   }
<INITIAL>"declare"               { return DECLARE;  }
<INITIAL>"external"              { return EXTERNAL; }
<INITIAL>"struct"                { return STRUCT;   }
<INITIAL>"not"                   { return NOT;      }
<INITIAL>"while"                 { return WHILE;    }
<INITIAL>"do"                    { return DO;       }
<INITIAL>"for"                   { return FOR;      }
<INITIAL>"in"                    { return IN;       }
<INITIAL>"true"                  { strncpy(yylval.strValue, "1", 2047);    return LTYPE_INTEGER; }
<INITIAL>"false"                 { strncpy(yylval.strValue, "0", 2047);    return LTYPE_INTEGER; }
<INITIAL>{ID}                    { strncpy(yylval.strValue, yytext, 2047); return LTYPE_VAR;     }
<INITIAL>{DIGIT}+                { strncpy(yylval.strValue, yytext, 2047); return LTYPE_INTEGER; }
<INITIAL>\'.\'                   { sprintf(yylval.strValue, "%d", (int)yytext[1]); return LTYPE_INTEGER; }
<INITIAL>"0x"{XDIGIT}+           { strncpy(yylval.strValue, yytext, 2047); return LTYPE_INTEGER; }
<INITIAL>{DIGIT}+"."{DIGIT}*     { strncpy(yylval.strValue, yytext, 2047); return LTYPE_FLOAT;   }
<INITIAL>{DIGIT}+"%"             { sprintf(yylval.strValue, "%3.2f", atof(yytext)/100.0f); return LTYPE_FLOAT; }
<INITIAL>"+="                    { return PLUS_EQ; }
<INITIAL>"*="                    { return MUL_EQ; }
<INITIAL>"-="                    { return SUB_EQ; }
<INITIAL>"/="                    { return DIV_EQ; }
<INITIAL>"<="                    { return LOW_EQ;  }
<INITIAL>">="                    { return SUP_EQ;  }
<INITIAL>"!="                    { return NOT_EQ;  }
<INITIAL>"<>"                    { return NOT_EQ;  }
<INITIAL>[ \t]+                  /* eat up whitespace */

<<EOF>> { UP_INCLUDE_STACK(); }

<INITIAL>.                       { yylval.charValue = *yytext; return *yytext;    }

%%


int yywrap(void) { return 1; yyunput(0,0); }

