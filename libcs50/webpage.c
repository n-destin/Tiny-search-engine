/* 
 * webpage - utility functions for downloading, saving, and loading web pages.
 *           See webpage.h for usage.
 *
 * Original by Ira Ray Jenkins - April 2014
 * 
 * Updated by David Kotz - April 2016, July 2017, April 2019, 2021
 * Updated by Xia Zhou - July 2016, July 2018
 *
 */

/* students shouldn't take advantage of the gnu extensions, 
 * but parsing html without them is a pain.
 */
#define _GNU_SOURCE       // strncasecmp, strdup

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <netdb.h>
#include "file.h"
#include "webpage.h"
#include "mem.h"

/* ***************************************** */
/* Private types */
struct URL {
  char* scheme;               // http://
  char* user;                 // username:password@
  char* host;                 // www.example.com
  char* path;                 // /path/to/file.html
  char* query;                // ?name1=val1&name2=val2
  char* fragment;             // #top
};

/* webpage_t: structure to represent a web page, and its contents.
 * The innards should not be visible to users of the webpage module.
 */
typedef struct webpage {
  char* url;                               // url of the page
  char* html;                              // html code of the page
  size_t html_len;                         // length of html code
  int depth;                               // depth of crawl
} webpage_t;

/* *********************************************************************** */
/* Private function prototypes */

static FILE* connectToHost(const char* hostname, const int port);
static inline bool isBlankLine(const char* line);
static char* removeDotSegments(char* input);
static void removeWhitespace(char* str);
static char* fixRelativeURL(char* base, char* rel, size_t len);
static bool parseURL(const char* str, struct URL* url);
static void freeURL(struct URL url);
static bool burstURL(const char* url, char** hostname, 
                     int* port, char** pathname);
#ifdef DEBUG
static void printURL(struct URL url);
#endif // DEBUG

/* *********************************************************************** */
/* Private global variables */

static const int MAX_TRY = 3;    // maximum attempts to fetch
static const int HTTP_PORT = 80; // default web server port

static const char* EXTS[] = {  // valid extensions
  "html",
  "htm",     // added by DFK
  //  "jsp", // removed by DFK
  //  "php", // removed by DFK
  NULL   // sentinel to end loops over this array
};


/* *********************************************************************** */
/* Public methods */

/* *********************************************************************** */
/* getter methods - see webpage.h for documentation */
int   webpage_getDepth(const webpage_t* page) { 
  return page ? page->depth : 0;
}
char* webpage_getHTML(const webpage_t* page)  { 
  return page ? page->html  : NULL;
}
char* webpage_getURL(const webpage_t* page)   { 
  return page ? page->url   : NULL; 
}

/**************** webpage_new ****************/
/* see webpage.h for documentation */
webpage_t* 
webpage_new(char* url, const int depth, char* html)
{
  if (url == NULL || depth < 0) {
    return NULL;
  }

  webpage_t* page = mem_assert(malloc(sizeof(webpage_t)), "webpage_t");

  page->url = url;
  page->depth = depth;
  page->html = html;
  page->html_len = html ? strlen(html) : 0;

  return page;
}

/**************** webpage_delete ****************/
/* see webpage.h for documentation */
void
webpage_delete(void* data)
{
  webpage_t* page = data;
  if (page != NULL) {
    if (page->url) free(page->url);
    if (page->html) free(page->html);
    free(page);
  }
}


/* ************* webpage_fetch ******************** */
/* see webpage.h for usage documentation.
 *
 * Limitations:
 *   * can only handle http (not https or other schemes)
 *   * can only handle URLs of form http://host[:port][/pathname]
 *   * cannot handle redirects (HTTP 301 or 302 response codes)
 * 
 * Pseudocode:
 *     1. check for valid page 
 *     2. parse url into hostname, port, and filename
 *     3. open a connection to the given host
 *     4. send http request
 *     5. fetch html response
 *     6. cleanup
 */
