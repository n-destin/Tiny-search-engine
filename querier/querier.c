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
    int page;
    int rank;
    char* URL;
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

void count_elements(void* arg, const int key, const int item){
    int* count = (int*) arg;
    *count += 1;
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
    // printf("%d  %d\n", count, counters_get(counters_union, key));
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

void append(void* arg, int key, int count)
{
    int* array = (int*) arg;
    int index = array[0] + 1;
    array[index] = key;
    array[index + 1] = count;
    array[0] = index + 1;
}

int* get_results_array(counters_t* results)
{
    int* counters_array = malloc(MAX_QUERY_LENGTH * sizeof(int));
    if (counters_array == NULL) {
        return NULL; // Handle malloc failure
    }
    counters_array[0] = 0;
    counters_iterate(results, counters_array, append);
    return counters_array;
}

int compareByRank(const void *a, const void *b) {
    searchResult_t *resultB = (searchResult_t *)b;
    searchResult_t *resultA = (searchResult_t *)a;
    return (resultA->rank - resultB->rank);
}

char* get_url(int page, const char* pageDirectory) {
    char filename[16];
    sprintf(filename, "%d", page);
    char* pathname = get_pathname(pageDirectory, filename);
    FILE* file = fopen(pathname, "r");
    if (file == NULL) {
        printf("here error jappend");
        fprintf(stderr, "Failed to open file");
        return NULL;
    }

    char* URL = malloc(1024);
    URL = file_readLine(file);
    fclose(file);
    return URL;
}



// Function to convert the array into search results and sort them
searchResult_t* getSortedResults(int* counters_array, int array_length, const char* pageDirectory) {
    int n = array_length * 2; // Calculate the number of search results
    searchResult_t* results = malloc(n * sizeof(searchResult_t));
    if (results == NULL) {
        return NULL; // Handle memory allocation failure
    }
    for (int i = 1; i <= n; i+=2) {
        results[i].page = counters_array[i];
        results[i].rank = counters_array[i+1];
        results[i].URL = get_url(results[i].page, pageDirectory);
    }
    qsort(results, n, sizeof(searchResult_t), compareByRank);

    return results;
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
                word_normalize(word);
                words[num_words++] = word;
            }
            word = strtok(NULL, " \n\t");
        }
        // for(int i = 0; i<num_words; i++){
        //     printf("word: %s \n", words[i]);
        // }        
        FILE* file = fopen("./named", "w");
        //counters_t * something = index_find f(indexLoad, "dartmouth");
        counters_t *results = or_sequence(indexLoad, words, num_words); // got the counter - unsorted
        int num_results = 0;
        counters_iterate(results, &num_results, count_elements);
        // printf("The count is: %d", num_results);
        // Helper function to compare two search results by rank
        int* results_array = get_results_array(results);
        searchResult_t* sorted_results = getSortedResults(results_array, num_results, pageDirectory);
        for(int i = 0; i< num_results; i++){
            printf("%d %d %s\n", sorted_results[i].page, sorted_results[i].rank, sorted_results[i].URL);
        }
        // for(int i = 0; i< MAX_QUERY_LENGTH; i++){
        //     printf("%d", results_array[i]->key);
        // }
        // sorting the results
        // counters_iterate(results, )
        counters_print(results, file);
        fclose(file);
        // counters_delete(results);
    }
    // index_delete(indexLoad);
    return 0;
}
