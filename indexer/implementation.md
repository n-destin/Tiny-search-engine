# CS50 Indexer Project

## Overview
The Indexer project, developed by Destin Niyomufasha, is designed to index pages retrieved by a web crawler. This tool reads webpages from a specified directory and builds an index mapping words to their occurrences across documents. The final index is saved to a file for future use.

## Compilation
To compile the indexer, navigate to the project directory and use the following command:


## Usage
The indexer should be run with two command-line arguments:

1. The directory containing the files produced by the crawler.
2. The path to the file where the index should be saved.

To run the indexer, use the following command:


## Implementation Details

### File Structure
- `indexer.c`: Implements the main functionality of the indexer.
- `index.c`: Contains helper functions and structures used by the indexer.

### Main Components

#### `indexer.c`
- **Main Functionality**:
  - Validates command-line arguments.
  - Checks if the page directory is valid.
  - Creates a new index.
  - Builds the index from the pages.
  - Saves the index to a file.
  - Frees memory and exits.

#### `index.c`
- **Data Structures**:
  - `webpage_t`: Represents a webpage with URL, HTML content, and other metadata.
  - `index_t`: Holds a hashtable where each entry maps a word to a counters structure.

- **Functions**:
  - `index_new(int num_slots)`: Creates a new index.
  - `index_build(char* pageDirectory, index_t* index)`: Builds the index from webpages.
  - `index_insert(index_t *index, const char *key, void *item)`: Inserts a word and its counters into the index.
  - `index_find(index_t *index, const char *key)`: Finds the counters for a word in the index.
  - `index_save(const char *fname, index_t *index)`: Saves the index to a file.
  - `index_load(const char *fname, index_t *index)`: Loads the index from a file.
  - `index_delete(index_t *index)`: Deletes the index and frees associated memory.

### Helpers
- **`index_save_helper(void *arg, const char *key, void *item)`**: Helps save the counters for each word in the file.
- **`counters_save_helper(void* file, const int id, const int count)`**: Saves the ID and count of each word occurrence to the file.
- **`index_load_helper(FILE *fp, index_t *index)`**: Helps load the index from a file.

## Dependencies
- `libcs50`: Provides data structures like hashtable and counters used in the project.
- `pagedir`, `word`, `file`: Utility modules for handling webpages, words, and file operations.

## Error Handling
The indexer includes robust error handling to manage invalid inputs, memory allocation failures, and file operations.

## Contributing
Contributions to the indexer project are welcome. Please ensure that any pull requests or changes maintain the coding standards and functionality of the existing codebase.

