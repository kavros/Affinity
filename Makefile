CC= icc -O3 -qopenmp -std=c99 -g
LIB = -lm

.PHONY: all
all:	build/bin/loops2
	
	

build/bin/loops2: src/loops2.c build/affinity.o
	$(CC) $^ -o $@

build/affinity.o: src/affinity.c include/affinity.h
	$(CC) -c $< -o $@
	

.PHONY:run
run:
	./build/bin/loops2

.PHONY: clean
clean:
	rm build/bin/loops2 
	rm build/queue.o
