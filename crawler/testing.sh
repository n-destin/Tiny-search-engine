# Destin Niyomufasha
# COSC 50 Spring 2024
# Testing File

echo "Testing with out of range: Greater than 10: 15"
mkdir -p ../data/letters
./crawler http://cs50tse.cs.dartmouth.edu/tse/wikipedia/Linked_list.html ../data/letters 15
if [ $? -ne 0 ]; then
    echo "Depth should be between 0 and 10"
fi

echo "Testing with out of range: Less than 0: -10"
./crawler http://cs50tse.cs.dartmouth.edu/tse/wikipedia/Linked_list.html ../data/letters -10
if [ $? -ne 0 ]; then
    echo "Depth should be between 0 and 10"
fi

echo "Testing with an unavailable directory"
./crawler http://cs50tse.cs.dartmouth.edu/tse/wikipedia/Linked_list.html ../WAKAKAKA 2
if [ $? -ne 0 ]; then
    echo "The directory not found"
fi

echo "Testing with few number of arguments"
./crawler http://cs50tse.cs.dartmouth.edu/tse/wikipedia/Linked_list.html ../data/letters
if [ $? -ne 0 ]; then
    echo "Few number of parameters"
fi

echo "Testing with Broken Link"
./crawler http://cs50tse.cs.dartmouth.edu/tse/sdjs/Linked_list.html ../data/letters 9 12 23
if [ $? -ne 0 ]; then
    echo "We can't crawl the website"
fi

echo "Testing with many number of arguments"
./crawler http://cs50tse.cs.dartmouth.edu/tse/wikipedia/Linked_list.html ../data/letters 9 12 23
if [ $? -ne 0 ]; then
    echo "Many parameters"
fi

echo "Scraping The website: http://cs50tse.cs.dartmouth.edu/tse/wikipedia/Linked_list.html at depths 0,1,3"
mkdir -p ../data/wikipedia
./crawler http://cs50tse.cs.dartmouth.edu/tse/wikipedia/Linked_list.html ../data/letters 3
if [ $? -eq 0 ]; then
    echo "Successfully crawled the website"
fi
