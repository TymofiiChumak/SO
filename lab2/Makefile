first:
	gcc -c 1.c
	gcc 1.o -o first


second1:
	gcc -c 2-1.c
	gcc 2-1.o -o second 

second2:
	gcc -c 2-2.c
	gcc 2-2.o -o second 


testFirst:
	./first generate f1.txt 100000 8192 sys
	./first copy f1.txt f2.txt 100000 8192 sys 
	./first sort f1.txt 100000 8192 sys 
	./first sort f1.txt 100000 8192 lib

	