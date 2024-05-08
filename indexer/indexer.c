/*
    Destin Niyomufasha
    CS 50 indexer.c file tse
    Implements the indexer
    to run: make 
    ./indexer [directory to files produced by crawler] [path to the file for the indexer]

*/


#include <stdio.h>
#include <stdlib.h>
#include "../common/pagedir.h"
#include "../common/word.h"
#include "../common/pagedir.h"
#include "../common/index.h"


int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: ./indexer pageDirectory indexFilename\n");
        return 1;
    }

    char* pageDirectory = argv[1];
    char* indexFilename = argv[2];

    if (pagedir_validate(pageDirectory) != 0) {
        fprintf(stderr, "Invalid directory: %s\n", pageDirectory);
        return 2;
    }

    index_t* index = index_new(700);
    if (index == NULL) {
        fprintf(stderr, "Failed to create index.\n");
        exit(1);
    }

    index_build(pageDirectory, index);
    index_save(indexFilename, index);
    index_delete(index);

    return 0;
}