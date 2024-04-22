# Makefile for CS50 library
#   Builds and compiles all these utility functions into a library.
#
# David Kotz, April 2016, July 2017, April 2019, 2021
# updated by Xia Zhou, July 2016

# object files, and the target library
OBJS = bag.o counters.o file.o hashtable.o hash.o mem.o set.o webpage.o
LIB = libcs50.a

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(FLAGS)
CC = gcc
MAKE = make

# Build $(LIB) by archiving object files
# (you will need to drop in copy of set.c, counters.c, hashtable.c)
$(LIB): $(OBJS)
	ar cr $(LIB) $(OBJS)

# Dependencies: object files depend on header files
bag.o: bag.h
counters.o: counters.h
file.o: file.h
hashtable.o: hashtable.h set.h hash.h 
hash.o: hash.h
mem.o: mem.h
set.o: set.h
webpage.o:  webpage.h

.PHONY: clean sourcelist

# list all the sources and docs in this directory.
# (this rule is used only by the Professor in preparing the starter kit)
sourcelist: Makefile *.md *.c *.h
	@echo $^

# clean up after our compilation
clean:
	rm -f core
	rm -f $(LIB) *~ *.o
