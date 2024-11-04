
# Makefile to compile and run the simple shell program

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

all: myshell

myshell: main.cc
	$(CXX) $(CXXFLAGS) -o myshell main.cc

clean:
	rm -f myshell

run: myshell
	./myshell
