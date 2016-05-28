CC = clang
CFLAGS = -O0 -Wall -Wextra -lpthread -ansi -pedantic -g

all: exec

exec: main.o
	$(CC) $(CFLAGS) -o exec main.o
 
main.o: main.c
	$(CC) $(CFLAGS) -c main.c
 
clean:
	rm -f main.o exec
