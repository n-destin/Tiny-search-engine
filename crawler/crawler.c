
/*
* Destin Niyomufasha
* Spring 2024
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crawler.h"
#include <stdbool.h>
#include "../libcs50/bag.h"
#include "../libcs50/hashtable.h"
#include "../libcs50/webpage.h"
#include "../common/pagedir.h"
#include "../libcs50/mem.h"
#include "../libcs50/file.h"


static void parseArgs(const int argc, char* argv[], char** seedURL, char** pageDirectory, int* maxDepth);
void crawl(char* seedURL, char* pageDirectory, const int maxDepth);
static void pageScan(webpage_t* page, bag_t* pagesToCrawl, hashtable_t* pagesSeen);
static bool check_internal(char* URL);


int main(const int argc, char* argv[]) {
    char* seedURL = NULL;
    char* pageDirectory = NULL;
    int maxDepth = 0;
    parseArgs(argc, argv, &seedURL, &pageDirectory, &maxDepth);
    crawl(seedURL, pageDirectory, maxDepth);
    return 0;
}

static void parseArgs(const int argc, char* argv[], char** seedURL, char** pageDirectory, int* maxDepth) {
    if (argc != 4) {
        fprintf(stderr, "Something went wrong");
        exit(1);
    }

    if( atoi(argv[3]) > 10 || atoi(argv[3] )< 0){
        fprintf(stderr, "The depth %d is out of range \n", atoi(argv[3]));
        exit(1);
    }
    *seedURL = argv[1];
    *pageDirectory = argv[2];
    *maxDepth = atoi(argv[3]);
}


void crawl(char* seedURL, char* pageDirectory, const int maxDepth) {
    hashtable_t* pagesSeen = hashtable_new(200);
    if(pagesSeen == NULL){
        fprintf(stderr, "pagesSeen is null");
        exit(1);
    }
    hashtable_insert(pagesSeen, seedURL, ""); // nothing as item
    int docID = 0;
    bag_t* pagesToCrawl = bag_new();
    if(pagesToCrawl == NULL){
        fprintf(stderr, "pagesToCrawl is null: Error allocating memory for it");
        exit(1);
    }
    webpage_t* seedPage = webpage_new(seedURL, 0, NULL);
    webpage_t* currentPage;


    bag_insert(pagesToCrawl, seedPage);
    pagedir_init(pageDirectory);

    while ((currentPage = bag_extract(pagesToCrawl)) != NULL) {
        if (webpage_fetch(currentPage)) {
            printf("%s  %d\n", webpage_getURL(currentPage), webpage_getDepth(currentPage));
            pagedir_save(currentPage, pageDirectory, docID);
            if(webpage_getDepth(currentPage) < maxDepth){
                pageScan(currentPage, pagesToCrawl, pagesSeen);
            }else{
                continue;
            }
            docID += 1;
        } else{
            fprintf(stderr, "Something went wrong %s \n", webpage_getURL(currentPage));
        }
    }

    // webpage_delete(seedPage);
    bag_delete(pagesToCrawl, NULL);
    hashtable_delete(pagesSeen, NULL);
}

static void pageScan(webpage_t* page, bag_t* pagesToCrawl, hashtable_t* pagesSeen) {
    int pos = 0;
    char* result;

    while ((result = webpage_getNextURL(page, &pos)) != NULL) {
        char* normalizedURL = normalizeURL(result);
        bool found = hashtable_find(pagesSeen, normalizedURL);
        bool is_internal = check_internal(normalizedURL);
        if (normalizedURL && !found && is_internal) {
            if(hashtable_insert(pagesSeen, normalizedURL, "")){
                char* newPageUrl = malloc(strlen(normalizedURL) + 1);
                if(newPageUrl != NULL){
                    strcpy(newPageUrl, normalizedURL);
                    webpage_t* newPage = webpage_new(newPageUrl, webpage_getDepth(page) + 1, NULL);
                    bag_insert(pagesToCrawl, newPage);
                }
            }
        } else{
            free(normalizedURL);
        }
    }
}

// remove the const --- will-sece
static bool check_internal(char* URL) {
    return isInternalURL(URL);
}
