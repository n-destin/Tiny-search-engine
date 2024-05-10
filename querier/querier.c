/*
Destin Niyomnufasha
CS 50
To use run: ./querier [pageDirectory] []
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../libcs50/file.h"
#include "../libcs50/webpage.h"
#include "../libcs50/hashtable.h"
#include "../libcs50/counters.h"
#include "../libcs50/mem.h"
#include "../common/index.h"
#include "../common/pagedir.h"
#include "../common/word.h"


#define MAX_QUERY_LENGTH 1024
#define MAX_WORD_LENGTH 100

counters_t* counters_intersection(counters_t* counters_one, counters_t* counters_two);
void counters_union(counters_t * counters_one, counters_t* counters_two);
typedef struct index {
    hashtable_t *ht;
} index_t;


typedef struct searchResult{   
    char * page;
    int count;
} searchResult_t;


typedef struct {
    counters_t* counters_two;
    counters_t* intersection;
} counters_args_t;



void validate_args(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: ./querier pageDirectory indexFilename\n");
        exit(1);
    }

    FILE* opened = fopen(argv[2], "r");
    if(opened == NULL){
        fprintf(stderr, "Error opening file %s", argv[2]);
        exit(1);
    }

    fclose(opened);

    if (pagedir_validate(argv[1]) != 0) {
        fprintf(stderr, "Invalid pageDirectory %s cannot be read\n", argv[1]);
        exit(1);
    }
}

void printsomething(void* arg, const int key, const int item){
    printf("I reached here");
    printf(arg, "%d %d \n", key, item);
}

void copy(void* arg, const int key, const int count)
{
    counters_t* counter = (counters_t * ) arg;
    counters_set(counter, key, count);
}


counters_t* and_sequence(index_t * index, char **words, int start, int end) {
    counters_t* result = (counters_t*) index_find(index, words[start]); // got the documents with the word
    if (result == NULL) {
        return NULL;
        };
    counters_t * intersection = counters_new();
    counters_iterate(result, intersection, copy);
    for (int i = start + 1; i <= end; i++) {
        if(strcmp(words[i], " ") != 0){
            counters_t *ct = index_find(index, words[i]);
            if (ct == NULL) {
                return NULL;
            };
            intersection = counters_intersection(intersection, ct);
        }
    }
    return intersection;
}


counters_t* or_sequence(index_t *index, char **words, int num_words) {
    counters_t *result = counters_new();
    if(result == NULL){
        exit(1);
    }
    FILE* opened = fopen("./naming", "w");
    int i = 0;
    while (i < num_words) {
        int start = i;
        while (i < num_words && strcmp(words[i], "or") != 0) i++;
        counters_t* and_result = and_sequence(index, words, start, i - 1);
        if (and_result != NULL) {
            counters_union(and_result, result);
        }
        i++;
    }
    counters_print(result, opened);
    fclose(opened);
    return result;
}

void counters_union_helper(void * arg, int key, int count)
{   
    FILE* file = fopen("./naming", "w");
    counters_t * counters_union = (counters_t*) arg;
    printf("%d  %d\n", count, counters_get(counters_union, key));
    int new_count = counters_get(counters_union, key) + count;
    counters_set(counters_union, key, new_count);
    counters_print(counters_union, file);
    fclose(file);
}


void counters_union(counters_t * counters_one, counters_t* counters_two){
    counters_iterate(counters_one, counters_two, counters_union_helper);
}

void add_result(void* arg, const int key, const int count1) {
    counters_args_t* args = (counters_args_t*) arg;
    int count2 = counters_get(args->counters_two, key);
    // printf("%d %d %d \n", key, count1, count2);
    // if both are present
    if (count1 > 0 && count2 > 0) {
        int minimum = count1;
        if(count1>count2){
            minimum = count2;
        }
        counters_set(args->intersection, key, minimum);
    }
}

void print_counters(counters_t * counter, int key, int count){
    printf("%d, %d", key, count);
}

counters_t* counters_intersection(counters_t* counters_one, counters_t* counters_two) {
    counters_t* intersection = counters_new();
    if (intersection == NULL){
        fprintf(stderr, "Out of memory");
        exit(1);
    };
    counters_args_t* args = malloc(sizeof(counters_args_t));
    args->counters_two = counters_two;
    args->intersection = intersection;
    counters_iterate(counters_one, args, add_result);
    return args->intersection;
}


int main(int argc, char *argv[]) {
    validate_args(argc, argv);

    char *pageDirectory = argv[1];
    char *indexFilename = argv[2];
    FILE* opened = fopen(indexFilename, "r");
    int numLines = file_numLines(opened);
    file_numLines(opened);
    fclose(opened);

    index_t * indexLoad = index_new(numLines);

    index_load(indexFilename, indexLoad);
    index_save("./laoded", indexLoad);
    if (indexLoad == NULL) {
        fprintf(stderr, "Error loading index from %s\n", indexFilename);
        exit(2);
    }

    char query[MAX_QUERY_LENGTH];
    while (fgets(query, MAX_QUERY_LENGTH, stdin) != NULL) {
        // char** someting 
        char* words[MAX_QUERY_LENGTH / 2 + 1]; // when we remove spaces
        int num_words = 0;
        char *word = strtok(query, " \n\t");
        while (word != NULL) {
            // printf("%s \n", word);
            if(strcmp(word, "and") != 0){
                words[num_words++] = word;
            }
            word = strtok(NULL, " \n\t");
        }

        // for(int i = 0; i<num_words; i++){
        //     printf("word: %s \n", words[i]);
        // }        
        FILE* file = fopen("./named", "w");
        //counters_t * something = index_find f(indexLoad, "dartmouth");
        counters_t *results = or_sequence(indexLoad, words, num_words);
        counters_print(results, file);
        fclose(file);
        counters_delete(results);
    }
    // index_delete(indexLoad);
    return 0;
}
