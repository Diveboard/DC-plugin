LIB_DEPS=ir*.c moschip.c

all: lib

lib:
	gcc -dynamiclib $(LIB_DEPS) -l iokit -framework CoreFoundation -o irda.dylib
	rm -f libirda.dylib
	ln -s irda.dylib libirda.dylib
clean:
	rm -fr *.dylib irda.dylib.dSYM *.o
