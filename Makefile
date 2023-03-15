HSRCS= scan.h decl.h defs.h
SRCS= cg.c decl.c expr.c gen.c main.c misc.c \
	scan.c stmt.c sym.c tree.c types.c

comp1mac: $(SRCS) $(HSRCS)
	gcc-12 -o comp1 -g -Wall $(SRCS)

comp1: $(SRCS) $(HSRCS)
	gcc -o comp1 -g -Wall $(SRCS)

clean:
	rm -f comp1 *.o *s out

test: comp1 test2/runtests
	(cd test2; chmod +x runtests; ./runtests)
