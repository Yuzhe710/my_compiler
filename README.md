# my_compiler
 compiler for subset of C, I mainly learn from [DoctorWkt's project](https://github.com/DoctorWkt/acwj), it is still ongoing...  
 
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

## Part_4 assembly code generation  
In this part, we generate assembly from AST (previously we only have interpreter to interpret the result).  
  
We uses 4 general purpose registers in x86-64, r8, r9, r10, r11  
  
code generator from AST is quite similar process (in terms of structure) compared to interpret it.  
  
One thing which does not make full sense to me is the x86 assembly code, the preamble and postamble.  
  
To compile：  
gcc-12 -o comp1 cg.c expr.c gen.c interp.c main.c scan.c tree.c  
  
To test with inputs:  
./comp1 input01 (the out.s contains the generated assembly code)  

## Part_5 print statements  
A statement grammar has been added . The BNF notation is:  
  
statements: statement  
       | statement statements  
       ;  
  
statement: 'print' expression ';'  
       ;  
  
So far the statement only has print keyword. But we have made changes to lexical scanner, parser to make code compatible with other future statements.  
  
As currently there is only one type of statement in our language, this part we gonna test only against that (ignore other tests from previous parts). We first scan and match a "print" keyword as the first token, then parse the following expressions as usual, buid AST, generate assembly code and finally free all the registers. Note that we also add semicolon into our language, which is expected to have at the end of every statement.  
  
To compile:  
gcc -o comp1 cg.c expr.c gen.c main.c misc.c scan.c stmt.c tree.c  
  
To test:  
Step 1: ./comp1 input05  
  
Step 2: gcc -o out out.s  
  
Step 3: ./out  
  
## Part_6 Variables  
This part we add variables. We are able to:  
1. Declare variables  
2. Use variables to get stored values  
3. Assign to variables  
  
---------------------------------------------------------------------  
A sample input:  
  
int fred;  
int jim;  
fred = 5;  
jim = 12;  
print fred + jim;  
  
---------------------------------------------------------------------  
We firstly define a symbol table structure, it is going to store symbols (variables).  
  
2 new tokens are defined. (as in the example above),  '=' is known as T_EQUALS and identifier names (variable) is T_IDENT.  
  
Correspondently, new AST node type will be A_ASSIGN, A_IDENT and A_LVALUE (explain later)  
--------------------------------------------------------------------  
We also have new grammar:  
  
statements: statement  
      |      statement statements  
      ;  
  
 statement: 'print' expression ';'  
      |     'int'   identifier ';'  
      |     identifier '=' expression ';'  
      ;  
  
 identifier: T_IDENT  
      ;  
  
--------------------------------------------------------------------  
  
Hence, from the sample input, we can see 3 types of statements related to variables, namely declaration, assignment and print.  
  
#variable declaration:  
We firstly match 'int', then the identifier (adds into symbol table) and finally match the semicolon.  
  
#assignment statement  
When parsing an assignment statement, we first parse an identifier, then we make a leaf node for that identifier with tree value being its id in symbol table and nodetype being A_LVALUE (means the variable on the left side of the assignment statement). However, this node will be the left subtree of the assignment node (node with the nodetype A_ASSIGN), whilst the expression on the right side of the assignment statement will be the left subtree, this is because when interpreting / generating assembly, AST is evaluated from left to right. We have to know which register hold the expression value before we load that register value into the variable.  
  
The left sub tree (the expression) will just be evaluated as before.  
  
                     =  
                    /   \  
                   /     \  
                  /       \  
(A_INTLIT, 3)    (A_LVIDENT, id)  
  
# print statement  
We firstly match the print keyword, then we parse the two identifiers which become the left and right subtree of the operation node.  
  
                     +  
                    /   \  
                   /     \  
                  /       \  
(A_IDENT, id)    (A_IDENT, id)  
  
--------------------------------------------------------------------    
  
To compile and test:  
gcc -o comp1 -g cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c  
  
./comp1 input06  
  
gcc -o out out.s  
  
./out  

## Part_7 Comparison  
This part we add some comparison operators, namely ==, !=, <, >, <= and >  
  
They correspond to new tokens  
T_EQ, T_NE, T_LT, T_GT, T_LE, T_GE  
  
And new AST node types  
A_EQ, A_NE, A_LT, A_GT, A_LE, A_GE,  
  
So when scanning, once we meet one '=', we need to check whether the next token is e.g. > or < or ..., if yes then we found our comparison tokens, if not then we put back the next char.  
  
New operator also have their precedence in AST. However I am confuse at this stage about why T_EQ, T_NE has higher precedence than T_STAR, T_SLASH, and why T_LT, T_GT, T_LE, T_GE are higher than T_EQ and T_NE....(the test input does not reflect the effects of these precedence I think)  
  
So when generating the AST, if the node type is the comparison operator, it will call assembly to generate the comparison result of two values (two registers from left and right sub-tree). The comparison result should be just 1 or 0 in the resulting register. Assembly with different instructions (sete, setne...) are used to deal with the six new comparison operators.  
  
To compile and test  
gcc -o comp1 -g cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c  
  
./comp1 input07  
  
gcc -o out out.s  
  
./out (where you should see each line of statement is executed, nine 1s as the result)  


