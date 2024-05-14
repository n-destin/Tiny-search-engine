# Querier Program Documentation
By Destin Niyomufasha

## Overview
The Querier is a component of a larger search engine system, responsible for processing user queries against an inverted index created by the Indexer. It handles complex queries involving logical AND and OR operations across multiple words.

## Requirements
- C compiler (e.g., GCC)
- Custom libraries from `libcs50` and `common` directories.

## Structure
The program utilizes several data structures and libraries to manage and process data efficiently:
- **`libcs50`**: Provides foundational data structures like `hashtable`, `counters`, etc.
- **`common`**: Includes utility functions for handling index files and validating directories.

### Key Constants
- `MAX_QUERY_LENGTH`: Defines the maximum length for a user's search query.
- `MAX_WORD_LENGTH`: Sets the upper limit for individual word length within a query.

### Data Structures
- **`index_t`**: Represents the inverted index.
- **`searchResult_t`**: Stores individual search results including the page number, rank, and URL.
- **`counters_args_t`**: Facilitates passing multiple arguments in counter operations, essential for AND/OR processing.

### run with:
./querier [pageDirectory] [indexFilename]