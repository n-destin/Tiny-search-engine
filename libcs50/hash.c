/* =========================================================================
 * hash.c - Jenkins' Hash, maps from string to integer
 *
 * Implementation details can be found at:
 *     http://www.burtleburtle.net/bob/hash/doobs.html
 * ========================================================================= 
 */

#include <string.h>
#include "hash.h" 

// hash_jenkins - see header file for usage
unsigned long
hash_jenkins(const char* str, const unsigned long mod)
{
  if (str == NULL || mod <= 1) {
    return 0;
  }

  size_t len = strlen(str);
  unsigned long hash = 0;

  for (int i = 0; i < len; i++) {
    hash += str[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return (hash % mod);
}
