.PHONY: all clean

all: test

test: condition.o main.o threadpool.o
	gcc $^ -o $@ -lpthread -lrt

%.o:%.c
	gcc -c $^ -o $@

clean:
	rm -rf *.o test 
