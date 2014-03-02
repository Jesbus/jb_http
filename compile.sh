#!/bin/bash

clear
echo "################################################################"
echo "################################################################"

cd binaries

if [ $1 == "definitions" ] || [ $1 == "all" ]; then
	echo "Compiling defs..."
	g++ -c ../definitions.cpp -Wno-write-strings -std=c++11 -lboost_regex 2>&1 | grep error
fi
if [ $1 == "interpreter" ] || [ $1 == "all" ]; then
	echo "Compiling interpreter..."
	g++ -c ../interpreter.cpp -Wno-write-strings -std=c++11 -lboost_regex 2>&1 | grep error
fi
if [ $1 == "parser" ] || [ $1 == "all" ]; then
	echo "Compiling parser..."
	g++ -c ../parser.cpp -Wno-write-strings -std=c++11 -lboost_regex 2>&1 | grep error
fi
if [ $1 == "compiler" ] || [ $1 == "all" ]; then
	echo "Compiling compiler..."
	g++ -c ../compiler.cpp -Wno-write-strings -std=c++11 -lboost_regex 2>&1 | grep error
fi
if [ $1 == "main" ] || [ $1 == "all" ]; then
	echo "Compiling main..."
	g++ -c ../main.cpp -Wno-write-strings -std=c++11 -lboost_regex 2>&1 | grep error
fi
echo "Linking..."
g++ main.o compiler.o parser.o interpreter.o definitions.o -Wno-write-strings -std=c++11 -lboost_regex -o jb_http
