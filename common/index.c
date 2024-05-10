/*
    Destin Niyomufasha
    CS 50 index.c file tse
    Containing the helper functions for indexer

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "index.h"
#include "../libcs50/webpage.h"
#include "pagedir.h"
#include "word.h"
#include "../libcs50/counters.h"
#include "../libcs50/hashtable.h"
#include "../libcs50/file.h"
#include <ctype.h>

// Declare Structs used

typedef struct webpage {
  char* url;                
  char* html;                           
  size_t html_len;               
  int depth;                              
} webpage_t;

typedef struct index {
    hashtable_t *ht;  
} index_t;

// Forward declaration of functions and helpers
static void index_save_helper(void *arg, const char *key, void *item);
static void index_load_helper(FILE *fp, index_t *index);
static void counters_save_helper(void* file, const int id, const int count);

// New index creation function
index_t* index_new(int num_slots) {
    index_t *index = malloc(sizeof(index_t));
    if (index == NULL) {
        return NULL;
    }
    index->ht = hashtable_new(num_slots);
    if (index->ht == NULL) {
        free(index);
        return NULL;
    }
    return index;
}
// build the index
void index_build(char* pageDirectory, index_t* index) {
    int docID = 0;
    webpage_t* page;
    char filename[16];
    sprintf(filename, "%d", docID);
    char* pathname = get_pathname(pageDirectory, filename);
    FILE* opened = fopen(pathname, "r");
    while (opened != NULL) {
        fclose(opened);
        page = pagedir_load(pathname);
        if(page == NULL){
            fprintf(stderr, "unbale to load the page \n");
        }
        if (page == NULL) break; // No more files
        int pos = 0;
        char* word;
        while ((word = webpage_getNextWord(page, &pos)) != NULL) {
            if (strlen(word) >= 3) {
                word_normalize(word); // Convert to lower case
                // printf("%s  ", word);
                counters_t* wordCounts = index_find(index, word); // did we have the same word?
                if (wordCounts == NULL) {
                    char * tosave = malloc(strlen(word));
                    strcpy(tosave, word);
                    wordCounts = counters_new();
                    if (!index_insert(index, tosave, wordCounts)) {
                        counters_delete(wordCounts);
                    }
                }
                counters_add(wordCounts, docID);
            }
            free(word);
        }

        // webpage_delete(page); // don't know why this is causing trouble
        docID++;
        sprintf(filename, "%d", docID);
        pathname = get_pathname(pageDirectory, filename);
        opened = fopen(pathname, "r");
    }
}

// Insert a word and its counters into the index
bool index_insert(index_t *index, const char *key, void *item) {
    return hashtable_insert(index->ht, key, item);
}

// Find a word's counters in the index
void * index_find(index_t *index, const char *key) {
    return hashtable_find(index->ht, key);
}

// Save the index to a file
void index_save(const char *fname, index_t *index) {
    FILE *fp = fopen(fname, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open file %s for writing.\n", fname);
        return;
    }
    hashtable_iterate(index->ht, fp, index_save_helper);
    fclose(fp);
}


// to print the id and count of ta given word 


// Helper function to save the counters for each word
static void index_save_helper(void *arg, const char *key, void *item) {
    FILE *fp = (FILE *)arg;
    counters_t *ctrs = (counters_t *)item; // get the counter
    fprintf(fp, "%s", key); // printing the word
    counters_iterate(ctrs, fp, (void (*)(void*, const int, const int))counters_save_helper);
    fprintf(fp, "\n");
}

static void counters_save_helper(void* file, const int id, const int count){
    // FILE* file_opened = fopen(file, "w");
    fprintf(file, " %d %d ", id, count);
}


// Load the index from a file
void index_load(const char *fname, index_t *index) {
    FILE *fp = fopen(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open file %s for reading.\n", fname);
        return;
    }
    index_load_helper(fp, index);
    fclose(fp);
}

// Helper function to load counters for each word
static void index_load_helper(FILE *fp, index_t *index) {
    char * word;
    char* line = file_readLine(fp);
    if(line == NULL){
        fprintf(stderr, "Empty file");
    }
    while ((line != NULL)) {
        counters_t *ctrs = counters_new();
        char* token = strtok(line, " ");
        int count = 0;
        int docID = 0;
        int indexing = 0;
        while( token != NULL) {
            if(indexing != 0){
                if(indexing % 2 != 0){
                    docID = atoi(token);
                }else{
                    count = atoi(token);
                    counters_set(ctrs, docID, count);
                }
            }else{
                // word = mallod(strlen(token));
                word = token;
            }
            token = strtok(NULL, " ");
            indexing +=1;
        }
        hashtable_insert(index->ht, word, ctrs);
        line = file_readLine(fp);
    }
}

// Delete the index and free the hashtable
void index_delete(index_t *index) {
    hashtable_delete(index->ht, (void (*)(void*))counters_delete);
    free(index);
}