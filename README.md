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
  
# variable declaration:  
We firstly match 'int', then the identifier (adds into symbol table) and finally match the semicolon.  
  
# assignment statement  
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

## Part_8 IF statement  
This part develops the first control structure, the IF statements. Several things have changed  
  
--------------------------------------------------------------------  
Firstly, the BNF grammar extends to:  
  
 compound_statement: '{' '}'          // empty, i.e. no statement  
      |      '{' statement '}'  
      |      '{' statement statements '}'  
      ;  
  
 statement: print_statement  
      |     declaration  
      |     assignment_statement  
      |     if_statement  
      ;  
  
 print_statement: 'print' expression ';'  ;  
  
 declaration: 'int' identifier ';'  ;  
  
 assignment_statement: identifier '=' expression ';'   ;  
  
 if_statement: if_head  
      |        if_head 'else' compound_statement  
      ;  
  
 if_head: 'if' '(' true_false_expression ')' compound_statement  ;  
  
 identifier: T_IDENT ;  
  
--------------------------------------------------------------------    
  
We assume (for now) all the statements are now begin with { and end with }. Our main function will then just call compound_statement instead of statements before.  
  
We have new tokens, {, }, (, ), if, else. They are T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN, T_IF, T_ELSE. We also added T_PRINT for "print" keyword.  
  
--------------------------------------------------------------------  
  
One major change is AST now has 3 children. left, mid, right. This is for the comparison. The left is the condAST (the condition tree), mid is the true part compound statement, and right is the optional false part compound statement. So far, only when compare-true-false these 3 parts all exist, tree will have 3 children.  
  
-----------------------------------------  
  
Another big modification made is the gluing of trees. Previously we can have multiple trees, one tree for one statement. Now we convert those unrelated trees into a single big tree. We glue the tree from left to right, bottom to up.  
For example,  
  
 stmt1;  
 stmt2;  
 stmt3;  
 stmt4;  
they will be glued into a tree  
            A_GLUE  
               /  \  
          A_GLUE stmt4  
          /      \  
      A_GLUE    stmt3  
       /    \  
    stmt1  stmt2  
  
This conversion step is actually optional. As in the single tree, it will be interpreted from bottom to up, left to right, which is absolutely the same process with multiple trees. Hence the assembly code will be generated in correct order.  
  
The if statement will generate 3 trees. condition tree, true tree, false tree(could be NULL).  
  
And the big compound statement will just contain all the statements, namely print statement, var declaration, assignment statement, if statement. When it matches the rbrace, it means current compound statement is ended and the already-generated tree can be returned.  
  
--------------------------------------------------------------------  
  
There are also assembly generation function. The assembly for comparison will be cmpq which compares two register values. Different than getting a final value from a comparison as we previously did, we need a jump command like jge (jump if greater than), and a label to jump to. If there is no ELSE part, we only need to generate one label, which directs the code to the place after the whole IF statement. Like this:  
  
(the sample input)  
{  
  int i; int j;  
  i=6; j=12;  
  if (i < j) {  
    print i;  
  }  
}  
  
The assembly:  
  
	.comm	i,8,8  
	.comm	j,8,8  
	movq	$6, %r8  
	movq	%r8, i(%rip)  
	movq	$12, %r8  
	movq	%r8, j(%rip)  
	movq	i(%rip), %r8  
	movq	j(%rip), %r9  
	cmpq	%r9, %r8  
	jge	L1  
	movq	i(%rip), %r8  
	movq	%r8, %rdi  
	call	printint  
L1:  
	movl	$0, %eax  
	popq	%rbp  
	ret  
  
And if there does have the ELSE part, two labels need to be generated. One directs us to the ELSE (false) part, and one directs us to the code after IF statement when we finish the true part. Like this:  
  
(sample test)  
{  
  int i; int j;  
  i=6; j=12;  
  if (i < j) {  
    print i;  
  } else {  
    print j;  
  }  
}  
  
(generated assembly)  
  
	.comm	i,8,8  
	.comm	j,8,8  
	movq	$6, %r8  
	movq	%r8, i(%rip)  
	movq	$12, %r8  
	movq	%r8, j(%rip)  
	movq	i(%rip), %r8  
	movq	j(%rip), %r9  
	cmpq	%r9, %r8  
	jge	L1  
	movq	i(%rip), %r8  
	movq	%r8, %rdi  
	call	printint  
	jmp	L2  
