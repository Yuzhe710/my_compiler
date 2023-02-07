# my_compiler
 compiler for subset of C
 
## Part_1 - A simple lexical scanner 
A simple lexical scanner that recognises 4 main math operators (+ - * /) and also integer literals.  

To compile:  
gcc main.c scan.c -o scanner  

To run with test inputs:  
./scanner input01  

## Part_2 - A simple but incorrect parser  
This part builds on a parser that parses the valid expressions into an Abstract Syntax Tree (AST) and interprets the results. However, the results are incorrect as the AST does not consider the operation precedence while being built. Expressions are parsed from left to right recursively as integer literals being leaf nodes and operations being internal nodes.  

In this part, the Backus-Naur Form (BNF) of our expressions is described as:  

expression: number  
          | expression '*' expression  
          | expression '/' expression  
          | expression '+' expression  
          | expression '-' expression  
          ;  
  
number:  T_INTLIT  
         ;  
  
To compile:  
gcc -o parser -g expr.c interp.c main.c scan.c tree.c  
  
To run against test inputs:  
./parser inputs01  
