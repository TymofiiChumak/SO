build:
	gcc main.c -o main -pthread -lm

build_filter:
	gcc filter_gen.c -o filter_gen

test_main:
	./filter_gen 5
	./main 1 dla.ascii.pgm filter.txt res.ascii.pgm
	./main 2 dla.ascii.pgm filter.txt res.ascii.pgm
	./main 4 dla.ascii.pgm filter.txt res.ascii.pgm
	./main 8 dla.ascii.pgm filter.txt res.ascii.pgm
	./filter_gen 15
	./main 1 dla.ascii.pgm filter.txt res.ascii.pgm
	./main 2 dla.ascii.pgm filter.txt res.ascii.pgm
	./main 4 dla.ascii.pgm filter.txt res.ascii.pgm
	./main 8 dla.ascii.pgm filter.txt res.ascii.pgm
	./filter_gen 45
	./main 1 dla.ascii.pgm filter.txt res.ascii.pgm
	./main 2 dla.ascii.pgm filter.txt res.ascii.pgm
	./main 4 dla.ascii.pgm filter.txt res.ascii.pgm
	./main 8 dla.ascii.pgm filter.txt res.ascii.pgm
