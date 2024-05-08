/*
    Destin Niyomufasha
    CS 50 word.c file tse
    Containing the word processing functions
*/



#include "word.h"
#include <ctype.h>

void word_normalize(char* word) {
    for (int i = 0; word[i]; i++) {
        word[i] = tolower(word[i]);
    }
}