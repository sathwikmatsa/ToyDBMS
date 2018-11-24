#include <iostream>
#include <cstddef>
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
    table* Table;
    for(size_t i = 0; i<Database.size(); i++){
        if(Database[i]->name == tableName){
            Table = Database[i];
            break;
        }
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
		case 3 : Attr->cascadeOnUpdateFK = *((bool*) data); break;
		case 4 : Attr->hasDefault = *((bool*) data); break;
		case 5 : Attr->defaultVal = *((string*) data); break;
		case 6 : Attr->notNULL = *((bool*) data); break; 
		case 7 : Attr->isPK = *((bool*) data); break;
		case 8 : Attr->isFK = *((bool*) data); break;
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