bool 
webpage_fetch(webpage_t* page)
{
  // check webpage structure - must have URL and not yet have HTML
  if (page == NULL || page->url == NULL || page->html != NULL) {
    return false;
  }

  // burst the URL into its components;
  // all we care about are hostname, port, and pathname
  char* hostname; // will be initialized by burstURL
  int port;       // will be initialized by burstURL
  char* pathname; // will be initialized by burstURL
  if (!burstURL(page->url, &hostname, &port, &pathname)) {
    return false;
  }

  // attempt to connect to server 
  FILE* http_fp = NULL; 
  for (int try = 0;  http_fp == NULL && try < MAX_TRY; try++) {
    // open connection - exit on error
    http_fp = connectToHost(hostname, port);

#ifndef NOSLEEP // CS50 students: please don't turn off the sleep!
    sleep(1);   // sleep one second between fetches, to lighten load on server
#endif
  }

  // failed to connect?
  if (http_fp == NULL) {
    return false;
  }

  // prepare and send HTTP request; receive response
  char* httpResponse = NULL;
  const char* httpFormat =
    "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n";
  if (fprintf(http_fp, httpFormat, pathname, hostname) >= 0) {
    // ensure stdio buffer is flushed to socket
    fflush(http_fp);
    // read the server's response
    httpResponse = file_readLine(http_fp);
  }

  free(hostname);
  free(pathname);

  // did we succeed? check the response
  bool success = false;

  if (httpResponse != NULL) {
    // check response code to see whether we succeeded
    int httpResponseCode = 0;
    if (sscanf(httpResponse, "HTTP/1.1 %d", &httpResponseCode) == 1
        && httpResponseCode == 200) {
      // success! ignore the rest of the header, then grab the page
      // read lines until we read a blank line or fail to read a line
      char* line = file_readLine(http_fp);
      while (line != NULL && !isBlankLine(line)) {
        free(line);
        line = file_readLine(http_fp);
      }
      // did we exit the loop because we read an empty line?
      if (line != NULL) {
        free(line); // the blank line

        // then grab everything else - that should be the page content
        char* html = file_readFile(http_fp);
        if (html != NULL) {
          page->html = html;
          success = true;
        } 
      }
    }
    free(httpResponse);
  }

  // clean up
  fclose(http_fp);

  return success;
}

/**************** webpage_getNextWord ****************/
/* see webpage.h for usage documentation.
 *
 * Code is courtesy of Ray Jenkins and/or Charles Palmer, 
 *   cleaned by David Kotz in April 2016, 2017; updated April 2019.
 *
 * Pseudocode:
 *     1. skip any leading non-alphabetic characters
 *     2. if we find a tag, i.e., <...tag...>, skip that tag
 *     3. save beginning of the word
 *     4. find the end, i.e., first non-alphabetic character
 *     5. create a new word buffer
 *     6. copy the word into the new buffer
 *     7. update *pos to first position past end of word
 *     8. return pointer to the word
 * 
 * Assumptions:
 *     1. webpage has html
 *     2. don't care about opening/closing tags: ignore anything between <...>
 *     3. if the html is malformed, we don't care: match '<' with next '>'
 */
char* 
webpage_getNextWord(webpage_t* page, int* pos)
{
  // make sure we have something to search, and a place for the result
  if (page == NULL || page->html == NULL || pos == NULL) {
    return NULL;
  }

  const char* doc = page->html;            // the html document
  const char* beg;                         // beginning of word
  const char* end;                         // end of word

  // consume any non-alphabetic characters
  while (doc[*pos] != '\0' && !isalpha(doc[*pos])) {
    // if we find a tag, i.e., <...tag...>, skip it
    if (doc[*pos] == '<') {
      end = strchr(&doc[*pos], '>');          // find the close
      
      if (end == NULL || *(++end) == '\0') { // ran out of html
        return NULL;
      }

      *pos = end - doc;       // skip over the <...tag...>
    } else {
      (*pos)++;               // just move forward
    }
  }

  // ran out of html
  if (doc[*pos] == '\0') {
    return NULL;
  }

  // doc[*pos] is the first character of a word
  beg = &(doc[*pos]);

  // consume word
  while (doc[*pos] != '\0' && isalpha(doc[*pos])) {
    (*pos)++;
  }

  // at this point, doc[*pos] is the first character *after* the word.
  // so doc[*pos-1] is the last character of the word.
  end = &(doc[*pos-1]);

  // 'beg' points to first character of the word
  // 'end' points to last character of the word
  int wordlen = end - beg + 1;

  // allocate space for length of new word + '\0'
  char* word = calloc(wordlen + 1, sizeof(char));
  if (word == NULL) {        // out of memory!
    return NULL;
  } else {
    // copy the new word
    strncpy(word, beg, wordlen);
    return word;
  }
}

