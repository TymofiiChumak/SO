buildSystemV:
	gcc -c util.c
	gcc golibroda.c -o golibroda 
	gcc clients.c -o clients

buildPOSIX:
	gcc -lrt -pthread golibroda1.c -o golibroda 
	gcc -lrt -pthread clients1.c -o clients