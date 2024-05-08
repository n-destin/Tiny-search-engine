#!/bin/bash

# Compile the common library
make -C ../common

# Compile the indexer program
make

# Compile the indextest program
make indextest

# Define a function to simplify running commands and checking exit status
run_test() {
    "$@"
    if [ $? -ne 0 ]; then
        echo "Error: Command failed with exit status $?"
    else
        echo "Success: Command completed successfully"
    fi
}

echo "Test case 1: Incorrect number of arguments"
run_test ./indexer

echo "Test case 2: Invalid directory"
run_test ./indexer ../data ../indexing/testing

echo "Test case 3: Valid testing for building an indexer"
run_test ./indexer ../data/letters ./indexer_output

echo "Test case 4: Valid testing for building an indextester"
run_test ./indextest ./indexer_output ./indextester_output

# Clean up test artifacts
rm -rf valid_directory
echo "Testing complete."
