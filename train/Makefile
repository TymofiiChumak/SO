main:
	gcc -fPIC -c  source.h source.c 
	gcc -shared -fPIC -Wl,-soname,libsource.so.1 \
		-o libsource.so.1.0.0 source.o -lc 
	ln -s libsource.so.1.0.0 libsource.so.1
	ln -s libsource.so.1 libsource.so
	ln -s /usr/lib/libsource.so.1.0.0 /usr/lib/libsource.so.1

clean:
		rm -f libsource.so.1
	rm -f libsource.so
	rm -f /usr/lib/libsource.so.1