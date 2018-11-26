# ToyDBMS
Simple RAM based DBMS in C++

# Dependencies
 - Bison
 - Flex
 - g++
 
 All the dependencies can be installed via `apt-get` on Linux.
 
 For Windows, install Bison and Flex from [GNUWin32](http://gnuwin32.sourceforge.net/packages.html) and setup [MinGW](https://sourceforge.net/projects/mingw-w64/) for g++. (add them to path)

# Usage
## Linux
to build: run `make`

to execute queries in input.txt:
```
> ./sql input.txt
```

## Windows
to build: run `make.bat`

to execute queries in input.txt:
```
> sql.exe input.txt
```
