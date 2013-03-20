all:
	gcc -dynamiclib *.c -l iokit -framework CoreFoundation -o irda.dylib
	rm -f libirda.dylib
	ln -s irda.dylib libirda.dylib
clean:
	rm -fr *.dylib irda.dylib.dSYM *.o
