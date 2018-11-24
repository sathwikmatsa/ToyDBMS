%{
#include <iostream>
#include "crud.h"
using namespace std;
extern FILE* yyin;
int yylex();
int yyerror(string s);
%}

%output "parser.cc"
%defines "parser.h"


%token create table_t int_t varchar def not_null fk ref on_del on_up set_nul set_def cascade pk select from and_t or_t insert into values delete_t max where as in

%left or_t
%left and_t

%union{
    ast_node* ast_node_ptr;
    string* val;
}

%token<val> literal id
/* TODO: add select, delete*/ 
%type<ast_node_ptr> PROGRAM QUERIES QUERY CREATE_TABLE INSERT IDS LITERALS CT_ARGS CT_ARG ATTR_DEF FK_PK_DEF TYPE CONSTRAINTS CONSTRAINT FK_CONSTRAINTS FK_CONSTRAINT  


%%

PROGRAM : QUERIES {$$ = $1; cout<<"VALID!"<<endl; cout<<$$->type<<endl; garbageCollector();}
        ;

QUERIES : QUERY ';'         {$$->childNodes.push_back($1);}
        | QUERY ';' QUERIES {$3->childNodes.push_front($1); $1 = $3;}
        ;

QUERY : CREATE_TABLE {$$ = $1;}
      //| SELECT       {$$ = $1;}
      | INSERT       {$$ = $1;}
      //| DELETE       {$$ = $1;}
      ;

INSERT : insert into id '(' IDS ')' values '(' LITERALS ')'
            {
                $$ = new ast_node();
                $$->type = INSERT;
                $$->tableName = (*$3);
                $$->childNodes.push_back($5);
                $$->childNodes.push_back($9);
                add_to_cleanup(AST_NODE, (void**) &$$);
            }
       | insert into id values '(' LITERALS ')'
            {
                $$ = new ast_node();
                $$->type = INSERT;
                $$->tableName = (*$3);
                $$->childNodes.push_back($6);
                add_to_cleanup(AST_NODE, (void**) &$$);
            }
       ;

LITERALS : literal
            {
                $$ = new ast_node();
                $$->type = LITERALS;
                $$->list_val.push_back((*$1));
                add_to_cleanup(AST_NODE, (void**) &$$);
            }
         | literal ',' LITERALS
            {
                $3->list_val.push_front((*$1));
                $$ = $3;
            }
         ;

/*DELETE : delete_t from id where CONDITION
            {
                $$ = new ast_node();
                $$->type = DELETE;
                $$->tableName = (*$3);
                $$->childNodes.push_back($5);
                add_to_cleanup(AST_NODE, (void**) &$$);
            }
       ;
*/
SELECT : select '*' from id where CONDITION
       | select IDS from id where CONDITION
       | select max '(' id ')' from id where CONDITION
       | select '*' from id
       | select IDS from id
       ;

CONDITION : CONDITION and_t CONDITION
          | CONDITION or_t CONDITION
          | id in NQ_OR_LITERAL
          | id '=' NQ_OR_LITERAL
          | id '!' '=' NQ_OR_LITERAL
          | id '<' NQ_OR_LITERAL
          | id '>' NQ_OR_LITERAL
          | id '<' '=' NQ_OR_LITERAL
          | id '>' '=' NQ_OR_LITERAL
          ;

NQ_OR_LITERAL : literal
              | '(' SELECT ')'
              ;


CREATE_TABLE : create table_t id '(' CT_ARGS ')'
                {
                    $$ = new ast_node();
                    $$->type = CREATE_TABLE;
                    $$->tableName = (*$3);
                    $$->childNodes.push_back($5);
                    add_to_cleanup(AST_NODE, (void**) &$$);
                }
             ;

CT_ARGS : CT_ARG
            {
                $$ = new ast_node();
                $$->type = CT_ARGS;
                $$->childNodes.push_back($1);
                add_to_cleanup(AST_NODE, (void**) &$$);
            }
        | CT_ARG ',' CT_ARGS
            {
                $3->childNodes.push_front($1);
                $$ = $3;
            }
        ;

CT_ARG : ATTR_DEF  { $$ = $1;} 
       | FK_PK_DEF { $$ = $1;}
       ;

ATTR_DEF : id TYPE CONSTRAINTS
            {
                $$ = new ast_node();
                $$->type = ATTR_DEF;
                $$->attrName = (*$1);
                $$->childNodes.push_back($2);
                $$->childNodes.push_back($3);
                add_to_cleanup(AST_NODE, (void**) &$$);
            }
         | id TYPE
            {
                $$ = new ast_node();
                $$->type = ATTR_DEF;
                $$->attrName = (*$1);
                $$->childNodes.push_back($2);
                add_to_cleanup(AST_NODE, (void**) &$$);
            }
         ;