/**************** webpage_getNextURL ****************/
/* See "webpage.h" for full documentation.
 *
 * Assumptions:
 *     1. page is valid, contains html and base_url
 *     2. *pos = 0 on initial call
 *
 * Pseudocode:
 *     1. check arguments
 *     2. if *pos = 0 (first call for this page): remove whitespace from html
 *     3. find hyperlink starting tags "<a" or "<A"
 *     4. find next href attribute "href="
 *     5. find next end tag ">"
 *     6. check that href comes before end tag
 *     7. deal with quoted and unquoted urls
 *     8. determine if url is absolute
 *     9. fixup relative links
 *    10. update *pos to position after the URL
 *    11. create new character buffer for result and return it
 */
char* 
webpage_getNextURL(webpage_t* page, int* pos)
{
  // make sure we have text and base url, and valid arg
  if (page == NULL || page->html == NULL || page->url == NULL || pos == NULL) {
    return NULL;
  }

  char* html = page->html;                 // the html document
  char* base_url = page->url;              // the base URL for this html
  int bad_link;                            // is this link ill formatted?
  int relative;                            // is this link relative?
  char delim;                              // url delimiter: ' ', ''', '"'
  char* lnk;                               // hyperlink tags
  char* href;                              // href in a tag
  char* end;                               // end of hyperlink tag or url
  char* ptr;                               // absolute vs. relative
  char* hash;                              // hash mark character

  // condense html, makes parsing easier
  if (*pos == 0) {
    removeWhitespace(html);
  }

  // parse for hyperlinks
  do {
    relative = 0;                        // assume absolute link
    bad_link = 0;                        // assume valid link

    // find tag "<a" or "<A""
    lnk = strcasestr(&html[*pos], "<a");

    // no more links on this page
    if (!lnk) { return NULL; }

    // find next href after hyperlink tag
    href = strcasestr(lnk, "href=");

    // no more links on this page
    if (!href) { return NULL; }

    // find end of hyperlink tag
    end = strchr(lnk, '>');

    // if the href we have is outside the current tag, continue
    if (end && (end < href)) {
      bad_link = 1; (*pos) += 2; continue;
    }

    // move href to beginning of url
    href+=5;

    // something went wrong, just continue
    if (!href) { bad_link=1; (*pos) += 2; continue; }

    // is the url quoted?
    if (*href == '\'' || *href == '"') {  // yes, href="url" or href='url'
      delim = *(href++);               // remember delimiter
      end = strchr(href, delim);       // find next of same delimiter
    } else {             // no, href=url
      end = strchr(href, '>');         // hope: <a ... href=url>
      // since we've stripped whitespace
      // this could mangle things like:
      // <a ... href=url name=val>
    }

    // if there is a # before the end of the url, exclude the #fragment
    hash = strchr(href, '#');
    if (hash && hash < end) {
      end = hash;
    }

    // if we don't know where to end the url, continue
    if (!end) {
      bad_link = 1; (*pos) += 2; continue;
    }

    // have a link now
    if (*href == '#') {                   // internal reference
      bad_link = 1; (*pos) += 2; continue;
    }

    // is the url absolute, i.e, ':' must precede any '/', '?', or '#'
    ptr = strpbrk(href, ":/?#");
    if (!ptr || *ptr != ':') { 
      relative = 1; 
    } else if (strncasecmp(href, "http", 4)) { // absolute, but not http(s)
      bad_link = 1; (*pos) += 2; continue;
    }
  } while (bad_link);                       // keep parsing

  // update position after the end of the url
  *pos = end - html;

  // have a good link now
  if (relative) {                           // need to fixup relative links
    char* result = fixRelativeURL(base_url, href, end - href);
    return result; // may be NULL if Fixup failed.
  } else {
    // create new buffer
    char* result = calloc(end-href+1, sizeof(char));
    if (result == NULL) {
      // out of memory
      return NULL;
    } else {
      // copy over absolute url
      strncpy(result, href, end - href);
      return result;
    }
  }
}