L1:  
	movq	j(%rip), %r8  
	movq	%r8, %rdi  
	call	printint  
L2:  
	movl	$0, %eax  
	popq	%rbp  
	ret  
  
--------------------------------------------------------------------  
  
To compile:  
  
gcc -o comp1 cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c  
  
./comp1 input08/09  
  
gcc -o out out.s  
  
./out  

## Part_9 While statement  
This part implements the WHILE statement.  
The BNF grammar is  
  
while_statement: 'while' '(' true_false_expression ')'  compound_statement  ;  
  
We have the new token and AST node type  
T_WHILE, A_WHILE  
  
--------------------------------------------------------------------  
  
Building an AST for while statement is similar to if statement. The node type is A_WHILE. Left child is condition AST and right child is body AST.  
  
--------------------------------------------------------------------  
  
To generate assembly for while, we need 2 labels. One is the start label. The other is the end label. We use cmpq to get a comparison result and jump to end label if the condition does not hold. If the condition does hold, we need to jump back to the start label after finishing the compound statement inside the loop.  
  
--------------------------------------------------------------------  
  
We can see such an example  
  
{ int i;  
  i=1;  
  while (i <= 10) {  
    print i;  
    i= i + 1;  
  }  
}  
  
will generate assembly as follow  
  
	.comm	i,8,8  
	movq	$1, %r8  
	movq	%r8, i(%rip)		# i= 1  
L1:  
	movq	i(%rip), %r8  
	movq	$10, %r9  
	cmpq	%r9, %r8		# Is i <= 10?  
	jg	L2			# Greater than, jump to L2  
	movq	i(%rip), %r8  
	movq	%r8, %rdi		# Print out i  
	call	printint  
	movq	i(%rip), %r8  
	movq	$1, %r9  
	addq	%r8, %r9		# Add 1 to i  
	movq	%r9, i(%rip)  
	jmp	L1			# and loop back  
L2:  
  
--------------------------------------------------------------------  
  
To compile:  
  
gcc -o comp1 -g cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c  
  
./comp1 input10  
  
gcc -o out out.s  
  
./out  

## Part_10 For statements  
This part develops the FOR statements  
  
--------------------------------------------------------------------  
  
The BNF grammar for FOR loop is  
  
 for_statement: 'for' '(' preop_statement ';'  
                          true_false_expression ';'  
                          postop_statement ')' compound_statement  ;  
  
 preop_statement:  statement  ;        (for now)  
 postop_statement: statement  ;        (for now)  
  
--------------------------------------------------------------------  
  
So we will have to deal with 4 statements: pre_op, condition, post_op, and body. We certainly will have a new scanned token T_FOR, but are we going to build a AST with A_FOR ?  
  
Actually, a FOR statement can be transformed into a WHILE statement. For example:  
  
pre_op  
while (condition) {  
    body  
    post_op  
 }  
So we can just use while statemeht, and glue pre_op, condition, body, post_op together  
such as  
                    A_GLUE  
                    /     \  
                preop     A_WHILE  
                            /    \  
                        decision  A_GLUE  
                                  /    \  
                             compound  postop  
  
So we just use the WHILE statements.  
  
--------------------------------------------------------------------  
  
Another thing is, since FOR statement has (pre_op; condition; post_op), we cannot call compound_statement() to parse and build this as compound_statement() needs a } to end. So here we implement a function just to parse and build a single AST. The compound_statement() will call single_statement() each time in its statement fetching and AST gluing loop. compound statement still ends with a }, but we modify each statement process function that semicolon ';' is not matched inside the function. In compound_statement(), after we process ASSIGNMENT STATEMENT or PRINT STATEMENT, we match the semicolon.  
  
And in for_statement(), we firstly get pre_op statement, and match a ';', then we get condition tree, match a ';'. Next we get a post_op, we match a ). The body tree will just be got from compound_statement().  
  
--------------------------------------------------------------------  
  
To compile:  
gcc -o comp1 -g cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c  
  
