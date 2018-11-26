#include <iostream>
#include <string>
#include <map>
#include <deque>
#include <cassert>
#include "crud.h"
#include "parser.h"
extern int yyparse();
extern FILE* yyin;

#define global_variable static
using namespace std;

ast_node* root = NULL;

global_variable string table_n;         //file scoped
global_variable string attribute_n;
global_variable record record_c;
global_variable int record_index;

void processFKPKDef(ast_node* fkpkDef){
    // primary key
    if(fkpkDef->attrID == 7){
        deque<string> pk_fields = fkpkDef->childNodes[0]->list_val;

        int nFields = pk_fields.size();
        for(int i = 0; i < nFields; i++){
            add_primaryKeyField(pk_fields[i], table_n);
            add_constraint(7 , pk_fields[i], table_n, NULL);
        }
    // foreign key
    } else if(fkpkDef->attrID == 10){
        add_constraint(8, fkpkDef->attrName, table_n, NULL);
        add_constraint(0, fkpkDef->attrName, table_n,
                (void*) &fkpkDef->list_val[0]);
        add_constraint(1, fkpkDef->attrName, table_n,
                (void*) &fkpkDef->list_val[1]);

        if(fkpkDef->childNodes[0] == NULL) return;
        // add FK constraints
        
        deque<ast_node*> fk_constraints = fkpkDef->childNodes[0]->childNodes;
        int nFKConstraints = fk_constraints.size();

        for(int i = 0; i < nFKConstraints; i++){
            if(fk_constraints[i]->op == 2){
                add_constraint(2, fkpkDef->attrName, table_n,
                        (void*) &fk_constraints[i]->op_val);
            } else if(fk_constraints[i]->op == 3){
                add_constraint(3, fkpkDef->attrName, table_n, NULL);
            }
        }
    }   
}

void processAttrDef(ast_node* attrDef){
    // set attribute_n
    attribute_n = attrDef->attrName;
    // obtain data type of attribute
    bool isString = attrDef->childNodes[0]->isString;

    // create attribute
    add_attribute(attribute_n, isString, table_n);

    if(attrDef->childNodes.size() == 1) return;

    // add constraints
    deque<ast_node*> constraints = attrDef->childNodes[1]->childNodes;
    int nConstraints = constraints.size();

    for(int i = 0; i < nConstraints; i++){
        assert(constraints[i]->type == CONSTRAINT
                && "Invalid Constraint!\n");
        
        if(constraints[i]->attrID == 6){
            add_constraint(6, attribute_n, table_n, NULL);
        } else if(constraints[i]->attrID == 4){
            add_constraint(4, attribute_n, table_n,
                    NULL);
            add_constraint(5, attribute_n, table_n,
                    (void*) &constraints[i]->literal_v);
        } else{
            assert(1 == 0 && "Invalid attrID!\n");
        }
    }
    
}

void processCTQ(ast_node* query){
    // set table_n
    table_n = query->tableName;
    if(!create_table_in_database(table_n)) return;
    
    // process attributes
    deque<ast_node*> arguments = query->childNodes[0]->childNodes;
    int nArgs = arguments.size();
    
    for(int i = 0; i < nArgs; i++){
        switch(arguments[i]->type){
            case ATTR_DEF : processAttrDef(arguments[i]); break;
            case FK_PK_DEF: processFKPKDef(arguments[i]); break;
            default: break;
        }
    }
}

void processIQ(ast_node* ins){
    //set table_n
    table_n = ins->tableName;
    table* Table = get_table(table_n);
    if(ins->childNodes.size() == 1){
        record newRecord(ins->childNodes[0]->list_val.begin(), 
                ins->childNodes[0]->list_val.end());
        //TODO: check for integrity violations
        Table->records.push_back(newRecord);
    } else if(ins->childNodes.size() == 2){
        record newRecord;
        newRecord.resize(Table->nAttrs, "NULL");
        deque<string> attr_ids = ins->childNodes[0]->list_val;
        deque<string> literals = ins->childNodes[1]->list_val;

        assert(attr_ids.size() == literals.size() 
                && "attributes len doesn't equal literals len!");

        int len = attr_ids.size();

        for(int i = 0; i < len; i++){
            int index_t = get_attr_index(attr_ids[i], table_n);
            if(index_t < 0) return;
            newRecord[index_t] = literals[i];
        }
        //TODO: check for integrity violations
        Table->records.push_back(newRecord);
    }
    return;
}