/******************** normalizeURL *******************************/
/* Normalize the url according to RFC 3986 chapter 3.
 * see webpage.h for documentation.
 *
 * Pseudocode:
 *     1. check arguments
 *     2. try to parse url
 *     3. check any file extensions
 *     4. clear the url
 *     5. remove dot segments and lowercase scheme and host
 *     6. allocate space for the new url string
 *     7. assemble the new url from pieces
 */
char*
normalizeURL(const char* url)
{
  if (url == NULL) {
    return NULL;
  }

  // try to parse the url
  struct URL tmp;               // pieces of the parsed url
  if (!parseURL(url, &tmp)) {
    freeURL(tmp);
    return NULL;
  }

  // check file extension
  if (tmp.path != NULL) {               // if we have a path
    char* dot = strrchr(tmp.path, '.');   // where is last '.' within string
    char* slash = strrchr(tmp.path, '/'); // where is last '/' within string

    // We expect to see URL of form /path/to/file.ext
    if (dot != NULL && slash != NULL && dot > slash) {
      char* ext = dot+1;                  // extension begins after '.'

      // check against list of known extensions
      if (strlen(ext) > 0) {
        bool isKnownExt = false;      // is the extension valid?
        for (int i = 0; EXTS[i] != NULL; i++) {
          if (strncasecmp(ext, EXTS[i], strlen(EXTS[i])) == 0) {
            isKnownExt = true;
            break;
          }
        }

        // no recognized extension found
        if (!isKnownExt) {
          freeURL(tmp);
          return NULL;
        }
      }
    }
  }

  // Allocate space for resulting URL - which will be no longer than url.
  char* result = malloc(strlen(url)+1);
  if (result == NULL) {
    freeURL(tmp);
    return NULL;
  } else {
    // initialize it to empty string
    *result = '\0';
  }

  // put normalized url back together
  if (tmp.scheme) {                         // scheme
    strcat(result, tmp.scheme);
  }
  if (tmp.user) {                           // user
    strcat(result, tmp.user);
  }
  if (tmp.host) {                           // host
    strcat(result, tmp.host);
  }
  if (tmp.path) {                           // path
    // remove . and .. segments
    char* path = removeDotSegments(tmp.path);
    if (path == NULL) {
      freeURL(tmp);
      return NULL;
    } else {
      strcat(result, path);
      free(path);
    }
  }
  if (tmp.query) {                          // query
    strcat(result, tmp.query);
  }
  if (tmp.fragment) {                       // fragment
    strcat(result, tmp.fragment);
  }

#ifdef REMOVE_SLASH
  // Remove trailing slash [DFK 2017].
  // This code allows crawler to realize that
  //    http://www.cs.dartmouth.edu == http://www.cs.dartmouth.edu/
  // but doing so actually prevents the crawler from following the 
  // server's implicit redirect to http://www.cs.dartmouth.edu/index.html
  // So, I've decided not to include it.
  if (*result != '\0') {
    char* last = result + strlen(result) - 1;
    if (*last == '/'){
      *last = '\0';
    }
  }
#endif // REMOVE_SLASH

  freeURL(tmp);
  return result;
}

/***********************************************************************
 * isInternalURL - see webpage.h for interface description.
 */
bool
isInternalURL(const char* url)
{
  if (url == NULL) {
    return false;
  } else {
    return (strncmp(url, INTERNAL_PREFIX, strlen(INTERNAL_PREFIX)) == 0);
  }
}


/***********************************************************************
 * INTERNAL FUNCTIONS
 ***********************************************************************/

/***********************************************************************
 * parseURL - attempts to parse str into a URL struct
 * @str: absolute url to parse
 * @url: pointer to a struct containing parts of a url; 
 *       inbound, its members are assumed uninitialized
 *       outbound, its members are initialized to NULL or malloc'd memory.
 *       Caller is responsible for calling freeURL(url) to free that space.
 *
 * Expects str to be an absolute url. Returns false if str cannot be
 * successfully parsed; otherwise, returns true.
 *
 * From RFC 3986 chapter 3:
 *
 * The following are two example URIs and their component parts:
 *
 *       foo://example.com:8042/over/there?name=ferret#nose
 *       \_/   \______________/\_________/ \_________/ \__/
 *        |           |            |            |        |
 *     scheme     authority       path        query   fragment
 *        |   _____________________|__
 *       / \ /                        \
 *       urn:example:animal:ferret:nose
 *
 * Should have no use outside of this file, thus declared static.
 */
