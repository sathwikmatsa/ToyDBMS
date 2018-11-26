%{
#include <iostream>
using namespace std;
#include "crud.h"
#include "parser.h"
bool DEBUG = false;
#define YY_DECL static int wrapped_lexer(void)
%}

INTEGER [0-9]+
IDENTIFIER [A-Za-z]+
STRING \"(\\.|[^\\"])*\"

%option noyywrap
%option outfile="lexer.cc"

%%

INSERT|insert               {return insert;}
INTO|into                   {return into;}
VALUES|values               {return values;}
DELETE|delete               {return delete_t;}
AS|as                       {return as;}
WHERE|where                 {return where;}
IN|in                       {return in;}
SELECT|select               {return select_t;}
FROM|from                   {return from;}
AND|and                     {return and_t;}
OR|or                       {return or_t;}
MAX|max                     {return max;}
CREATE|create               {return create;}
TABLE|table                 {return table_t;}
INT|int                     {return int_t;}
VARCHAR|varchar             {return varchar;}
DEFAULT|default             {return def;}
"NOT NULL"|"not null"       {return not_null;}
"FOREIGN KEY"|"foreign key" {return fk;}
"REFERENCES"|"references"   {return ref;}
"ON DELETE"|"on delete"     {return on_del;}
"ON UPDATE"|"on update"     {return on_up;}
"SET NULL"|"set null"       {return set_nul;}
"SET DEFAULT"|"set default" {return set_def;}
"CASCADE"|"cascade"         {return cascade;}
"PRIMARY KEY"|"primary key" {return pk;}
{INTEGER}                   {
                                string* s = new string(yytext);
                                add_to_cleanup(STRING, (void**) &s);
                                yylval.val = s; return literal;
                            }
{STRING}                    {
                                string* s = new string(yytext); string* r = new string(s->substr(1, s->length() - 2)); 
                                add_to_cleanup(STRING, (void**) &s); add_to_cleanup(STRING, (void**) &r); 
                                yylval.val = r; return literal;
                            }
{IDENTIFIER}                {
                                string* s = new string(yytext);
                                yylval.val = s; return id;
                            }

[ \n\t] ;
.                           {return yytext[0];}

%%

int yylex(){
    int token = wrapped_lexer();
    if(DEBUG) cout<<token<<endl;
    return token;
}
