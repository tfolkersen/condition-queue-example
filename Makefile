CC := g++
FLAGS := -Wall -lpthread

a: clear unordered.o
	./unordered.o

b: clear ordered.o
	./ordered.o

%.o: %.cpp
	$(CC) $< -o $@ $(FLAGS)

clean:
	rm *.o

c: clean

clear:
	clear