static bool
parseURL(const char* str, struct URL* url)
{
  int host_len;                            // length of host segment
  char* scheme_end;                        // scheme end point, : or :/ or ://
  char* user_end;                          // end of user info, @
  char* host_beg;                          // beginning of host, : or @
  char* host_end;                          // end of host, scheme:host/
  char* path_end;                          // end of path, ? or # or end of url
  char* query_beg;                         // end of query, # or end of url
  char* frag_beg;                          // end of fragment, end of url

  // make sure we have a str and url struct
  if (str == NULL || url == NULL) {
    return false;
  }

  // initialize the structure
  url->scheme = NULL;
  url->user = NULL;
  url->host = NULL;
  url->path = NULL;
  url->query = NULL;
  url->fragment = NULL;
  
  // make sure absolute url, i.e., ':' must preceede any '/', '?', or '#'
  scheme_end = strpbrk(str, ":/?#");
  if (scheme_end == NULL || *scheme_end != ':') {
    return false;
  }

  scheme_end++;                            // consume ':'

  // do we have scheme:<path> or scheme:<host><path>
  if (strncmp(scheme_end, "//", 2) == 0) {     // have host
    scheme_end += 2;                       // consume "//"
  }

  // allocate url scheme
  url->scheme = calloc(scheme_end - str + 1, sizeof(char));
  if (url->scheme == NULL) {
    return false;
  }

  // copy scheme in lowercase
  for (int i = 0; str+i < scheme_end; i++) {
    url->scheme[i] = tolower(str[i]);
  }


  // get user information, anything between scheme and first '@'
  user_end = strpbrk(scheme_end, "@/");
  if (user_end && *user_end == '/') {    // no user info
    user_end = NULL;
  }

  if (user_end != NULL) {       // have user info
    user_end++;                 // consume '@'

    // allocate user
    url->user = calloc(user_end - scheme_end + 1, sizeof(char));
    if (url->user == NULL) {
      return false;
    }

    // copy user info
    strncpy(url->user, scheme_end, user_end - scheme_end);
  }

  // get host information
  host_end = strchr(scheme_end, '/');
  const char* host_e;           // end of host, / or end of url

  if (host_end == NULL && user_end == NULL) {        // scheme:host
    host_len = strlen(str) - (scheme_end - str);
    host_beg = scheme_end;
    host_e = &str[strlen(str)];
  } else if (host_end == NULL && user_end != NULL) { // scheme:user@host
    host_len = strlen(str) - (user_end - str);
    host_beg = user_end;
    host_e = &str[strlen(str)];
  } else if (host_end != NULL && user_end == NULL) { // scheme:host/path...
    host_len = host_end - scheme_end + 1;
    host_beg = scheme_end;
    host_e = host_end;
  } else {                                      // scheme:user@host/path...
    host_len = host_end - user_end + 1;
    host_beg = user_end;
    host_e = host_end;
  }

  // allocate host
  url->host = calloc(host_len + 1 , sizeof(char));
  if (url->host == NULL) {
    return false;
  }

  // lowercase it
  for (char* ptr = host_beg; ptr < host_e; ptr++) {
    url->host[ptr - host_beg] = tolower(*ptr);
  }

  // get path part, between host and query and/or fragment
  path_end = strpbrk(scheme_end, "?#");

  if (path_end) {                           // .../path? or .../path#
    url->path = calloc(path_end - host_e + 1, sizeof(char));
    if (url->path == NULL) {
      return false;
    }
    strncpy(url->path, host_e, path_end - host_e);
  } else {                                 // .../path
    url->path = calloc(&str[strlen(str)] - host_e + 1, sizeof(char));
    if (url->path == NULL) {
      return false;
    }

    strcpy(url->path, host_e);
  }

  // get fragment, anything after first '#'
  frag_beg = strchr(scheme_end, '#');

  if (frag_beg != NULL) {       // have fragment
    url->fragment = calloc(&str[strlen(str)] - frag_beg + 1, sizeof(char));
    if (url->fragment == NULL) {
      return false;
    }

    strcpy(url->fragment, frag_beg);
  }

  // get query, anything after first '?' before any '#'
  query_beg = strchr(scheme_end, '?');

  if (query_beg != NULL && frag_beg == NULL) { // ...?name=value
    url->query = calloc(&str[strlen(str)] - query_beg + 1, sizeof(char));
    if (url->query == NULL) {
      return false;
    }

    strcpy(url->query, query_beg);
  } else if (query_beg && frag_beg && query_beg < frag_beg) { // ...?name=value#top
    url->query = calloc(frag_beg - query_beg + 1, sizeof(char));
    if (url->query == NULL) {
      return false;
    }

    strncpy(url->query, query_beg, frag_beg - query_beg);
  }

  return true;                                // if we got this far, good
}

