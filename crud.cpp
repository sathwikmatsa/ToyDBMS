#include <iostream>
#include <cstddef>
#include <map>
#include "crud.h"

using namespace std;

vector <heapObject*> HeapObjects;
database Database;
ofstream logfile;
bool LOG;

void garbageCollector(){
    if(LOG) logfile<<"func: garbageCollector()  file: crud.cpp\n";
    int l = HeapObjects.size();
    for(int i = 0; i<l; i++){
        heapObject* obj = HeapObjects[i];
        if(obj->t == TABLE){
            delete (table*) obj->data;
        } else if(obj->t == ATTR) {
            delete (attr*) obj->data;
        } else if(obj->t == AST_NODE){
            delete (ast_node*) obj->data;
        } else {
            delete (string*) obj->data;
        }
        delete obj;
    }
    HeapObjects.clear();
}

void add_to_cleanup(obj_type t, void** ptr){
    if(LOG) logfile<<"func: add_to_cleanup()  file: crud.cpp\n";
    heapObject* newHeapObj = new heapObject();
    newHeapObj->t = t;
    newHeapObj->data = (*ptr);
}


bool create_table_in_database(string tableName){
    if(LOG) logfile<<"func: create_table_in_database()  file: crud.cpp\n";
    //TODO: make sure to discard related operations if false
    // display error and return false if table name already exists!
    for(size_t i = 0; i<Database.size(); i++){
        if(Database[i]->name == tableName){
            cout<<"Error: Table already exists!"<<endl;
            return false;
        }
    }

    table* newTable = new table();
    newTable->name = tableName;
    Database.push_back(newTable);

    // add newTable to HeapObjects for cleanup at the end
    add_to_cleanup(TABLE, (void**) newTable);
    return true;
}

bool add_attribute(string attrName, bool isString, string tableName){
    if(LOG) logfile<<"func: add_attribute()  file: crud.cpp\n";
    table* Table = NULL;
    for(size_t i = 0; i<Database.size(); i++){
        if(Database[i]->name == tableName){
            Table = Database[i];
            break;
        }
    }

    if(Table == NULL) {
        cout<<"Error: Table-"<<tableName<<" not found!"<<endl;
        return false;
    }

    attr* newAttr = new attr();
    newAttr->name = attrName;
    newAttr->isString = isString;
    
    Table->attributes.push_back(newAttr);
    Table->nAttrs+=1;
    
    // add newAttr to HeapObjects for cleanup at the end
    add_to_cleanup(ATTR, (void**) &newAttr);
    return true;
}

void add_constraint(int attrID, string attrName, string tableName, void* data){
    if(LOG) logfile<<"func: add_constraint()  file: crud.cpp\n";
    table* Table;
    for(size_t i = 0; i<Database.size(); i++){
        if(Database[i]->name == tableName){
            Table = Database[i];
            break;
        }
    }

    attr* Attr;
    for(int i = 0; i<Table->nAttrs; i++){
        if(Table->attributes[i]->name == attrName){
            Attr = Table->attributes[i];
            break;
        }
    }

    switch(attrID){
		case 0 : Attr->FK_table = *((string*) data); break;
		case 1 : Attr->FK_attr = *((string*) data); break;
		case 2 : Attr->onDeleteFK = *((int*) data); break;
		case 3 : Attr->cascadeOnUpdateFK = true; break;
		case 4 : Attr->hasDefault = true; break;
		case 5 : Attr->defaultVal = *((string*) data); break;
		case 6 : Attr->notNULL = true; break; 
		case 7 : Attr->isPK = true; break;
		case 8 : Attr->isFK = true; break;
		default : return;
	}

}

void add_record(record r, string tableName){
    if(LOG) logfile<<"func: add_record()  file: crud.cpp\n";
    table* Table;
    bool tableExists = false;
    for(size_t i = 0; i<Database.size(); i++){
        if(Database[i]->name == tableName){
            Table = Database[i];
            tableExists = true;
            break;
        }
    }

    if(!tableExists){
        cout<<"Error: Table-"<<tableName<<" not found!"<<endl;
        return;
    }

    Table->records.push_back(r);
}

