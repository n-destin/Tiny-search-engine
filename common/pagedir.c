#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include "../libcs50/webpage.h"
#include <string.h>
#include "../libcs50/file.h"

#ifndef WEBPAGE_H 
typedef struct webpage {
  char* url;                            
  char* html;                      
  int html_len;              
  int depth;      
} webpage_t;

#endif

char* get_pathname(const char* pageDirectory, const char* filename) {
    int length = strlen(pageDirectory) + strlen(filename) + 2;
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


// exit if you are not able to read the file pageDirectory/.crawler
int pagedir_validate(const char* pageDirectory) {
    char *check_pathname = get_pathname(pageDirectory, ".crawler");
    FILE* file = fopen(check_pathname, "r");
    if (file != NULL) {
        fclose(file);
        return 0;
    }
    return 1;
}


// creating a webpage

// For this function, I got help from another student on how to implement it. Espesically about the fseek  and ftell
webpage_t* pagedir_load(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;

    char url[256];
    int depth;
    fscanf(file, "%s\n%d\n", url, &depth);
    char* html = NULL;
    // int len = 0;

    fseek(file, 0, SEEK_END);
    long bytes = ftell(file);
    fseek(file, 0, SEEK_SET);
    html = malloc(bytes + 1);
    fread(html, 1, bytes, file);
    html[bytes] = '\0';

    webpage_t* page = webpage_new(url, depth, html);
    fclose(file);
    return page;
}
