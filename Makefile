CC= icc -O3 -qopenmp -std=c99 -g
LIB = -lm

.PHONY: all
all:	build/bin/loops2
	

build/bin/loops2: src/loops2.c 
	$(CC) $^ -o $@

build/queue.o: src/queue.c include/queue.h
	$(CC) -c $< -o $@
	

.PHONY:run
run:
	./build/bin/loops2

.PHONY: clean
clean:
	rm build/bin/loops2 
	rm build/queue.o
