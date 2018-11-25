#include <iostream>
#include <cstddef>
#include <map>
#include "crud.h"

using namespace std;

vector <heapObject*> HeapObjects;
database Database;

void garbageCollector(){
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
    heapObject* newHeapObj = new heapObject();
    newHeapObj->t = t;
    newHeapObj->data = (*ptr);
}


bool create_table_in_database(string tableName){
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
    table* Table = NULL;
    for(size_t i = 0; i<Database.size(); i++){
        if(Database[i]->name == tableName){
            Table = Database[i];
            break;
        }
    }
    return Table;
}

int get_attr_index(string attrName, string tableName){
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
    cout<<"Database:"<<endl;
    for(size_t i = 0; i<Database.size(); i++){
        print_table(Database[i]);
    }
}

void print_table(table* Table){
    cout<<Table->name<<endl;
    print_attributes(Table);
    print_records(Table);
}

void print_attributes(table* Table){
    for(int i = 0; i<Table->nAttrs; i++){
        cout<<Table->attributes[i]->name<<" ";
    }
    cout<<endl;
}

void print_records(table* Table){
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
