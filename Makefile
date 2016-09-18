CLAGS=-g
PFLAGS= -lsimlib -lm
 
CC = g++

all: compile

compile: main.cpp
	$(CC) $(CFLAGS) -o pivovar main.cpp AverageValue.h MaterialStore.h $(PFLAGS)

run:
	./pivovar

clean:
	rm -f pivovar

rebuild: clean all

