# CS50 Tiny Search Engine (TSE) utility library

These modules support the TSE project.  Feel free to drop in your implementation of the data-structure modules, but **do not change any of the other source files in this directory.**

## Usage

To build `libcs50.a`, run `make`. 

The starter kit includes a pre-built library, `libcs50-given.a`, in case you prefer to use our Lab3 solutions rather than your own.
If you prefer our data-structure implementation over your own, update the Makefile rule for `$(LIB)`, as instructed by comments there.

To clean up, run `make clean`.

## Overview

 * `bag` - the **bag** data structure from Lab 3
 * `counters` - the **counters** data structure from Lab 3
 * `file` - functions to read files (includes readLine)
 * `hashtable` - the **hashtable** data structure from Lab 3
 * `hash` - the Jenkins Hash function used by hashtable
 * `memory` - handy wrappers for malloc/free
 * `set` - the **set** data structure from Lab 3
 * `webpage` - functions to load and scan web pages
