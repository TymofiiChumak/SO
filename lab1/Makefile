optimization := -O0

loadLibStatic:
	gcc -c charBlocks.h 
	gcc -c $(optimization) charBlocks.c
	ar rcs libcharBlocks.a charBlocks.o
	
loadLibDynamic:
	gcc -fPIC -c $(optimization) charBlocks.h charBlocks.c 
	gcc -shared -fPIC -Wl,-soname,libcharBlocks.so.1 \
		-o libcharBlocks.so.1.0.0 charBlocks.o -lc 
	rm -f libcharBlocks.so.1
	rm -f libcharBlocks.so
	rm -f /usr/lib/libcharBlocks.so.1
	ln -s libcharBlocks.so.1.0.0 libcharBlocks.so.1
	ln -s libcharBlocks.so.1 libcharBlocks.so
	ln -s /usr/lib/libcharBlocks.so.1.0.0 /usr/lib/libcharBlocks.so.1

loadMainStatic:
	make loadLibStatic
	gcc -c main.c 
	gcc main.o libcharBlocks.a -o main

loadMainDynamic:
	make loadLibDynamic
	gcc main.c -lcharBlocks  -L ./ -o main


loadMainDyn:
	gcc -c maindyn.c
	gcc -ldl maindyn.o -o maindyn



testDynamic:
	./main dynamic create_table 1000000 1000000 search_element 10000 add 100000
	./main dynamic create_table 1000000 1000000 remove 100000 remove_and_add 100000

testStatic:
	./main static create_table 10000 100000 search_element 1000 add 1000
	./main static create_table 10000 100000 remove 1000 remove_and_add 10000

testOptimization:
	echo "optimization level Os" >> raport2.txt
	make optimization="-Os" loadMainDynamic
	make testDynamic testStatic
	echo "optimization level O2" >> raport2.txt
	make optimization="-O2" loadMainDynamic
	make testDynamic testStatic
	echo "optimization level O0" >> raport2.txt
	make optimization="-O0" loadMainDynamic
	make testDynamic testStatic