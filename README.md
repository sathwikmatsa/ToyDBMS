# ToyDBMS
Simple RAM based DBMS in C++

## Dependencies
 - Bison
 - Flex
 - g++
 
 All the dependencies can be installed via `apt-get` on Linux.
 
 For Windows, install Bison and Flex from [GNUWin32](http://gnuwin32.sourceforge.net/packages.html) and setup [MinGW](https://sourceforge.net/projects/mingw-w64/) for g++. (add them to path)

## Usage
Download repo :
```
git clone https://github.com/sathwikmatsa/ToyDBMS.git
cd ToyDBMS
```

### Linux
to build: run `make`

to execute queries in input.txt:
```
$ ./sql input.txt
```

### Windows
to build: run `make.bat`

to execute queries in input.txt:
```
> sql.exe input.txt
```

## Features
supported queries:
- CREATE TABLE
- SELECT
- INSERT
- DELETE
- WHERE, MAX, IN
#### compatible with MySQL syntax
Note: this is a single database system

## How it Works?
`lexer` (sql_lexer.l) : tokenizes the contents of input.txt

`parser` (sql_parser.y) : consumes the tokens from lexer and constructs an AST

`interpreter` (interpreter.cpp) : evaluates nodes in the AST to realize the output

`crud.cpp` contains the implementation of functions for dealing with storing/manipulation of data which are used by the interpreter.

### Code Walkthrough Example
consider the following query: `CREATE TABLE PERSON(ID INT, NAME VARCHAR(15));`

**_lexer_** produces the following tokens: 
```
[create] [table_t] [id] [(] [id] [int_t] [,] [id] [varchar] [(] [literal] [)] [)] [;]
```
**_parser_** recognizes it by the following [rules](https://github.com/sathwikmatsa/ToyDBMS/blob/master/sql_parser.y#L262-#L323):
```
CREATE_TABLE : create table_t id '(' CT_ARGS ')'
CT_ARGS : CT_ARG | CT_ARG ',' CT_ARGS
CT_ARG : ATTR_DEF
ATTR_DEF : id TYPE
TYPE : int_t | varchar '(' literal ')'
```
* each non terminal token (in caps) can use a `ast_node` pointer to store required information like table name, arguments etc.
* ast_node pointers of non terminals on RHS are stored in `childNodes` member of ast_node of the corresponding LHS non terminal.

**_interpreter_** evaluates the query by calling `processQuery`.
* processQuery : identifies NODE type -> calls `processCTQ`
* `processCTQ` : 
     * creates table in database by calling `create_table_in_database`
     * adds attributes to the table by evaluating the childNode (CT_ARGS) by retreiving childNodes of CT_ARGS and processing each node based on it's type. In our case it's ATTR_DEF, so it calls `processAttrDef` which in turn calls `add_attribute` with appropriate parameters.
     
## TODO
- [ ] implement checks for attribute constraints and integrity constraints
- [ ] handle NULL values
- [ ] code refactoring
