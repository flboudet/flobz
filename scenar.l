%{
    #include "scenar.y.h"
    #include <stdlib.h>
    #include <string.h>
    void yyerror(char *);
    int linenum;
%}

%S STRINGg

%%
\n           { linenum++; }

"<--".*"-->" {}

[-]*[0-9]+    { yylval.i = atoi(yytext); return INTEGER; }
<INITIAL>["]  { yylval.str[0]=0; BEGIN STRINGg; }
<STRINGg>["]   { BEGIN INITIAL; return STRING; }
<STRINGg>"\\n" { strcat(yylval.str,"\n"); }
<STRINGg>[a-zA-Z0-9\ \.\:\!\,\'\?]+ { strcat(yylval.str,yytext); }
","          { return COMMA; }

"background" { return BACKGROUND; }
"foreground" { return FOREGROUND; }
"vector"     { return VECTOR;     }
"textarea"   { return TEXTAREA;   }
"say"        { return SAY;  }
"wait"       { return WAIT; }
"sec"        { return SEC;  }
"notextarea" { return NOTEXTAREA; }
"loop"       { return LOOP; }


[ \t]       ;       /* skip whitespace */

.            { return yytext[0]; }

%%

int yywrap(void) {
    return 1;
}