void processSelect();
void sanitize();

bool processCondition(ast_node* condition){
    if(condition->childNodes.size() == 1){
        int attr_index = get_attr_index(condition->attrName, table_n);
        assert(attr_index >= 0);
        bool isString = get_table(table_n)->attributes[attr_index]->isString;
        //set attribute_n
        attribute_n = condition->attrName;
        deque<string> list_values;
        if(condition->cond_operand == 1){
            //values = sanitize(processSelect());
        } else {
            list_values = condition->childNodes[0]->list_val;
        }
        int len = list_values.size();
        switch(condition->op){
            case 2:
            case 3: {
                for(int i = 0; i < len; i++){
                    if(isString){
                        if((record_c[attr_index]) == (list_values[i])) 
                            return true; 
                    } else {
                        if(stoi(record_c[attr_index]) == stoi(list_values[i]))
                            return true;
                    }
                }
            }break;
            case 4: {
                for(int i = 0; i < len; i++){
                    if(isString){
                        if((record_c[attr_index]) != (list_values[i])) 
                            return true; 
                    } else {
                        if(stoi(record_c[attr_index]) != stoi(list_values[i]))
                            return true;
                    }
                }
            }break;
            case 5: {
                for(int i = 0; i < len; i++){
                    if(isString){
                        if((record_c[attr_index]) >= (list_values[i])) 
                            return false;
                    } else {
                        if(stoi(record_c[attr_index]) >= stoi(list_values[i]))
                            return false;
                    }
                }
                return true;
            }break;
            case 6: {
                for(int i = 0; i < len; i++){
                    if(isString){
                        if((record_c[attr_index]) <= (list_values[i])) 
                            return false;
                    } else {
                        if(stoi(record_c[attr_index]) <= stoi(list_values[i]))
                            return false;
                    }
                }
                return true;
            }break;
            case 7: {
                for(int i = 0; i < len; i++){
                    if(isString){
                        if((record_c[attr_index]) > (list_values[i])) 
                            return false;
                    } else {
                        if(stoi(record_c[attr_index]) > stoi(list_values[i]))
                            return false;
                    }
                }
                return true;
            }break;
            case 8: {
                for(int i = 0; i < len; i++){
                    if(isString){
                        if((record_c[attr_index]) < (list_values[i])) 
                            return false;
                    } else {
                        if(stoi(record_c[attr_index]) < stoi(list_values[i]))
                            return false;
                    }
                }
                return true;
            }break;
            default: break;
        }
    } else {
        if(condition->op == 0) 
            return processCondition(condition->childNodes[0]) 
                && processCondition(condition->childNodes[1]);
        else
            return processCondition(condition->childNodes[0]) 
                || processCondition(condition->childNodes[1]);
    }
    return false; 
}

void processDQ(ast_node* query){
    //set table_n
    table_n = query->tableName;
    table* Table = get_table(table_n);

    int nRecords = Table->records.size();

    //iterate over and set record_c & record_index
    record_index = 0;
    while(record_index < nRecords){
        record_c = Table->records[record_index];
        if(processCondition(query->childNodes[0])){
            Table->records.erase(Table->records.begin() + record_index);
            nRecords--;
            record_index--;
        }
        record_index++;
    }
}

table* processSelect(ast_node* sel){
    //TODO
    return NULL;
}

void processSQ(ast_node* query){
    table* Table = processSelect(query);
    print_table(Table);
}

void processQuery(ast_node* query){
    switch(query->type){
        case CREATE_TABLE: processCTQ(query); break;
        case INSERT: processIQ(query); break;
        case DELETE: processDQ(query); break;
        //case SELECT: processSQ(query); break;
        default: break;
    }
}

int main(int argc, char* argv[]){
    if(argc == 2) yyin = fopen(argv[1],"r"); 
    yyparse();
    assert(root->type == QUERIES && "Root element type isn't QUERIES!\n");
    int nQueries = root->childNodes.size();
    for(int i = 0; i < nQueries; i++){
        processQuery(root->childNodes[i]);
    }
    print_database();
    garbageCollector();
    return 0;
}
