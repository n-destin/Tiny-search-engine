/* 
 * file utilities - reading a word, line, or entire file
 * 
 * David Kotz, 2016, 2017, 2019, 2021
 */

#ifndef __PAGEDIR_H
#define __PAGEDIR_H

#include <stdio.h>
#include <dirent.h>
#include <stdbool.h>
#include "../libcs50/webpage.h"
#include "../libcs50/hashtable.h"

/**************** page_init ****************/
/* The user provides: 
                        - a name of the directory to be initialized
 * We create a new directory with the file .crawler
 * 
 */
bool pagedir_init(const char *pageDirectory) ;

/**************** pagedir_save ****************/
/* 
 * The user provides: 
                        - a valid pointer to a page: i.e the page->html is not null
                        - a name of directoy where the webpages are being saved
                        - a unique document Id
* We add the file to the directory if it is there
 */
void pagedir_save(const webpage_t* page, const char* pageDirectory, const int docID);

// Validates that a directory contains a .crawler file indicating it was initialized for crawling
int pagedir_validate(const char* pageDirectory);

char* get_pathname(char* pageDirectory, const char* id);

// Loads a webpage from a file
webpage_t* pagedir_load(const char* filename);

#endif // WEBPAGEDIR_H
