MAKE = make TESTING=$(TESTING)
EXEC = crawler
CFLAGS = -Wall -pedantic -std=c11 -ggdb $(TESTING) -I../lib -I../set
CC = gcc
MAKE = make
# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all
LIBS = ../libcs50/libcs50-given.a ../common/commonlib.a

OBJS = crawler.o

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

.PHONY: clean

valgrind: $(EXEC)
	$(VALGRIND) ./$(EXEC)


test: $(OBJS)
	bash testing.sh

clean:
	rm -f *~ *.o
	rm -f $(EXEC)
