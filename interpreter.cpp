#include <iostream>
#include <string>
#include <map>
#include <deque>
#include <cassert>
#include "crud.h"
#include "parser.h"
extern int yyparse();
extern FILE* yyin;

using namespace std;

ast_node* root = NULL;

table* processSelect(ast_node* sel);

void processFKPKDef(ast_node* fkpkDef, table* Table){
    if(LOG) logfile<<"func: processFKPKDef()  file: interpreter.cpp\n";
    // primary key
    if(fkpkDef->attrID == 7){
        deque<string> pk_fields = fkpkDef->childNodes[0]->list_val;

        int nFields = pk_fields.size();
        for(int i = 0; i < nFields; i++){
            add_primaryKeyField(pk_fields[i], Table);
            add_constraint(7 , pk_fields[i], Table, NULL);
        }
    // foreign key
    } else if(fkpkDef->attrID == 10){
        add_constraint(8, fkpkDef->attrName, Table, NULL);
        add_constraint(0, fkpkDef->attrName, Table,
                (void*) &fkpkDef->list_val[0]);
        add_constraint(1, fkpkDef->attrName, Table,
                (void*) &fkpkDef->list_val[1]);

        if(fkpkDef->childNodes[0] == NULL) return;
        // add FK constraints
        
        deque<ast_node*> fk_constraints = fkpkDef->childNodes[0]->childNodes;
        int nFKConstraints = fk_constraints.size();

        for(int i = 0; i < nFKConstraints; i++){
            if(fk_constraints[i]->op == 2){
                add_constraint(2, fkpkDef->attrName, Table,
                        (void*) &fk_constraints[i]->op_val);
            } else if(fk_constraints[i]->op == 3){
                add_constraint(3, fkpkDef->attrName, Table, NULL);
            }
        }
    }   
}

void processAttrDef(ast_node* attrDef, table* Table){
    if(LOG) logfile<<"func: processAttrDef()  file: interpreter.cpp\n";
    // obtain data type of attribute
    bool isString = attrDef->childNodes[0]->isString;

    // create attribute
    add_attribute(attrDef->attrName, isString, Table);

    if(attrDef->childNodes.size() == 1) return;

    // add constraints
    deque<ast_node*> constraints = attrDef->childNodes[1]->childNodes;
    int nConstraints = constraints.size();

    for(int i = 0; i < nConstraints; i++){
        assert(constraints[i]->type == CONSTRAINT
                && "Invalid Constraint!\n");
        
        if(constraints[i]->attrID == 6){
            add_constraint(6, attrDef->attrName, Table, NULL);
        } else if(constraints[i]->attrID == 4){
            add_constraint(4, attrDef->attrName, Table,
                    NULL);
            add_constraint(5, attrDef->attrName, Table,
                    (void*) &constraints[i]->literal_v);
        } else{
            assert(1 == 0 && "Invalid attrID!\n");
        }
    }
    
}

void processCTQ(ast_node* query){
    if(LOG) logfile<<"func: processCTQ()  file: interpreter.cpp\n";
    table* Table;
    if(!create_table_in_database(query->tableName)) return;
    Table = get_table_from_database(query->tableName);
    // process attributes
    deque<ast_node*> arguments = query->childNodes[0]->childNodes;
    int nArgs = arguments.size();
    
    for(int i = 0; i < nArgs; i++){
        switch(arguments[i]->type){
            case ATTR_DEF : processAttrDef(arguments[i], Table); break;
            case FK_PK_DEF: processFKPKDef(arguments[i], Table); break;
            default: break;
        }
    }
}

void processIQ(ast_node* ins){
    if(LOG) logfile<<"func: processIQ()  file: interpreter.cpp\n";
    table* Table = get_table_from_database(ins->tableName);
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
            int index_t = get_attr_index(attr_ids[i], Table);
            if(index_t < 0) return;
            newRecord[index_t] = literals[i];
        }
        //TODO: check for integrity violations
        Table->records.push_back(newRecord);
    }
    return;
}

deque<string> sanitize(table* Table, string attrName){
    if(LOG) logfile<<"func: sanitize()  file: interpreter.cpp\n";
    attrList attrs = Table->attributes;
    int len = attrs.size();
    int attr_index;
    bool has_attr = false;

    for(int i = 0; i < len; i++){
        if(attrs[i]->name == attrName){
            attr_index = i;
            has_attr = true;
            break;
        }
    }

    if(!has_attr){
        cout<<"ERROR: 13"<<endl;
    }

    deque<string> list_val;
    int nRecords = Table->records.size();

    for(int i = 0; i < nRecords; i++){
        list_val.push_back(Table->records[i][attr_index]);
    }

    return list_val;
}