void add_primaryKeyField(string attrName, string tableName){
    if(LOG) logfile<<"func: add_primaryKeyField()  file: crud.cpp\n";
    table* Table;
    for(size_t i = 0; i<Database.size(); i++){
        if(Database[i]->name == tableName){
            Table = Database[i];
            break;
        }
    }

    bool attrExists = false;
    for(int i = 0; i<Table->nAttrs; i++){
        if(Table->attributes[i]->name == attrName){
            attrExists = true;
            break;
        }
    }

    if(!attrExists){
        cout<<"ERROR: Attribute-"<<attrName<<" not found!"<<endl;
        return;
    }
    
    Table->primaryKeys.push_back(attrName);

}

table* get_table(string tableName){
    if(LOG) logfile<<"func: get_table()  file: crud.cpp\n";
    table* Table = NULL;
    for(size_t i = 0; i<Database.size(); i++){
        if(Database[i]->name == tableName){
            Table = Database[i];
            break;
        }
    }
    return Table;
}

table* copy_of_table(string tableName, deque<string> attrs_i){
    if(LOG) logfile<<"func: copy_of_table()  file: crud.cpp\n";
    table* Table = get_table(tableName); // original
    table* newTable = new table();   
    newTable->name = Table->name;
    
    deque<string> attrs = attrs_i;
    int len = attrs.size();

    if(len == 0){
        attrList attrsL = Table->attributes;
        int nAttrs = attrsL.size();
        for(int i = 0; i < nAttrs; i++){
            attrs.push_back(attrsL[i]->name);
        }
        len = nAttrs;
    }

    int recs_size = Table->records.size();
    newTable->nAttrs = len;
    newTable->records.resize(recs_size, {});
    
    for(int i = 0; i < len; i++){
        string attr_name = attrs[i];
        int attr_index = get_attr_index(attr_name, tableName);
        newTable->attributes.push_back(Table->attributes[attr_index]);
        for(int j = 0; j < recs_size; j++){
            newTable->records[j].push_back(Table->records[j][attr_index]);
        }
    }
    return newTable;
}

int get_attr_index(string attrName, string tableName){
    if(LOG) logfile<<"func: get_attr_index()  file: crud.cpp\n";
    table* Table;
    for(size_t i = 0; i<Database.size(); i++){
        if(Database[i]->name == tableName){
            Table = Database[i];
            break;
        }
    }

    bool attrExists = false;
    int index = 0;
    for(int i = 0; i<Table->nAttrs; i++){
        if(Table->attributes[i]->name == attrName){
            attrExists = true;
            return index;
        }
        index++;
    }

    if(!attrExists){
        cout<<"ERROR: Attribute-"<<attrName<<" not found!"<<endl;
        return -1;
    }
}

void print_database(){
    if(LOG) logfile<<"func: print_database()  file: crud.cpp\n";
    cout<<"Database:"<<endl;
    for(size_t i = 0; i<Database.size(); i++){
        print_table(Database[i]);
    }
}

void print_table(table* Table){
    if(LOG) logfile<<"func: print_table()  file: crud.cpp\n";
    cout<<Table->name<<endl;
    print_attributes(Table);
    print_records(Table);
}

void print_attributes(table* Table){
    if(LOG) logfile<<"func: print_attributes()  file: crud.cpp\n";
    for(int i = 0; i<Table->nAttrs; i++){
        cout<<Table->attributes[i]->name<<" ";
    }
    cout<<endl;
}

void print_records(table* Table){
    if(LOG) logfile<<"func: print_records()  file: crud.cpp\n";
    for(size_t i = 0; i<Table->records.size(); i++){
        for(int j = 0; j<Table->nAttrs; j++){
            cout<<Table->records[i][j]<<" ";
        }
        cout<<endl;
    }
}


map<int, string> type_of = {{0, "PROGRAM"}, {1, "QUERIES"},
    {2, "QUERY"}, {3, "CREATE_TABLE"}, {4, "SELECT"}, {5, "INSERT"},
    {6, "DELETE"}, {7, "IDS"}, {8, "LITERALS"}, {9, "CONDITION"},
    {10, "NQ_OR_LITERAL"}, {11, "CT_ARGS"}, {12, "CT_ARG"},
    {13, "ATRR_DEF"}, {14, "FK_PK_DEF"}, {15, "TYPE"},
    {16, "CONSTRAINTS"}, {17, "CONSTRAINT"}, {18, "FK_CONSTRAINTS"}, 
    {19, "FK_CONSTRAINT"}};
