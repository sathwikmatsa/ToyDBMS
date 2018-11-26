all: sql

parser.cc parser.h : sql_parser.y
		yacc -d sql_parser.y

lexer.cc : sql_lexer.l parser.h
		lex sql_lexer.l

sql: parser.cc lexer.cc crud.cpp interpreter.cpp
		g++ -o sql crud.cpp interpreter.cpp parser.cc lexer.cc

clean:
		rm -f *.cc *.o sql parser.h 