./comp1 input11  
  
gcc -o out out.s  
  
./out  

## Part_11 Function 'main' declaration  
This part we begin to implement functions. It will be a long journey and this part we only declare a function and execute (assembly is generated for the function).  
  
--------------------------------------------------------------------  
  
BNF grammar for function:  
  
function_declaration: 'void' identifier '(' ')' compound_statement ;  
  
We have new token T_VOID and A_FUNCTION  
  
Our function do not have arguments so far, and return type will be void  
  
--------------------------------------------------------------------  
  
To parse and build AST for function, we firstly match the "void" keyword, then function name as an identifier (variable). Then left and right parenthesis. And finally the body compound statement. An unary node is made with A_FUNCTION, body compound statement tree and symbol table identifier slot number.  
  
--------------------------------------------------------------------  
  
Assembly code are generated before and after the function (need more understand). The function itself is generated by processing the body AST tree.  
  
--------------------------------------------------------------------  
  
To compile:  
gcc -o comp1 -g cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c  
  
./comp1 input12  
  
gcc -o out out.s  
  
./out  

## Part_12 Types  
The compiler from previous part only has 'INT' as type, this part we aim to add one more type, CHAR, and also prepare for future type additions.  
  
--------------------------------------------------------------------  
  
On x86-64, INT takes 4 bytes while CHAR is 1 byte. CHAR can be seen as an INT, but INT cannot be taken as CHAR. Hence if we encounter assignment like char j = 3333, we must give it a warning of type incompatible.  
  
In types.c, we have such a function checking the compatibility. So far we need to check the compatibility when EVALUATE EXPRESSION, PRINT STATEMENTS and in ASSIGNMENTS.  
  
When evaluating expressions, newly parsed expression tokens should have the compatible type with previous tokens. So in binexpr(), we need to check the compatibility.  
  
When printing statements. So far we only print CHAR or INT, so actually no type compatibility checking is necessary. Here we just check if the expression to be printed has type compatible to INT.  
  
When assigning values to variables, type checking is important. We allow CHAR type to be assigned to a INT variable, but not the other way around.  
  
Hence, in type_compatible() function, if any one type is P_VOID (corresponds to VOID return type), it is not compatible with the other. If both are the same type, free to go. If an CHAR is checked with INT, CHAR can be widen. But if an INT is assigned to CHAR (only happen in assignment statements, that is the function of onlyright parameter), they are not compatible and that CHAR will not be widen.  
  
--------------------------------------------------------------------  
  
We add a field "TYPE" into AST struct, we add 4 types in an enumerate: P_NONE, P_VOID (only with function), P_INT, P_CHAR. Types will be passed when making an AST node.  
  
P_NONE value indicates that the AST node does not represent an expression and has no type. For example when the node's operation is A_GLUE, A_PRINT, A_IF, A_WHILE, A_FOR.  
  
When building AST for assignment statements, expressions, printing statements, when we check the type compatibility between types from left and right sub-child, if one side needs to be widen, then that side of sub tree will be nested with a unary node with node type A_WIDEN.  
  
--------------------------------------------------------------------  
  