bool processCondition(ast_node* condition, record* Record, table* Table){
    if(LOG) logfile<<"func: processCondition()  file: interpreter.cpp\n";
    if(condition->childNodes.size() == 1){
        int attr_index = get_attr_index(condition->attrName, Table);
        assert(attr_index >= 0);
        bool isString = Table->attributes[attr_index]->isString;
        deque<string> list_values;
        if(condition->childNodes[0]->cond_operand == 1){
            table* table_c = processSelect(condition->childNodes[0]->childNodes[0]);
            list_values = sanitize(table_c, condition->attrName);
        } else {
            list_values = condition->childNodes[0]->list_val;
        }
        int len = list_values.size();
        switch(condition->op){
            case 2:
            case 3: {
                for(int i = 0; i < len; i++){
                    if(isString){
                        if(((*Record)[attr_index]) == (list_values[i])) 
                            return true; 
                    } else {
                        if(stoi((*Record)[attr_index]) == stoi(list_values[i]))
                            return true;
                    }
                }
            }break;
            case 4: {
                for(int i = 0; i < len; i++){
                    if(isString){
                        if(((*Record)[attr_index]) != (list_values[i])) 
                            return true; 
                    } else {
                        if(stoi((*Record)[attr_index]) != stoi(list_values[i]))
                            return true;
                    }
                }
            }break;
            case 5: {
                for(int i = 0; i < len; i++){
                    if(isString){
                        if(((*Record)[attr_index]) >= (list_values[i])) 
                            return false;
                    } else {
                        if(stoi((*Record)[attr_index]) >= stoi(list_values[i]))
                            return false;
                    }
                }
                return true;
            }break;
            case 6: {
                for(int i = 0; i < len; i++){
                    if(isString){
                        if(((*Record)[attr_index]) <= (list_values[i])) 
                            return false;
                    } else {
                        if(stoi((*Record)[attr_index]) <= stoi(list_values[i]))
                            return false;
                    }
                }
                return true;
            }break;
            case 7: {
                for(int i = 0; i < len; i++){
                    if(isString){
                        if(((*Record)[attr_index]) > (list_values[i])) 
                            return false;
                    } else {
                        if(stoi((*Record)[attr_index]) > stoi(list_values[i]))
                            return false;
                    }
                }
                return true;
            }break;
            case 8: {
                for(int i = 0; i < len; i++){
                    if(isString){
                        if(((*Record)[attr_index]) < (list_values[i])) 
                            return false;
                    } else {
                        if(stoi((*Record)[attr_index]) < stoi(list_values[i]))
                            return false;
                    }
                }
                return true;
            }break;
            default: break;
        }
    } else {
        if(condition->op == 0) 
            return processCondition(condition->childNodes[0], Record, Table) 
                && processCondition(condition->childNodes[1], Record, Table);
        else
            return processCondition(condition->childNodes[0], Record, Table) 
                || processCondition(condition->childNodes[1], Record, Table);
    }
    return false; 
}

void processDelete(table* Table, ast_node* condition, bool del_if_true){
    if(LOG) logfile<<"func: processDelete()  file: interpreter.cpp\n";
    int nRecords = Table->records.size();

    int record_index = 0;
    while(record_index < nRecords){
        record Record = Table->records[record_index];
        if(processCondition(condition, &Record, Table) == del_if_true){
            Table->records.erase(Table->records.begin() + record_index);
            nRecords--;
            record_index--;
        }
        record_index++;
    }
}

void processDQ(ast_node* query){
    if(LOG) logfile<<"func: processDQ()  file: interpreter.cpp\n";
    table* Table = get_table_from_database(query->tableName);
    processDelete(Table, query->childNodes[0], true);
}

void processMAX(table* Table){
    if(LOG) logfile<<"func: processMAX()  file: interpreter.cpp\n";
    int nRecords = Table->records.size();
    bool isString = Table->attributes[0]->isString;
    assert(isString == false);
    int MAX_i = stoi(Table->records[0][0]);
    string MAX_s;
    for(int i = 1; i < nRecords; i++){
        if(MAX_i < stoi(Table->records[i][0])){
            MAX_i = stoi(Table->records[i][0]);
            MAX_s = Table->records[i][0];
        }
    }
    
    Table->records.clear();
    Table->records.push_back({MAX_s});
}

table* processSelect(ast_node* sel){
    if(LOG) logfile<<"func: processSelect()  file: interpreter.cpp\n";
    table* newTable = NULL;
    table* oldTable = get_table_from_database(sel->tableName);
    switch(sel->op){
        case 0: {
            newTable = copy_of_table(oldTable, {});
            processDelete(newTable, sel->childNodes[0], false);
        }break;
        case 1: {
            newTable = copy_of_table(oldTable, sel->childNodes[0]->list_val);
            processDelete(newTable, sel->childNodes[1], false);
        }break;
        case 2: {
            deque<string> c_attr{sel->attrName};
            newTable = copy_of_table(oldTable, c_attr);
            processDelete(newTable, sel->childNodes[0], false);
            processMAX(newTable);
        }break;
        case 3: {
            deque<string> c_attr{sel->attrName};
            newTable = copy_of_table(oldTable, c_attr);
            processMAX(newTable);
        }break;
        case 4: {
            newTable = copy_of_table(oldTable, {});
        }break;
        case 5: {
            newTable = copy_of_table(oldTable, sel->childNodes[0]->list_val);
        }break;
        default: break;
    }
    add_to_cleanup(TABLE, (void**) newTable);
    return newTable;
}

void processSQ(ast_node* query){
    if(LOG) logfile<<"func: processSQ()  file: interpreter.cpp\n";
    table* Table = processSelect(query);
    print_table(Table);
}

void processQuery(ast_node* query){
    if(LOG) logfile<<"func: processQuery()  file: interpreter.cpp\n";
    switch(query->type){
        case CREATE_TABLE: processCTQ(query); break;
        case INSERT: processIQ(query); break;
        case DELETE: processDQ(query); break;
        case SELECT: processSQ(query); break;
        default: break;
    }
}

int main(int argc, char* argv[]){
    if(argc == 2) yyin = fopen(argv[1],"r"); 
    if(yyparse() == 1) return 1;
    assert(root->type == QUERIES && "Root element type isn't QUERIES!\n");
    LOG = true;
    logfile.open("log.txt");
    if(LOG) logfile<<"func: main()  file: interpreter.cpp\n";
    int nQueries = root->childNodes.size();
    for(int i = 0; i < nQueries; i++){
        processQuery(root->childNodes[i]);
    }
    garbageCollector();
    logfile.close();
    return 0;
}
