#include <iostream>
#include "crud.h"
#include "parser.h"
extern int yyparse();
extern FILE* yyin;
using namespace std;

ast_node* root;

int main(int argc, char* argv[]){
    if(argc == 2) yyin = fopen(argv[1],"r"); 
    yyparse();
    cout<< (root->type == QUERIES) <<endl;
    garbageCollector();
    return 0;
}
