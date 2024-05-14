CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb -I../lib -I../common
# Linker flags and libraries
LIBS = ../common/commonlib.a ../libcs50/libcs50-given.a
# LDLIBS = -lcs50 -lcommon
# Executable name
EXEC = querier
# Object files
OBJS = querier.o ../common/pagedir.o ../common/word.o ../common/index.o
# ITOBJS = indextest.o ../common/pagedir.o 
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o $@

querier.o: querier.h ../common/pagedir.h ../common/word.h ../common/index.h ../libcs50/file.h
pagedir.o: ../common/pagedir.h
word.o: ../common/word.h
index.o: ../common/index.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean valgrind test

clean:
	rm -f *~ *.o
	rm -f $(EXEC)

valgrind: $(EXEC)
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXEC)

test: $(EXEC)
	bash testing.sh