/* ****************** freeURL ***************************** */
/* free the members of the URL struct - but not the struct itself.
 */
static void 
freeURL(struct URL url)
{
  if (url.scheme != NULL)   { free(url.scheme); }
  if (url.user != NULL)     { free(url.user); }
  if (url.host != NULL)     { free(url.host); }
  if (url.path != NULL)     { free(url.path); }
  if (url.query != NULL)    { free(url.query); }
  if (url.fragment != NULL) { free(url.fragment); }
}


#ifdef DEBUG
/* ****************** printURL ***************************** */
/* Print members of the URL struct - for debugging.
 */
static void 
printURL(struct URL url)
{
  printf("URL ");
  printf("scheme '%s'; ", url.scheme);
  printf("user '%s'; ", url.user);
  printf("host '%s'; ", url.host);
  printf("path '%s'; ", url.path);
  printf("query '%s'; ", url.query);
  printf("fragment '%s'; ", url.fragment);
  printf("\n");
}
#endif // DEBUG

/* ****************** burstURL ********************* */
/* Burst the URL into components (hostname, port, pathname).
 *
 * Input: URL, assumed non-NULL and already normalized.
 * 
 * Output: fill in the other parameters:
 *   a pointer to new string containing the hostname
 *   an integer representing the port
 *   a pointer to new string containing the pathname.
 * and
 *   return true if successful. 
 * 
 * If success, the hostname and pathname must be free'd later.
 * Each string is allocated enough space to hold the whole URL, 
 * which is more than necessary, allowing a little growth if needed.
 * 
 * burstURL is much simpler than parseURL and is used by 
 * webpage_fetch because it can't handle anything other than simple
 * http://hostname[:port][/path] forms of URL anyway.
 */
static bool
burstURL(const char* url, char** hostname, int* port, char** pathname)
{
  // make plenty of space for the resulting strings
  int length = strlen(url);

  // initialize hostname to empty string
  *hostname = calloc(sizeof(char), length); // initialized to all nulls
  if (*hostname == NULL) {
    return false;
  }

  // initialize pathname to slash
  *pathname = calloc(sizeof(char), length); // initialized to all nulls
  if (*pathname == NULL) {
    free(*hostname);
    return false;
  } else {
    **pathname = '/';
  }

  // initialize port to default port
  *port = HTTP_PORT;

  // parse various forms of the URL
  if (sscanf(url, "http://%[^:]:%d/%s", *hostname, port, *pathname+1) == 3) {
    return true;
  } else if (sscanf(url, "http://%[^/]/%s", *hostname, *pathname+1) == 2) {
    return true;
  } else if (sscanf(url, "http://%[^:]:%d", *hostname, port) == 2) {
    return true;
  } else if (sscanf(url, "http://%[^/]/", *hostname) == 1) {
    return true;
  } else if (sscanf(url, "http://%s", *hostname) == 1) {
    return true;
  } else {
    free(*hostname); *hostname = NULL;
    free(*pathname); *pathname = NULL;
    return false;
  }
}


/* ********************* connectToHost ************************** */
/* Connect to the given hostname and port, 
 * returning an open FILE* for the socket,
 * or NULL on failure.
 */
