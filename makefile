CFLAGS= -pthread -std=gnu99
CC= gcc $(CFLAGS)

all: main
program: main.o
main.o: main.c 

clean:
	rm -f main main.o
run: hr-timer
	./main
