CC = gcc
CFLAGS = -D_REENTRANT -lpthread -Wall -pthread

all: parque

Parque: parque.c
	$(CC) parque.c -o parque $(CFLAGS)

clean:
	rm parque
