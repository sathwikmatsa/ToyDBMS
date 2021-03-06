#ifndef _HEADER_CRUD
#define _HEADER_CRUD
#include <vector>
#include <string>
#include <deque>
#include <map>
#include <fstream>

using namespace std;

//********** DATA TYPES ************

typedef struct{
    // must be provided
    string name;
    bool isString; // false: integer

    // additional constraints

//ID
/*0*/   string FK_table;
/*1*/   string FK_attr;
/*2*/   int onDeleteFK; // 0: cascade, 1: set default, 2: set null
/*3*/   bool cascadeOnUpdateFK;
/*4*/   bool hasDefault;
/*5*/   string defaultVal;
/*6*/   bool notNULL;
/*7*/   bool isPK;
/*8*/   bool isFK;
} attr;

typedef vector<attr*> attrList;
typedef vector<string> record;
typedef vector<record> recordList;
typedef vector<string> columnList;

typedef struct{
    string name;
    int nAttrs;
    attrList attributes;
    recordList records;
    columnList primaryKeys;
} table;

typedef vector<table*> database;

enum NODE {PROGRAM, QUERIES, QUERY, CREATE_TABLE, SELECT, INSERT,
           DELETE, IDS, LITERALS, CONDITION, NQ_OR_LITERAL, CT_ARGS,
           CT_ARG,ATTR_DEF, FK_PK_DEF, TYPE, CONSTRAINTS, CONSTRAINT,
           FK_CONSTRAINTS, FK_CONSTRAINT };

typedef struct ast_node{
    NODE type;
    deque<struct ast_node*> childNodes;
    deque<string> list_val;
    string tableName;
    string attrName;
    bool isString;
    string literal_v;
    int attrID;
    int op, op_val;
    map<string, string> rename;
    int cond_operand;
} ast_node;

enum obj_type{TABLE, ATTR, AST_NODE, STRING};
typedef struct{
    obj_type t;
    void* data;
} heapObject;

//************ GLOBAL VARIABLES ************

extern vector <heapObject*> HeapObjects;
extern database Database;
extern ast_node* root;
extern map<int, string> type_of;
extern ofstream logfile;
extern bool LOG;

//*************** FUNCTIONS ****************

void garbageCollector();
void add_to_cleanup(obj_type t, void** ptr);
bool create_table_in_database(string tableName);
bool add_attribute(string attrName, bool isString, table* Table);
void add_constraint(int attrID, string attrName, table* Table, void* data);
void add_record(record r, string tableName);
void add_primaryKeyField(string attrName, table* Table);
table* get_table_from_database(string tableName);
table* copy_of_table(table* Table, deque<string> attrs);
int get_attr_index(string attrName, table* Table);
void print_database();
void print_table(table* Table);
void print_attributes(table* Table);
void print_records(table* Table);

#endif
