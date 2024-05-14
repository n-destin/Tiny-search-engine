#!/bin/bash

# Define the location of the querier executable and the test data
QUERIER_EXEC="./querier"
PAGE_DIRECTORY="../data/letters"
INDEX_FILENAME="../indexer/indexer_output"
OUTPUT_FILE="test_output.out"

# Check if the querier executable exists
if [ ! -f "$QUERIER_EXEC" ]; then
    echo "Querier executable not found at $QUERIER_EXEC" &> $OUTPUT_FILE
    exit 1
fi

# Check if the page directory exists
if [ ! -d "$PAGE_DIRECTORY" ]; then
    echo "Page directory not found at $PAGE_DIRECTORY" &> $OUTPUT_FILE
    exit 1
fi

# Check if the index file exists
if [ ! -f "$INDEX_FILENAME" ]; then
    echo "Index file not found at $INDEX_FILENAME" &> $OUTPUT_FILE
    exit 1
fi

# Start the interactive session with the querier executable
echo "Enter queries one by one, and press Control-D to finish:"
$QUERIER_EXEC $PAGE_DIRECTORY $INDEX_FILENAME

# Check the exit status
if [ $? -ne 0 ]; then
    echo "Error during the querier session" &>> $OUTPUT_FILE
fi
