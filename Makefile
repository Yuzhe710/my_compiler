HSRCS= scan.h decl.h defs.h
SRCS= cg.c decl.c expr.c gen.c main.c misc.c \
	scan.c stmt.c sym.c tree.c types.c

comp1mac: $(SRCS) $(HSRCS)
	gcc-12 -o mycompiler -g -Wall $(SRCS)

mycompiler: $(SRCS) $(HSRCS)
	gcc -o mycompiler -g -Wall $(SRCS)

clean:
	rm -f mycompiler tests/*.o tests/*.s 

test: mycompiler tests/runtests
	(cd tests; chmod +x runtests; ./runtests)
