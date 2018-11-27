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
#### compatible with MySQL syntax
Note: this is a single database system

## How it Works?
`lexer` (sql_lexer.l) : tokenizes the contents of input.txt

`parser` (sql_parser.y) : consumes the tokens from lexer and constructs an AST

`interpreter` (interpreter.cpp) : evaluates nodes in the AST to realize the output

`crud.cpp` contains the implementation of functions for dealing with storing/manipulation of data which are used by the interpreter.
