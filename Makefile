# manual way to build
all:
	flex -o lexer.cpp lexer.l
	bison -o parser.cpp parser.y
	g++ -g main.cc lexer.cpp parser.cpp interpreter.cpp -o a.out

clean:
	rm -rf lexer.cpp
	rm -rf parser.cpp parser.hpp location.hh position.hh stack.hh
	rm -rf a.out
