# Document-Search-Project

The purpose of this project is to implement a method for parsing through many documents in a directory and printing out
the contents of the document of most interest. The program parses through the contents of each txt document in the 
specified folder and loads each word into a hashtable which specifies the number of times each word appears in each document.
The program uses an algorithm for detecting and eliminating "stop words", trivial words such as common articles. The documents
are ranked based on their order of relevance based on the key words from the user input search query.

Commands:
- make
- make clean
- ./search 25 "search query words" 
(25 is the number of buckets specified for the hashtable. Words in quotes are search query key words)
