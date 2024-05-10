#ifndef QUERIER_H
#define QUERIER_H

#include "../libcs50/hashtable.h"
#include "../libcs50/counters.h"
#include "../common/index.h"

#define MAX_QUERY_LENGTH 1024
#define MAX_WORD_LENGTH 100

// // Structure for storing search results
// typedef struct searchResult {
//     char *page;
//     int count;
// } searchResult_t;

// Structure to hold counters for intersection
typedef struct {
    counters_t *counters_two;
    counters_t *intersection;
} counters_args_t;

// Function Prototypes
counters_t *counters_intersection(counters_t *counters_one, counters_t *counters_two);
void counters_union(counters_t *counters_one, counters_t *counters_two);
void validate_args(int argc, char *argv[]);
counters_t *and_sequence(index_t *index, char **words, int start, int end);
counters_t *or_sequence(index_t *index, char **words, int num_words);
void counters_union_helper(void *arg, int key, int count);
void add_result(void *arg, const int key, const int count);

#endif // QUERIER_H
