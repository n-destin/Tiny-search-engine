/* 
 * counters.h - header file for CS50 counters module
 *
 * A "counter set" is a set of counters, each distinguished by an integer key.
 * It's a set - each key can only occur once in the set - but instead of
 * storing (key,item) pairs, it tracks a counter for each key.  It starts
 * empty. Each time `counters_add` is called on a given key, that key's
 * counter is incremented. The current counter value can be retrieved by
 * asking for the relevant key.
 * 
 * David Kotz, April 2016, 2017, 2019, 2021
 * Xia Zhou, July 2017
 */

#ifndef __COUNTERS_H
#define __COUNTERS_H

#include <stdio.h>
#include <stdbool.h>

/**************** global types ****************/
typedef struct counters counters_t;  // opaque to users of the module

/**************** functions ****************/

/**************** FUNCTION ****************/
/* Create a new (empty) counter structure.
 *
 * We return:
 *   pointer to a new counterset; NULL if error (out of memory).
 * We guarantee:
 *   counterset is intialized empty.
 * Caller is responsible for:
 *   later calling counters_delete();
 */
counters_t* counters_new(void);

/**************** counters_add ****************/
/* Increment the counter indicated by key.
 * 
 * Caller provides:
 *   valid pointer to counterset, and key (must be >= 0)
 * We return:
 *   the new value of the counter related to the indicated key.
 *   0 on error (if ctrs is NULL, key is negative, or out of memory)
 * We guarantee:
 *   the counter's value will be >= 1, on successful return.
 * We do:
 *  ignore if ctrs is NULL or key is negative.
 *  if the key does not yet exist, create a counter for it and initialize to 1.
 *  if the key does exist, increment its counter by 1.
 */
int counters_add(counters_t* ctrs, const int key);

/**************** counters_get ****************/
/* Return current value of counter associated with the given key.
 *
 * Caller provides:
 *   valid pointer to counterset, and key (must be >= 0)
 * We return:
 *   current value of counter associated with the given key, if present;
 *   0 if ctrs is NULL, key < 0, or key is not found.   
 * Note:
 *   counterset is unchanged as a result of this call.
 */
int counters_get(counters_t* ctrs, const int key);

/**************** counters_set ****************/
/* Set the current value of counter associated with the given key.
 *
 * Caller provides:
 *   valid pointer to counterset, 
 *   key (must be >= 0), 
 *   counter value (must be >= 0).
 * We return:
 *   false if ctrs is NULL, if key < 0 or count < 0, or if out of memory.
 *   otherwise returns true.
 * We do:
 *   If the key does not yet exist, create a counter for it and initialize to
 *   the given value. 
 *   If the key does exist, update its counter value to the given value.
 */
bool counters_set(counters_t* ctrs, const int key, const int count);

/**************** counters_print ****************/
/* Print all counters; provide the output file.
 *
 * Caller provides:
 *   valid pointer to counterset, 
 *   FILE open for writing.
 * We print:
 *   Nothing if NULL fp. 
 *   "(null)" if NULL ctrs.
 *   otherwise, comma-separated list of key=counter pairs, all in {brackets}.
 */
void counters_print(counters_t* ctrs, FILE* fp);

/**************** counters_iterate ****************/
/* Iterate over all counters in the set.
 *
 * Caller provides:
 *   valid pointer to counterset, 
 *   arbitrary void* arg,
 *   valid pointer to itemfunc that can handle one item.
 * We do:
 *   nothing, if ctrs==NULL or itemfunc==NULL.
 *   otherwise, call itemfunc once for each item, with (arg, key, count).
 * Note:
 *   the order in which items are handled is undefined.
 *   the counterset is unchanged by this operation.
 */
void counters_iterate(counters_t* ctrs, void* arg, 
                      void (*itemfunc)(void* arg, 
                                       const int key, const int count));

/**************** counters_delete ****************/
/* Delete the whole counterset.
 *
 * Caller provides:
 *   a valid pointer to counterset.
 * We do:
 *   we ignore NULL ctrs.
 *   we free all memory we allocate for this counterset.
 */
void counters_delete(counters_t* ctrs);

#endif // __COUNTERS_H
