build1:
	gcc -c util.c
	gcc server.c -o server
	gcc client.c -o client

build2:
	gcc -c util1.c
	gcc -lrt server1.c -o server
	gcc -lrt client1.c -o client

runclient:
	gcc runclient.c -o runclient