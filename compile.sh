#!/bin/bash

clear
echo "################################################################"
echo "################################################################"
echo "################################################################"
echo "################################################################"
echo "################################################################"
echo "################################################################"
echo "################################################################"
echo "################################################################"
echo "################################################################"
echo "################################################################"

if [ $1 == "main" ] || [ $1 == "all" ]; then
	echo "Compiling main..."
	g++ -c jb_http_main.cpp -Wno-write-strings -std=c++11 -lboost_regex 2>&1 | grep error
fi
if [ $1 == "definitions" ] || [ $1 == "all" ]; then
	echo "Compiling defs..."
	g++ -c jb_http_definitions.cpp -Wno-write-strings -std=c++11 -lboost_regex 2>&1 | grep error
fi
if [ $1 == "interpreter" ] || [ $1 == "all" ]; then
	echo "Compiling interpreter..."
	g++ -c jb_http_interpreter.cpp -Wno-write-strings -std=c++11 -lboost_regex 2>&1 | grep error
fi
if [ $1 == "parser" ] || [ $1 == "all" ]; then
	echo "Compiling parser..."
	g++ -c jb_http_parser.cpp -Wno-write-strings -std=c++11 -lboost_regex 2>&1 | grep error
fi
if [ $1 == "compiler" ] || [ $1 == "all" ]; then
	echo "Compiling compiler..."
	g++ -c jb_http_compiler.cpp -Wno-write-strings -std=c++11 -lboost_regex 2>&1 | grep error
fi
echo "Linking..."
g++ jb_http_main.o jb_http_compiler.o jb_http_parser.o jb_http_interpreter.o jb_http_definitions.o -Wno-write-strings -std=c++11 -lboost_regex -o jb_http
