MAKE = make TESTING=$(TESTING)

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(TESTING) -I../lib -I../set
CC = gcc
MAKE = make
# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

LIBS = ../libcs50/libcs50-given.a
LIB = commonlib.a
OBJS = pagedir.o

$(LIB): $(OBJS)
	ar cr $(LIB) $(OBJS)

pagedir.o: pagedir.h

clean:
	rm -f *~ *.o
	rm -f $(EXEC)
