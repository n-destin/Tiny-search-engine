#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include "../libcs50/webpage.h"
#include <string.h>

#ifndef WEBPAGE_H 
typedef struct webpage {
  char* url;                            
  char* html;                      
  size_t html_len;              
  int depth;      
} webpage_t;

#endif

char* get_pathname(const char* pageDirectory, const char* filename) {
    size_t length = strlen(pageDirectory) + strlen(filename) + 2;
    char* pathname = malloc(length);
    if (pathname == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    sprintf(pathname, "%s/%s", pageDirectory, filename);
    return pathname;
}



bool pagedir_init(const char *pageDirectory) {
    char *init_pathname = get_pathname(pageDirectory, ".crawler");
    FILE *fp = fopen(init_pathname, "w");
    if (fp == NULL) {
        fprintf(stderr, "Failed to create .crawler file in directory '%s'\n", pageDirectory);
        exit(1);
    }
    fclose(fp);
    free(init_pathname);
    return true;
}

void pagedir_save(const webpage_t* page, const char* pageDirectory, const int docID) {
    char filename[16];
    sprintf(filename, "%d", docID);
    char *pathname = get_pathname(pageDirectory, filename);
    FILE *fp = fopen(pathname, "w");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open page file '%s' for writing\n", pathname);
        return;
    }
    fprintf(fp, "%s\n%d\n%s", page->url, page->depth, page->html);
    free(pathname);
    fclose(fp);
}

