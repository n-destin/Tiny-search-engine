#ifndef INDEX_H
#define INDEX_H

#include "../libcs50/hashtable.h"

typedef struct index index_t; 

index_t* index_new(int num_slots);

void index_build(char* pageDirectory, index_t* index);
bool index_insert(index_t *index, const char* key, void *item);
void *index_find(index_t *index, const char *key);
void index_save(const char *fname, index_t *index);
void index_load(const char *fname, index_t *index);
void index_delete(index_t *index);

#endif // INDEX_H