When generating assembly, we need to consider the types while generating a global symbol (allocating how many bytes for that in memory), load /initialise it (load the variable's value to a register) and store it (store value from register to variable).  
  
To compile and test:  
gcc -o comp1 -g cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c  
  
./comp1 input13  
  
gcc -o out out.s  
  
./out  

## Part_13 Functions Part 2  
In this part we extend previous ability on Functions. Beforehand, we were able to declare a  function structure. This time we can make a function call with a single argument and return from a non-void function (only). We also add a new type: LONG (8 bytes), aside from our previous CHAR(1 byte) and INT (4 bytes)  
  
--------------------------------------------------------------------  
  
At this stage, the BNF syntax for a function call is  
  
  function_call: identifier '(' expression ')'   ;  
  
Only one parameter is supported at this moment and that parameter is not really passed into function yet. We just want to show that we can now do the function call with that grammar:  
e.g. fred(5)  
  
According to the BNF above, parsing function calls is easy. We firstly parse the function name as the identifier, then (, then argument expression, then ).  The function call AST will be an unary node with node type A_FUNCCALL and function's return type as type.  
  
We can now have expression like x = fred + jim and x = fred(5) + jim. Both are assignment statement but the first fred is identifier while the second is function call. We hence need to add a function call case when parsing assignment statement.  
  
--------------------------------------------------------------------  
  
We then deal with result of function. A function can return a value or not. We firstly parse a return statement. Its BNF is like:  
  
return_statement: 'return' '(' expression ')'  ;  
  
The parsing is easy as well. We also have to check the compatibility between function's type and returned type. We made a global variable Functionid to record the function we are currently in. The return statement itself is a unary node with node type being A_RETURN and type being P_NONE.  
  
We need to ensure that a non-void function return a value. Note that function's body are statements which will be glued together in a final compound statement tree. According to our glueing algorithm, the final statement will appear at the top right of the tree. Hence when parsing function declaration, we need to check the top right node has the type A_RETURN if function is not void.  
  
Note that at this stage we do not support statement which solely calls a void function.  
  
--------------------------------------------------------------------  
  
Since a new type, LONG has been added. Our method of check type compatibility is now based on the size of the type. See type_compatible().  
  
--------------------------------------------------------------------  
  
At last, assembly code is generated for calling and returning from a function. In x64, calling a function requires loading the (first) argument into %rdi, returned value from a function can be read from %rax. We made corresponding changes when moving data between registers and variables with respect to CHAR, INT and LONG.  
  
--------------------------------------------------------------------  
  
To compile:  
  
gcc -o comp1 -g cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c  
  
./comp1 input14  
  
gcc -o out out.s lib/printint.c (link assembly output with lib/printint.c)  
  
./out  
 
## Part_14 Pointer (Part 1)  
This part we add pointer. We skip generating assembly for ARM platform for now.  
  
We want to:  
1. Declaration of pointer variables  
2. Assignment of an address to a pointer  
3. Dereferencing a pointer to get the value it points  
  
--------------------------------------------------------------------  
  
We add new token T_AMPER for '&'. (for * it is just stars)  
  
And we also add 4 more pointer types: P_VOIDPTR, P_CHARPTR, P_INTPTR, P_LONGPTR
, and 2 new node types (operations): A_ADDR, A_DEREF
We now need to have a pointer_to() and a value_at() function to switch between the original type and pointer type.  
  
In parse_type(), aside from parsing normal types, we scan in one or more further '*' tokens (each ) before the identifier, this allows user to have like char *****fred (but actually not allowed as every time a * is scanned, pointer_to() is called. But we cannot switch from like P_INTPTR to P_INT for now. )  
  
--------------------------------------------------------------------  
  
With pointers, our expression can be made of elements with prefix * or &. Those elements can be seen as prefix_expression, whose BNF grammar looks like this:  
  
prefix_expression: primary  
        | '*' prefix_expression  
        | '&' prefix_expression  
        ;  
  
Technically this also allows  
  
x = ***y;  
a = &&&b;  
  
But currently we do not support multiple * and &. Hence, we have a prefix() function which scans * and & before getleft() scans the identifier. If we have a &ident, the ident node's type will be changed from A_IDENT to A_ADDR. If *ident, we make a new unary node with A_DEREF as node type. If no * nor & exists, in prefix() we will just call getleft(), to parse the normal literals or identifiers  
  
--------------------------------------------------------------------  
  
We generate corresponding assembly when we meet A_ADDR or A_DEREF nodes. We use leaq to load the address of a variable into a register, and use (%r) to get the value %r points to (r contains the address) into register r.  
  
--------------------------------------------------------------------  
  
To compile and test:  
  
gcc -o comp1 -g -Wall cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c  
  
./comp1 input15.c  
  
gcc -o out out.s lib/printint.c  
  
./out  

## Part_15 Global variables  
This part we aim to declare global variables properly. Previously, only thing we can do is declare variables. We also can declare multiple vars at the same time (e.g. in a same line, seperated by comma). Like this:  
  
```
int x, y, z;  
```  
  
Here is the new BNF grammar for global declarations, both functions and variables:  
```
global_declarations : global_declarations 
      | global_declaration global_declarations
      ;

 global_declaration: function_declaration | var_declaration ;

 function_declaration: type identifier '(' ')' compound_statement   ;

 var_declaration: type identifier_list ';'  ;

 type: type_keyword opt_pointer  ;
 
 type_keyword: 'void' | 'char' | 'int' | 'long'  ;
 
 opt_pointer: <empty> | '*' opt_pointer  ;
 
 identifier_list: identifier | identifier ',' identifier_list ;
```  
We now have a new token T_COMMA, indicating ","  
  
--------------------------------------------------------------------  
  
Two places that need to have major changes. 
  
Firstly, we need to have a function called **global_declarations()**, it has a while loop in which multiple vars and/or functions are defined.  
  
Secondly, in the **var_declaration()**, a while loop is also needed to parse multiple identifiers. When we have T_SEMI after one identifier is parsed, if the next token is T_COMMA, we continue the loop, if the next token is T_SEMI, the loop is aborted.  
  
To compile and test:  
gcc -o comp1 -g -Wall cg.c decl.c expr.c gen.c main.c misc.c scan.cstmt.c sym.c tree.c types.c  
  
--------------------------------------------------------------------  
  
./comp1 input16.c  
  
gcc -o out out.s lib/printint.c  
  
./out  

## Part_16 Modify type checking and Pointer Offset  
This part we have a new type checing function **modify_type()**. It looks a bit messy now and not the best versions we can have. But it can support our needs and simplify code in **assignment_statement()**, **return_statement()**, **print_statement()** and **binexpr()**.  
  
--------------------------------------------------------------------   
  
Pointers offset allow us to do: &c + 1, or *(p + 1)...  
To achieve this, we need to deal with the case when we meet A_SCALE in **genAST()**. We load an int which is the offset value and multiply with the register containing the size of the identidier. For offset equaling power of 2, we perform optimisations by shifting the bits.  

## Part_17 Lvalues and Rvalues Revisited  
This part makes some modifications to Lvalues and Rvalues in assignments. In essential, we want to be able to do:  
```
a=b=3;
*p = 16;

```
which are not currently supported.  
  
--------------------------------------------------------------------  
  
The names lvalue and rvalue come from the two sides of an assignment statement: lvalues are on the left, rvalues are on the right. Previously when we have statement like `a=b`, we treat right-hand side as `LVIDENT` whereas left-hand side as expression. It does not fit with our new assignments shown above.  
  
We do the following changes. Firstly, we treat assignment statements as expressions. This means that we do not parse left and right side of the `=` seperately but as a whole expression. `=` is now an operand but with lower precedence than `+` and `-`... (add a new precedence value for =). Secondly, we need to ensure right-associativity for assignments. It means that operator binds more tightly to the expression on the right than to the left (See comments and tutorials, for example a=b=3). Thirdly, and also for right assiciativity, we need to swap the left and right tree when parsing the assignment expression, because we need to generate the assembly code for the right-hand rvalue bedore the code for the left-hand value (also see above example).  
  
We also write a new function which allows us to visualise the AST, the `dumpAST()`. We also get rid of `print` statement for now, as well as multiple variables (those seperated by comma).  
  
--------------------------------------------------------------------  
  
To compile and test:

gcc -o comp1 -g -Wall cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c  
  
./comp1 input18.c (or input18a.c)  
  
gcc -o out out.s lib/printint  
  
./out  

## Part_18 Array  
This part, we implement features of array. We aim to be able to have those following declarations workable.  
```
int ary[5];               // Array of five int elements
  int *ptr;                 // Pointer to an int

  ary[3]= 63;               // Set ary[3] (lvalue) to 63
  ptr   = ary;              // Point ptr to base of ary
  // ary= ptr;              // error: assignment to expression with array type
  ptr   = &ary[0];          // Also point ptr to base of ary, ary[0] is lvalue
  ptr[4]= 72;               // Use ptr like an array, ptr[4] is an lvalue
```  
Array is contigious of memory blocks with same type of data. Array name can be the pointer to the base address of the array. The elements of array is mutable, but the base address of the array is not mutable.  
  
--------------------------------------------------------------------  
  
Firstly we have a new field in the symtable struct: size. It means the number of elements in the symbol. If the symbol is an array name, size is the number of elements in array. If the symbol is a single identifier, size = 1. If symbol is function's name, size = 0.  
  
Secondly, to declare an array with certain size, we have such BNF grammar:  
```
variable_declaration: type identifier ';'
        | type identifier '[' P_INTLIT ']' ';'
        ;
```  
Hence when we meet an identifier, if there is an `[` after that, it means we have an array declaration. In `var_declaration()`, we process the array by adding its name into symbol table. Note that the type of such symbol is the pointer type. Then, we allocate the spaces in assembly (`cgglobsym(id)`) for the number of elements.  
  
Thirdly, we need to access array by indexes. In `expr.c` we have a new `array_access()` function which parses array's symbol as left tree whose type is A_ADDR, and parses index value (as expression) as right tree. After type checking (check that index expression is an int type, and array's type is pointer type, A_SCALE tree will be returned which indicates pointer's base address will be scaled to the indexed position), A_ADD tree will be built to connect left and right sub-tree, and finally A_DEREF tree on top as we aim to get the element of that address.  
  
Fourthly, to deal with expression like `*(ptr + 2)`, we need to parse parenthesis in expression. This part is done in `getleft()` in `expr.c`  
  
--------------------------------------------------------------------  
  
To compile and test:  
gcc -o comp1 -g -Wall cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c  
  
./comp1 input20.c  
  
gcc -o out out.s lib/printint.c  
  
./out  
  
## Part_19 Character and String Literals  
This part we make character and string literals definition available.  
  
We make a new token for String Literals. T_STRLIT. For characters, as they can just be treated as integer, nothing added. We will scan a string literal into Text buffer.  
  
After we can recognise string literals, we need to be aware that an expression can have string literal as element. By looking at [BNF Grammar for C](https://www.lysator.liu.se/c/ANSI-C-grammar-y.html)
written by Jeff Lee in 1985,
```
primary_expression
        : IDENTIFIER
        | CONSTANT
        | STRING_LITERAL
        | '(' expression ')'
        ;
```
We need to modify `getleft()` in `expr.c`. To have a unary node with node type **A_STRLIT**, and type **P_CHARPTR** (as string is an array of chars) for a string literal. At this time we need to allocate spaces for each characters in the string. We also need a label at the beginning, which refers to the base address of the string. In `genAST()`, when we meet **A_STRLIT**, we load the (base) address which is contained in label into a new register, using `leaq`.  
  
--------------------------------------------------------------------  
  
To compile and test:  
gcc -o comp1 -g cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c  
  
./comp1 input21.c  
  
gcc -o out out.s lib/printint.c  
  
./out  
  
## Part_20: More operators  
We add some more operators in this part.  
  
| Scanned Input | Token |
|:-------------:|-------|
|   <code>&#124;&#124;</code>        | T_LOGOR |
|   `&&`        | T_LOGAND |
|   <code>&#124;</code>         | T_OR |
|   `^`         | T_XOR |
|   `<<`        | T_LSHIFT |
|   `>>`        | T_RSHIFT |
|   `++`        | T_INC |
|   `--`        | T_DEC |
|   `~`         | T_INVERT |
|   `!`         | T_LOGNOT |
  
Some operators are prefix operators. Such as `T_MINUS`, `T_INVERT`, `T_LOGNOT`, `T_INC`, `T_DEC`. We need to deal with them in `prefix()` in `expr.c`. In addition, 3 opertors will be apply on rvalues (they will not appear at the left hand side of the =), namely `T_MINUS`, `T_INVERT`, `T_LOGNOT`.  
  
Similarly, `T_INC` and `T_DEC` can be postfix operators. We will deal with that in a function called `postfix()`, Introducing function call and array access will also move into `postfix()`.  
  
Meanwhile, in comparison (if and while, for statements), we convert an integer expression to a boolean value, so that the register will be set to 0 or 1.  
  
--------------------------------------------------------------------  
  
To compile and test:  

gcc -o comp1 -g cg.c decl.c expr.c gen.c main.c misc.c scan.c stmt.c sym.c tree.c types.c  
  
./comp1 tests/input22.c  
./comp1 tests/input23.c  
./comp1 tests/input24.c  
  
gcc -o out out.s lib/printint.c  
  
./out

  

  