TYPE : int_t
        {
            $$ = new ast_node();
            $$->type = TYPE;
            $$->isString = false;
            add_to_cleanup(AST_NODE, (void**) &$$);
        }
     | varchar '(' literal ')'
        {
            $$ = new ast_node();
            $$->type = TYPE;
            $$->isString = true;
            add_to_cleanup(AST_NODE, (void**) &$$);
        }
     ;

CONSTRAINTS : CONSTRAINT
                {
                    $$ = new ast_node();
                    $$->type = CONSTRAINTS;
                    $$->childNodes.push_back($1);
                    add_to_cleanup(AST_NODE, (void**) &$$);
                }
            | CONSTRAINT CONSTRAINTS
                {
                    $2->childNodes.push_front($1);
                    $$ = $2;
                }
            ;

CONSTRAINT : not_null
                {
                    $$ = new ast_node();
                    $$->type = CONSTRAINT;
                    $$->attrID = 6;
                    add_to_cleanup(AST_NODE, (void**) &$$);
                }
           | def literal
                {
                    $$ = new ast_node();
                    $$->type = CONSTRAINT;
                    $$->attrID = 4;
                    $$->literal_v = (*$2);
                    add_to_cleanup(AST_NODE, (void**) &$$);
                }
           ;

FK_PK_DEF : pk '(' IDS ')'
            {
                $$ = new ast_node();
                $$->type = FK_PK_DEF;
                $$->attrID = 7;
                $$->childNodes.push_back($3);
                add_to_cleanup(AST_NODE, (void**) &$$);
            }
          | fk '(' id ')' ref id '(' id ')' FK_CONSTRAINTS
            {
                $$ = new ast_node();
                $$->type = FK_PK_DEF;
                $$->attrID = 10; // 0 & 1
                $$->attrName = (*$3);
                $$->list_val.push_back((*$6));
                $$->list_val.push_back((*$8));
                $$->childNodes.push_back($10);
                add_to_cleanup(AST_NODE, (void**) &$$);
            }
          ;

IDS : id
        {
            $$ = new ast_node();
            $$->type = IDS;
            $$->list_val.push_back((*$1));
            add_to_cleanup(AST_NODE, (void**) &$$);
        }
    | id ',' IDS
        {
            $3->list_val.push_front((*$1));
            $$ = $3;
        }
    | id as id
        {
            $$ = new ast_node();
            $$->type = IDS;
            $$->list_val.push_back((*$1));
            $$->rename[(*$1)] = (*$3);
            add_to_cleanup(AST_NODE, (void**) &$$);
        }
    | id as id ',' IDS
        {
            $5->list_val.push_front((*$1));
            $5->rename[(*$1)] = (*$3);
            $$ = $5;
        }
    ;

FK_CONSTRAINTS : FK_CONSTRAINT
                    {
                        $$ = new ast_node();
                        $$->type = FK_CONSTRAINTS;
                        $$->childNodes.push_back($1);
                        add_to_cleanup(AST_NODE, (void**) &$$);
                    }
               | FK_CONSTRAINT FK_CONSTRAINTS
                    {
                        $2->childNodes.push_back($1);
                        $$ = $2;
                    }
               ;

FK_CONSTRAINT : on_del cascade
                    {
                        $$ = new ast_node();
                        $$->type = FK_CONSTRAINT;
                        $$->op = 2;
                        $$->op_val = 0;
                        add_to_cleanup(AST_NODE, (void**) &$$);
                    }
              | on_del set_nul
                    {
                        $$ = new ast_node();
                        $$->type = FK_CONSTRAINT;
                        $$->op = 2;
                        $$->op_val = 2;
                        add_to_cleanup(AST_NODE, (void**) &$$);
                    }
              | on_del set_def
                    {
                        $$ = new ast_node();
                        $$->type = FK_CONSTRAINT;
                        $$->op = 2;
                        $$->op_val = 1;
                        add_to_cleanup(AST_NODE, (void**) &$$);
                    }
              | on_up cascade
                    {
                        $$ = new ast_node();
                        $$->type = FK_CONSTRAINT;
                        $$->op = 2;
                        $$->op_val = 0;
                        add_to_cleanup(AST_NODE, (void**) &$$);
                    }
              ;

%%

int main(int argc, char* argv[]){
    if(argc == 2) yyin = fopen(argv[1],"r"); 
    yyparse();
    return 0;
}

int yyerror(string s){
    cout<<"ERROR"<<endl;
    return 1;
}
