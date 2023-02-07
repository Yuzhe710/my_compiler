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

## Part_3 A correct parser  
This part implements a correct parser, which results from the building AST correctly obey the operation precedence.  
  
To build such a AST, firstly the BNF of our grammar changes from  
  
expression: number  
          | expression '*' expression  
          | expression '/' expression  
          | expression '+' expression  
          | expression '-' expression  
          ;  
  
number:  T_INTLIT  
         ;  
  
To  
  
expression: additive_expression  
    ;  
  
additive_expression:  
      multiplicative_expression  
    | additive_expression '+' multiplicative_expression  
    | additive_expression '-' multiplicative_expression  
    ;  
  
multiplicative_expression:  
      number  
    | number '*' multiplicative_expression  
    | number '/' multiplicative_expression  
    ;  
  
number:  T_INTLIT  
         ;  
  
We now have 2 types of expressions: additive and multiplicative. The grammar now forces the numbers to be part of multiplicative expressions only. This forces the "*" and "/" operators to bind more tightly to the numbers on either side, so that they have higher precedence.  
  
We can still do it in the Recursive Decent Parser with explicit operator precedence (as shown in DoctorWkt's tutorial) but will be annoyed by the amount of code (we need to write a new function to handle each new higher-precedence operator). Thus, Pratt parser was adopted. It has a table of precedence values associated with each token. The AST will be basically built by making the subtree of higher precedence operators first, and join lower subtrees. It enable us to have a single function to do the expression parsing.  
  
To compile,  
gcc -o parser expr.c interp.c main.c scan.c tree.c  
  
To test:  
  
./parser input01 (this time only final result will be present)  
