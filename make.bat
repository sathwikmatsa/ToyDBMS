@echo off
bison -yd sql_parser.y
flex -l sql_lexer.l
g++ -o parser.exe crud.cpp interpreter.cpp parser.cc lexer.cc
