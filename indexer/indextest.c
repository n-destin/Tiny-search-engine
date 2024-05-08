/*
    Destin Niyomufasha
    CS 50 indextest.c file tse
    Reads an indexer produced by indexer.c into a new file
    to run: make indextest
    then 
    ./indextest [oldfilename] [newfilename]
 
*/


#include <stdio.h>
#include <stdlib.h>
#include "indextest.h"
#include "../common/pagedir.h"
#include "../common/word.h"
#include "../common/pagedir.h"
#include "../common/index.h"


int 
main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Not enough number of arguments\n");
        exit(1);
    }

    FILE* oldFilename = fopen(argv[1], "r");
    if(oldFilename == NULL){
        fprintf(stderr, "The file %s could not be opened \n", argv[1]);
        exit(1);
    }

    
    FILE* newFilename = fopen(argv[2], "w");
    if(newFilename == NULL){
        fprintf(stderr, "The file %s cannot be opened", argv[2]);
        exit(2);
    }


    int num_lines = file_numLines(oldFilename);
    index_t* new_index = index_new(num_lines);
    fclose(oldFilename);
    if(new_index == NULL){
        fprintf(stderr, "cannot initialize a new index");
        exit(1);
    };

    index_load(argv[1], new_index);
    index_save(argv[2], new_index);
    index_delete(new_index);

    return 0;
}

