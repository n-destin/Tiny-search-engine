/* 
 * webpage - utility functions for downloading, saving, and loading web pages
 *
 * This module defines the opaque `webpage_t` type, and a suite of
 * functions for creating and manipulating "web pages".  A "web page"
 * is really a struct holding
 *     the URL of this web page, in canonical form
 *     the depth at which the crawler found this page
 *     the HTML for the page - may be NULL. 
 *
 * Sometimes you just want to keep track of web pages without HTML -
 * perhaps because you have not yet fetched that HTML - in that case,
 * the webpage object has a null HTML pointer.
 * 
 * Othertimes, you have fetched the HTML and want to work with it;
 * then, the webpage object has a non-null HTML pointer.
 *
 * Original by Ira Ray Jenkins - April 2014
 * 
 * Updated by David Kotz - April 2016, July 2017, April 2019, 2021
 *
 */

#ifndef __WEBPAGE_H
#define __WEBPAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/***********************************************************************/
/* webpage_t: opaque struct to represent a web page, and its contents.
 */
typedef struct webpage webpage_t;

/* getter methods */
int   webpage_getDepth(const webpage_t* page);
char* webpage_getURL(const webpage_t* page);
char* webpage_getHTML(const webpage_t* page);

/**************** webpage_new ****************/
/* Allocate and initialize a new webpage_t structure.
 *
 * Caller provides:
 *   url   must be a non-null pointer to malloc'd memory.
 *   depth must be non-negative.
 *   html  may be null; if not, must point to malloc'd memory.
 *   IMPORTANT: url and html will later be free'd by webpage_delete.
 *
 * We return:
 *   pointer to new webpage_t, or NULL on any error.
 *
 * Caller is responsible for:
 *   later calling webpage_delete with the returned pointer.
 *
 * IMPORTANT:
 *  We do NOT fetch the html from url; fetch it later with webpage_fetch().
 *  Parameters url and html are copied, but their strings are NOT copied.
 *  In effect, the webpage module adopts responsibility for those two strings,
 *  which is why they must have resulted from malloc() ... later the webpage
 *  module will free() those two strings in webpage_delete().
 */
webpage_t* webpage_new(char* url, const int depth, char* html);

/**************** webpage_delete ****************/
/* Delete a webpage_t structure created by webpage_new().
 *
 * Caller provides
 *   pointer to a webpage_t provided by webpage_new.
 *   (parameter is void* so this function can be used as an itemdelete()).
 *
 * IMPORTANT:
 *   we call free() on both the url and the html, if not NULL.
 */
void webpage_delete(void* data);

/***************** webpage_fetch ******************************/
/* retrieve HTML from page->url, store into page->html
 *
 * Caller provides
 *   page, a valid webpage_t* as returned from webpage_new(), and
 *     1. page->url contains the url to fetch
 *     2. page->html is NULL at call time
 *
 * We return:
 *   true if the fetch was successful; otherwise, false;
 *   if the fetch succeeded, page->html will contain the content retrieved.
 *
 * Caller is responsible for:
 *   If this function is successful, a new, null-terminated character
 *   buffer will be allocated as page->html. The caller must later free this
 *   memory, typically by calling webpage_delete().
 *
 * Usage example:
 *  webpage_t* page = webpage_new(url, 0, NULL);
 *  if(webpage_fetch(page)) {
 *    char* html = webpage_getHTML(page);
 *    printf("Found html: %s\n", html);
 *  }
 *  webpage_delete(page);
 *
 * Limitations:
 *   * can only handle http (not https or other schemes)
 *   * can only handle URLs of form http://host[:port][/pathname]
 *   * cannot handle redirects (HTTP 301 or 302 response codes)
 */
bool webpage_fetch(webpage_t* page);


/**************** webpage_getNextWord ***********************************/
/* return the next word from page->html[pos]
 *
 * Caller provides
 *   page: pointer to valid webpage_t with page->html not NULL.
 *   pos: pointer to an int representing current position in html buffer;
 *        should be 0 on the initial call.
 *        After return, *pos is the index after the word returned.
 *
 * We return:
 *   pointer to string containing the next word, if any; otherwise NULL.
 * 
 * Side effect:
 *   page->html will be compressed to remove white space.
 *
 * Caller is responsible for:
 *   later free()ing the string returned.
 *
 * Usage example: (retrieve all words in a page)
 * int pos = 0;
 * char* result;
 *
 * while ((result = webpage_getNextWord(page, &pos)) != NULL) {
 *     printf("Found word: %s\n", result);
 *     free(result);
 * }
 */

char* webpage_getNextWord(webpage_t* page, int* pos);

/****************** webpage_getNextURL ***********************************/
/* return the next url from page->html[pos]
 *
 * Caller provides:
 *   page: pointer to valid webpage_t with page->html not NULL.
 *   pos: pointer to an int representing current position in html buffer;
 *        should be 0 on the initial call.
 *        After return, *pos is the index after the URL returned.
 *
 * We return:
 *   pointer to string containing the next URL, if any; otherwise NULL.
 *
 * Side effect:
 *   page->html will be compressed to remove white space.
 *
 * Caller is responsible for:
 *   later free()ing the string returned.
 *
 * Known bugs:
 *   Does not work well on directory-type URLs like
 *      http://cs50tse.cs.dartmouth.edu/tse/letters
 *   unless they include a trailing slash, like
 *      http://cs50tse.cs.dartmouth.edu/tse/letters/
 *
 * Usage example: (retrieve all urls in a page)
 * int pos = 0;
 * char* result;
 *
 * while ((result = webpage_getNextURL(page, &pos)) != NULL) {
 *     printf("Found url: %s\n", result);
 *     free(result);
 * }
 */

char* webpage_getNextURL(webpage_t* page, int* pos);

/***********************************************************************
 * normalizeURL - returns a normalized form of the url
 *
 * Caller provides:
 *    url: string containing absolute url to normalize
 *
 * Returns:
 *  a new string with normalized copy of the input url, or
 *  NULL if the url is NULL, or
 *  NULL if the url can't be parsed or normalized, or
 *  NULL if the url refers to a file unlikely to contain html, or
 *  NULL if cannot allocate new memory.
 *
 * Caller is responsible for:
 *  eventually free()ing the returned string.
 *
 * Usage example:
 *   char* url = normalizeURL("HTTP://UsEr:PaSs@www.EXAMPLE.com/path/.././file.html?name=val#top");
 * url should be: http://UsEr:PaSs@www.example.com/file.html?name=val#top
 */
char* normalizeURL(const char* url);


/***********************************************************************
 * isInternalURL - verify whether the given url is 'internal' to CS50
 *
 * Caller provides:
 *   url: string containing a *normalized* absolute url
 *
 * Returns:
 *   true if the url is non-NULL and "internal",
 *   false otherwise.
 *
 * "internal" means that the normalized url begins with INTERNAL_PREFIX.
 */
bool isInternalURL(const char* url);

// All normalized URLs beginning with this prefix are considered "internal"
static const
char INTERNAL_PREFIX[] = "http://cs50tse.cs.dartmouth.edu/tse/";

#endif // __WEBPAGE_H
