@echo off
bison -yd sql_parser.y
flex -l sql_lexer.l
g++ -o parser.exe parser.cc crud.cpp lexer.cc
