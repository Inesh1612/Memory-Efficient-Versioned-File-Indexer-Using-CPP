# CS253 Assignment 1: Report
Inesh Aggarwal 
240465

## 1. Overview

This report outlines the design and implementation of the Memory-Efficient Versioned File Indexer. The system processes large text files incrementally using a fixed-size buffer, without loading the entire file into memory.

## 2. Object-Oriented Design

The system is built using distinct classes. 
It fulfills the requirement of having at least four user-defined classes, each with a specific responsibility:
### version_index
Stores the mapping of unique words to their frequencies for a specific version identifier.
### tokenizer
Processes the raw character buffers to extract valid words (contiguous sequences of alphanumeric characters). It handles case-insensitivity and correctly reconstructs words that are split across buffer boundaries. It provides two overloaded methods: one to collect tokens into a dynamically allocated vector, and another to directly increment a frequency map to prevent extra allocations entirely.
### buffered_file_reader
Safely opens files and reads them into a fixed-size character buffer. It manages the I/O operations and passes data chunks to the tokenizer. 
### query hierarchy & query_processor
An abstract base class `query` defines a common interface for query operations.  
Derived classes:
- `word_count_query`
- `top_k_query`
- `difference_query`
The `query_processor` executes these using runtime polymorphism.

## 3. C++ Requirements Demonstrated

### Inheritance
An abstract base class `query` is defined, from which specific query operations (`word_count_query`, `top_k_query`, `difference_query`) are derived.
### Runtime Polymorphism
The `query` base class declares a pure virtual function `virtual void execute()const=0;`. The `query_processor` executes these dynamically using base class pointers.
### Function Overloading
The code demonstrates function overloading in two distinct classes:
-`version_index::add_word()`:
One version takes just a string (incrementing the count by 1), while the overloaded version takes a string and a specific integer count.
-`tokenizer::tokenize()`:
One version returns a `vector<string>`, while the overloaded version directly modifies a passed `unordered_map` reference to eliminate memory overhead.
### Exception Handling
The program uses `try`, `catch`, and `throw` blocks in `main()` to handle runtime errors, including a custom `buffer_size_exception` for invalid buffer configurations.
### Templates
A user-defined template function `to_lowercase<T>()` is implemented for generic case conversion.

## 4. Version Management

Each input file corresponds to a user-defined version identifier. The system maintains separate indices for each version within the same execution, allowing comparison-based queries such as difference queries.

## 5. Memory Management & Buffer Constraints

The system uses a fixed-size buffer (256 KB – 1024 KB) to read files incrementally.
Memory usage is:
- Independent of total file size
- Dependent only on number of unique words stored 
Boundary token splitting is handled by maintaining a `left` string that carries the trailing characters of a chunk into the next buffer processing cycle.

## 6. Output Format

For every execution, the program outputs:
- Version name
- Query result
- Buffer size in KB
- Execution time in seconds

## 7. Time Complexity Analysis

Let:
- N = total characters in file
- M = number of unique words

File Reading & Tokenization: O(N)  
Word Count Query: O(1) average  
Difference Query: O(1) average  
Top-K Query: O(M log M) (due to sorting)

## 8. Space Complexity

Space complexity is O(M), where M is the number of unique words stored in the `unordered_map`.

## 9. Execution Instructions

Compile:
g++ -std=c++17 240465_Inesh.cpp -o analyzer

Word Query:
./analyzer --file dataset_v1.txt --version v1 --buffer 512 --query word --word error

Difference Query:
./analyzer --file1 dataset_v1.txt --version1 v1 --file2 dataset_v2.txt --version2 v2 --buffer 512 --query diff --word error

Top-K Query:
./analyzer --file dataset_v1.txt --version v1 --buffer 512 --query top --top 10

## 10. Conclusion

The system successfully demonstrates memory-efficient versioned file indexer using object-oriented design principles and C++ features. It satisfies all assignment requirements including inheritance, polymorphism, templates, exception handling, and function overloading while maintaining efficient time and space complexity.