static FILE* 
connectToHost(const char* hostname, const int port)
{
  // Look up the hostname specified on command line
  struct hostent *hostp = gethostbyname(hostname);
  if (hostp == NULL) {
    return NULL;
  }

  // Initialize fields of the server address
  struct sockaddr_in server;  // address of the server
  server.sin_family = AF_INET;
  bcopy(hostp->h_addr_list[0], &server.sin_addr, hostp->h_length);
  server.sin_port = htons(port);

  // Create socket (a file descriptor)
  int comm_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (comm_sock < 0) {
    return NULL;
  }

  // And connect that socket to that server   
  if (connect(comm_sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
    return NULL;
  }

  // to make it easier to work with, switch to stdio
  FILE* http_fp = fdopen(comm_sock, "r+");
  if (http_fp == NULL) {
    return NULL;
  }

  return http_fp;
}


/* ***************************************************************** */
/*
 * removeDotSegments - removes . and .. segments from url paths
 * @input: the character buffer to cleanse
 *
 * Returns a newly allocated character buffer with . and .. segments removed
 * according to the algorithm in RFC 3986 section 5.2.4 "Remove Dot Segments".
 * See: http://www.ietf.org/rfc/rfc1738.txt
 *
 * Should have no use outside of this file, thus declared static.
 *
 * Implementation adapted for CS50 TSE Crawler from the cURL library:
 *
 * Copyright (c) 1996 - 2014, Daniel Stenberg, <daniel@haxx.se>.
 *
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright
 * notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of a copyright holder shall not
 * be used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization of the copyright holder.
 */
static char* 
removeDotSegments(char* input)
{
  size_t in_len;                           // input length
  size_t copy_len;                         // copy length
  char* copy;                              // copy of input that is modified
  char* copyptr;                           // handle for free'ing copy'
  char* out;                               // output buffer
  char* outptr;                            // pointer to current write point

  if (!input || strlen(input) < 1)
    return NULL;

  // save lengths
  in_len = copy_len = strlen(input);

  // create output buffer
  out = outptr = calloc(in_len + 1, sizeof(char));
  if (!out)
    return NULL; /* out of memory */

  // copy the input buffer
  copy = copyptr = strdup(input);
  if (!copy) {
    free(out);
    return NULL;
  }

  // 2.  While the input buffer is not empty, loop as follows:
  do {
    // A. If the input buffer begins with a prefix of "../" or "./",
    //    then remove that prefix from the input buffer; otherwise,
    if (!strncmp("./", copy, 2)) {
      copy += 2;
      copy_len -= 2;
    }
    else if (!strncmp("../", copy, 3)) {
      copy += 3;
      copy_len -= 3;
    }

    // B. if the input buffer begins with a prefix of "/./" or "/.",
    //    where "." is a complete path segment, then replace that
    //    prefix with "/" in the input buffer; otherwise,
    else if (!strncmp("/./", copy, 3)) {
      copy += 2;
      copy_len -= 2;
    }
    else if (!strcmp("/.", copy)) {
      copy[1] = '/';
      copy++;
      copy_len -= 1;
    }

    // C. if the input buffer begins with a prefix of "/../" or "/..",
    //    where ".." is a complete path segment, then replace that
    //    prefix with "/" in the input buffer and remove the last
    //    segment and its preceding "/" (if any) from the output
    //    buffer; otherwise,
    else if (!strncmp("/../", copy, 4)) {
      copy += 3;
      copy_len -= 3;

      // remove the last segment
      while (outptr > out) {
        outptr--;
        if (*outptr == '/')
          break;
      }
      *outptr = 0;
    }
    else if (!strcmp("/..", copy)) {
      copy[2] = '/';
      copy += 2;
      copy_len -= 2;

      // remove the last segment
      while (outptr > out) {
        outptr--;
        if (*outptr == '/')
          break;
      }
      *outptr = 0;
    }

    // D. if the input buffer consists only of "." or "..", then remove
    //    that from the input buffer; otherwise, */
    else if (!strcmp(".", copy) || !strcmp("..", copy)) {
      *copy = 0;
    }

    // E. move the first path segment in the input buffer to the end of
    //    the output buffer, including the initial "/" character (if
    //    any) and any subsequent characters up to, but not including,
    //    the next "/" character or the end of the input buffer. */
    else {
      do {
        *outptr++ = *copy++;
        copy_len--;
      } while (*copy && (*copy != '/'));
      *outptr = '\0';
    }
  } while (*copy != '\0');    // keep going

  // cleanup copy
  free(copyptr);

  return out;
}

/* ***************************************************************** */
/*
 * fixRelativeURL - resolves a relative url to an absolute url
 * @base: base url to resolve from
 * @rel: relative url to resolve
 * @len: length of the relative url
 *
 * Returns a newly allocated character buffer that represents the
 * absolute url from the base and relative urls. Returns NULL if
 * an absolute url cannot be established.
 *
 * This is a quick attempt at RFC 3986 section 5.2.
 */

static char* 
fixRelativeURL(char* base, char* rel, size_t len)
{
  char* abs_url;                           // absolute url to build
  char* slash;                             // right-most '/' in a path
  struct URL tmp;                          // parsed url

  // we need a base url to work with
  if (!base) {
    return NULL;
  }

  // allocate new absolute url
  abs_url = calloc(strlen(base) + len + 2, sizeof(char));
  if (!abs_url) {
    return NULL;
  }

  // attempt to parse the base url
  if (!parseURL(base, &tmp)) {
    free(abs_url);                       // cleanup the absolute url

    abs_url = NULL;                      // going to return NULL
    goto cleanup;                        // sorry Dijkstra
  }

  // put absolute url back together
  if (tmp.scheme) {                         // scheme
    strcat(abs_url, tmp.scheme);
  }

  if (tmp.user) {                           // user
    strcat(abs_url, tmp.user);
  }

  if (tmp.host) {                           // host
    strcat(abs_url, tmp.host);              // always ends in slash
  }

  // do we have a relative url?
  if (rel) { // yes, add it to the abs_url
    // is the relative URL relative to domain root, or relative to base?
    if (rel[0] == '/') {
      // relative to domain root
      strncat(abs_url, rel, len);      // add relative url
    } else {
      // relative to base_url
      // add the base path up to the right-most '/'
      if (tmp.path
          && (slash = strrchr(tmp.path, '/'))
          && (slash != tmp.path)) {
        strncat(abs_url, tmp.path, slash - tmp.path);
      }
      strcat(abs_url, "/");            // separate base and relative path
      strncat(abs_url, rel, len);      // add relative url
    }
  } else { // no relative url; finish the abs_url
    // DFK: not sure this case occurs, or if it does, what action to take.
    // add the base path up to the right-most '/'
    if (tmp.path
        && (slash = strrchr(tmp.path, '/'))
        && (slash != tmp.path)) {
      strncat(abs_url, tmp.path, slash - tmp.path);
    }
  }

  // we can ignore the base query and fragment, they shouldn't apply

 cleanup:                                     // cleanup memory
  freeURL(tmp);

  return abs_url;
}


/* ***************************************************************** */
/*
 * removeWhitespace - removes whitespace from str
 * @str: char buffer to modify
 *
 * Eliminates whitespace by shifting and condensing all the non-whitespace
 * toward the beginning of the buffer. This does not alter the size of the
 * buffer.
 *
 * Should have no use outside of this file, thus declared static.
 */
static void
removeWhitespace(char* str)
{
  char* prev;                              // previous whitespace
  char* cur;                               // current non-whitespace

  // start at beginning of str
  cur = prev = str;

  do {
    while (isspace(*cur)) cur++;           // consume any whitespace
  } while ((*prev++ = *cur++));            // condense to front of str
}

/* **************** isBlankLine ******************/
/* Input: line, a non-NULL pointer to a string.
 * Return true if the string is pointing to a blank line, that is, 
 * an empty string, or
 * with only newline (LF), only carriage return (CR), or only CRLF.
 */
static inline bool
isBlankLine(const char* line)
{
  return (   (line[0] == '\0')
             || (strcmp(line, "\n") == 0) 
             || (strcmp(line, "\r") == 0) 
             || (strcmp(line, "\r\n") == 0));
}
