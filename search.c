#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <glob.h>
#include <ctype.h>
#include "hashmap.h"
#define MAX_CHARS_PER_WORD 100
#define MAX_CHARS_PER_LINE 100

//function prototypes
void printDoc(char **paths, int N, int id);
int num_query(char *query);
char** read_query(char *query, int n);
void rank(struct hashmap* myMap, char** query, char** paths, int Q, int N);
double rankHelper(struct hashmap* myMap, char* word, int N);
struct hashmap* training(int num_buckets, char** paths, int num_files);
int stringToInt(char* str, int j);
void stop_word(struct hashmap* myMap, int* idArray, int N);
void wordExtract(struct hashmap* myMap, int doc_id, char* filename);
void printPaths(int document_id, char *paths);
void printMap(struct hashmap* map);

int main(int argc, char **argv){
	//Avoid unused variable error
	argc = argc;

	//First determine the number of buckets
	char *buckets = argv[1];
	int bindex = 0;
	//Determine bindex, how many digits in the number representing num_buckets
	while(buckets[bindex] != '\0'){
		bindex++;
  	}

  	//Input number of buckets as a string and bindex to acquire num_buckets as int
	int num_buckets = stringToInt(buckets, bindex);
  	//Number of words in user search query
  	int num_search = num_query(argv[2]);
	//num_files represents the number of documents in database
	int num_files;
	//Call read_query to acquire array of words
	char **userQuery = NULL;

	//First check that num_search != 0
	if(num_search != 0){
		userQuery = read_query(argv[2], num_search);
	}else{
		printf("No words entered into search query!\n");
	}

	//Use glob to acquire the document paths in the directory
	char *searchStr = "p5docs/*.txt";
	glob_t result;
	//glob call
	glob(searchStr, GLOB_ERR, NULL, &result);
	//Initialize the number of files and set paths pointer to the array of filepaths
	num_files = result.gl_pathc;
	char** paths = result.gl_pathv;
	
	//If number of files is 0, skip over everything
	if(num_files != 0){
		//acquire the hashmap by calling training
		struct hashmap* myMap = training(num_buckets, paths, num_files);

		//printMap(myMap);

		//Prints out ranked list
		rank(myMap, userQuery, paths, num_search, num_files);

		//Free everything used	
		hm_destroy(myMap);
	}else{
		printf("Error: No documents in directory!\n");
	}
	
	//Check to see if userQuery needs to be freed
	if(num_search != 0){
		int i;
		for(i=0; i<num_search; i++){
			free(userQuery[i]);
		}		
		free(userQuery);
	}

	//Remember to free glob
	globfree(&result);
}

//Prints out the contents of document
void printDoc(char **paths, int N, int id){
	int doc_id;
	int i;
	int k;
	//num is used to acquire the document_id from the path string
	char num[12];
	char *filepath;

	//Parse through each file
	for(i=0; i<N; i++){
		//Initialize k
		k=0;

		//First, increment until k reaches index of end string char
		while(paths[i][k] != '\0'){
			k++;
		}

		//From there, parse backwards until the D char is reached
		while(paths[i][k] != 'D'){
			k--;
		}

		//Increment k to acquire the first char of document_id
		k++;
		int j = 0;

		//Increment until . char is reached to isolate document_id
		while(paths[i][k] != '.'){
			//Set character for num
			num[j] = paths[i][k];
			//Increment both indexes
			j++;
			k++;
		}

		//Add end string character
		num[j] = '\0';

		//Convert string to int and initialize ranks[i][0] with the id
		//initialize ranks[i][1] with 0
		doc_id = stringToInt(num, j);
		if(doc_id == id){
			filepath = paths[i];
			break;
		}
	}	

	//Need an extra char for end of string
	char inputLine[MAX_CHARS_PER_LINE + 1];
	char ch; //Variable in which each char is read
	int cursorPosition = 0; //Position in inputLine for current char
	FILE *dataFile = fopen(filepath, "r"); //Open the file
	ch = fgetc(dataFile); //Acquire first character

	//Keep reading chars until EOF char is read
	while(ch != EOF){
		//If we haven't seen the end of a line, put char into current inputLine
		if(ch != '\n'){
			//Check whether we have exceeded the space allotted
			if(cursorPosition >= MAX_CHARS_PER_LINE){
				//Can't append
				printf("Input line size exceeded - exiting program\n");
				exit(0);
			}

			//There is space available so append char to inputLine
			inputLine[cursorPosition] = ch;
			cursorPosition++;
		}else{
			//Place end of string character
			inputLine[cursorPosition] = '\0';
			//Print buffer
			printf("%s\n", inputLine);
			//Reset cursor to 0
			cursorPosition = 0;
		}

		//Acquire next character
		ch = fgetc(dataFile);
	}

	//Check to see if last line is in buffer
	if(cursorPosition != 0){
		//Place end of string character
		inputLine[cursorPosition] = '\0';
		//Print buffer
		printf("%s\n", inputLine);
	}
	
	//Close file
	fclose(dataFile);
}

//Helper method for finding the amount of words in a userquery string
int num_query(char *query){
	int i = 0;
	//flag represents whether or not ready to record new word
	int flag = 1;
	//n represents number of words in query
	int n = 0;

	//Logic for acquiring number of words
	while(query[i] != '\0'){
		//If ' ' is not detected
		if(query[i] != ' '){
			//If ready to record new word
			if(flag == 1){
				//Increment number of words
				n++;
				//No longer ready to record new word
				flag = 0;
			}
		}else{
			//When new space is detected, ready to record new word
			flag = 1;
		}
		//Increment index to acquire next char
		i++;
	}

	return n;
}

char** read_query(char *query, int n){
	//i is used as loop index
	int i;
	//j is the index used to parse through query
	int j = 0;
	//k keeps track of the number of characters recorded in each word
	int k;

	//Dynamically allocate memory for array of query words with n
	char **wordQuery = (char**) malloc(n * sizeof(char*));

	//Populate wordQuery
	for(i=0; i<n; i++){
		//Allocate heap memory for each word in query
		wordQuery[i] = (char*) malloc(21 * sizeof(char));
		//Initialzie k to 0
		k = 0;
		//Checks to see if end of lin character was reached
		while(query[j] != '\0'){
			//Checks to see if space was detected
			if(query[j] == ' '){
				//If a word was recorded in buffer (k>0)
				if(k != 0){
					//Add end string character
					wordQuery[i][k] = '\0';
					//Increment index for query
					j++;
					//Reset k to 0
					k = 0;
					//Break from loop to move onto next word
					break;
				}else{
					//If k == 0, no word was recorded, increment index
					j++;
				}
			}else{
				//If char was not space, record character for word
				wordQuery[i][k] = tolower(query[j]);
				//Increment k (char count) and j (query index)
				k++;
				j++; 
			}
		}

		//Check to see if characters had been recorded in buffer
		if(k != 0){
			//If so, add end string character
			wordQuery[i][k] = '\0';
		}
	}

	return wordQuery;
}

//Ranking method
void rank(struct hashmap* myMap, char** query, char** paths, int Q, int N){
	//Dynamically allocated 2D array for storing document_id and 
	//ranks[i][0] represents the document id
	//ranks[i][1] represents the corresponding tf-idf ranking for that doccument
	double **ranks = (double**) malloc(N * sizeof(double*));

	//num is used to acquire the document_id from the path string
	char num[12];
	//k is used to keep track of the index for acquiring document_id
	int k;
	//i and j are used for parsing through for loop
	int i;
	int j;
	//idf represents inverse document frequency
	double idf;
	//Open search_scores.txt to write scores
	FILE *writeToDoc = fopen("search_scores.txt", "a");

	//Part 1:====================================================================================
	//Parse through each file
	for(i=0; i<N; i++){
		//dynamically allocate 2D array
		ranks[i] = (double*) malloc(2 * sizeof(double));

		//Initialize k
		k=0;

		//First, increment until k reaches index of end string char
		while(paths[i][k] != '\0'){
			k++;
		}

		//From there, parse backwards until the D char is reached
		while(paths[i][k] != 'D'){
			k--;
		}

		//Increment k to acquire the first char of document_id
		k++;
		int j = 0;

		//Increment until . char is reached to isolate document_id
		while(paths[i][k] != '.'){
			//Set character for num
			num[j] = paths[i][k];
			//Increment both indexes
			j++;
			k++;
		}

		//Add end string character
		num[j] = '\0';

		//Convert string to int and initialize ranks[i][0] with the id
		//initialize ranks[i][1] with 0
		ranks[i][0] = stringToInt(num, j);
		ranks[i][1] = 0;
	}	

	//Part 2:====================================================================================
	//Increment through query words
	for(j=0; j<Q; j++){
		//Acquire idf for word
		idf = rankHelper(myMap, query[j], N);
		
		//Increment through list of documents
		for(k=0; k<N; k++){
			//Acquire number of occurrences using hm_get
			int occurrence = hm_get(myMap, query[j], ranks[k][0]);
			//If occurrence != -1, the node exists
			if(occurrence != -1){
				//Add onto ranks accordingly
				ranks[k][1] += (double) occurrence * idf;
			}
		}
	}

	//Part 3:====================================================================================
	int highestID;
	double highestRank = 0.0;
	double largest;

	for(i=0; i<N; i++){
		//Initialize largest to 0.0
		largest = 0.0;

		//Increment through ranks
		for(j=0; j<N; j++){
			if(ranks[j][1] >= largest){
				//Set largest to new largest
				largest = ranks[j][1];
				//Keep track of index of largest
				k = j;
			}
		}

		//Update highestID and highestRank with the highest ranked id and rank in ranks[][] respectively
		if(i == 0){
			highestID = (int) ranks[k][0];
			highestRank = ranks[k][1];
		}

		//Write information to the searchfile
		fprintf(writeToDoc, "D%d.txt: %lf\n", (int) ranks[k][0], ranks[k][1]);
		//Clear ranks[k][1] so the next largest can be found during the next loop phase
		ranks[k][1] = -1;
	}

	//Remember to close the file after use
	fclose(writeToDoc);

	//Free double array
	for(i=0; i<N; i++){
		free(ranks[i]);
	}
	free(ranks);
	
	//If Q == 0, no terms are in search query. If the highest ranked == 0, no matches found	
	if(Q == 0){
		printf("No matches found because search query is empty!\n");
	}else if(N == 1){
		printf("Only one document in the directory. May or may not be relevant to your search:\n");
		printDoc(paths, N, highestID);
	}else if(highestRank == 0.0){
		printf("No matches found!\n");
	}else{
		printDoc(paths, N, highestID);
	}

	return;
}

//Parameters: hashmap, search word, total number of documents (N)
double rankHelper(struct hashmap* myMap, char* word, int N){
	//Acquires hash
	int myhash = hash_code(myMap, word);
	int df;

	//Initialize index
	struct wordNode* index = myMap->map[myhash];

	//Parse through map until word is found
	while(index != NULL){
		//If word is found
		if(strcmp(index->word, word) == 0){
			df = index->df;
			//Return the idf
			return log10(((double) N)/((double) df));
		}

		//Increment index
		index = index->nextWord;
	}

	//If word is not in the hashmap, return default
	return log10(((double) N)/1.0);
}

//Training method for populating hashmap
struct hashmap* training(int num_buckets, char** paths, int num_files){
	//Declare variables needed
	struct hashmap *map = hm_create(num_buckets);
	int document_id;
	int i;
	int k;
	char num[12];
	int *idArray = (int*) malloc(num_files * sizeof(int));

	//Parse through each file
	for(i=0; i<num_files; i++){
		//Intialize k
		k=0;

		//First, increment until k reaches index of end string char
		while(paths[i][k] != '\0'){
			k++;
		}

		//From there, parse backwards until the D char is reached
		while(paths[i][k] != 'D'){
			k--;
		}

		//Increment k to acquire the first char of document_id
		k++;
		int j = 0;

		//Increment until . char is reached to isolate document_id
		while(paths[i][k] != '.'){
			//Set character for num
			num[j] = paths[i][k];
			//Increment both indexes
			j++;
			k++;
		}

		//Add end string character
		num[j] = '\0';

		//Convert string to int and set to document_id
		document_id = stringToInt(num, j);
		idArray[i] = document_id;

		//Call wordExtract() to input entry into hashmap
		wordExtract(map, document_id, paths[i]);
	}

	//Stop word removal
	stop_word(map, idArray, num_files);
	free(idArray);

	return map;
}

//Helper method for converting string to integer
int stringToInt(char* str, int j){
	//declare variables needed for conversion
	int i = 0;
	int ascii;
	int value = 0;

	//Parses through string to convert to int
	while(str[i] != '\0'){
		//Acquire int by subtracting ascii offset
		ascii = ((int) str[i]) - 48;
		//Apply weight to the digit based on its position
		value += ascii * (int) pow(10, (j-1));
		//Increment indexes
		i++;
		j--;
	}

	//Return converted int value
	return value;
}

//stop_word removal
void stop_word(struct hashmap* myMap, int* idArray, int N){
	//Acquire first index
	struct wordNode* index;
	struct wordNode* prevIndex;
	int i;
	int j;

	//Parse through hashmap
	for(i=0; i<myMap->num_buckets; i++){
		//Acquire head of list for bucket
		index = myMap->map[i];

		//Parse through each bucket
		while(index != NULL){
			//Increment index
			prevIndex = index;
			//Increment index first just in case prevIndex gets freed
			index = index->nextWord;

			//Check to see if idf for word is 0.0, meaning we've found a stop word
			if(log10((double) N/(double) prevIndex->df) == 0.0){
				//Remove the word from every document
				for(j=0; j<N; j++){
					hm_remove(myMap, prevIndex->word, idArray[j]);
				}
			}
		}
	}

	return;
}

//Word extract reads every word from the txt file and inserts it into hashmap
void wordExtract(struct hashmap* myMap, int doc_id, char* filename){
	//Open File and initialize elements
	FILE *dataFile = fopen(filename, "r");

	//Make sure there is enough space for a word
	char inputWord[MAX_CHARS_PER_WORD]; 
	int document_id = doc_id;

	//Initialize chIndex and acquire first character
	int chIndex = 0;
	char ch = fgetc(dataFile);

	//While not the End of File character
	while (ch != EOF){
		//If end of a line and space is not seen, put char into current inputWord
		if(ch != '\n' && ch != ' '){
			//Checks to see if word limit is reached to avoid overflow. Remember, last char is '\0'
			if(chIndex == 99){
				printf("Error: Word exceeds max character limit - exiting program.\n");
				exit(0);
			}

			inputWord[chIndex] = tolower(ch);
			chIndex++;
		}else{
			//Check to see if a valid string was recorded
			if(chIndex != 0){
				//Add end string character
				inputWord[chIndex] = '\0';
				//printf("%s\n", inputWord);
				//Acquire num_occurrences of this word in this document
				int count = hm_get(myMap, inputWord, document_id); 
				
				//-1 means the element does not exist yet
				if(count == -1){ 
					//Add the first one
					hash_table_insert(myMap, inputWord, document_id, 1);
				}else{
					//Add one more to existing
					hash_table_insert(myMap, inputWord, document_id, (count+1));
				}

				//Reset chIndex for next word
				chIndex = 0;
			}
		}

		//Acquire next character
		ch = fgetc(dataFile);
	}


	//Check to see if a valid string was recorded
	if(chIndex != 0){
		//Add end string character
		inputWord[chIndex] = '\0';

		//Acquire num_occurrences of this word in this document
		int count = hm_get(myMap, inputWord, document_id);

		//-1 means the element does not exist yet
		if(count == -1){ 
			//Add the first one
			hash_table_insert(myMap, inputWord, document_id, 1);
		}else{
			//Add one more to existing
			hash_table_insert(myMap, inputWord, document_id, (count+1));
		}
	}

	//Close and return
	fclose(dataFile);
	return;
}

//Helper method for printing the hashmap for testing purposes
void printMap(struct hashmap* myMap){
	int i = 0;
	struct wordNode* index;
	struct llnode* index2;

	for(i=0; i<myMap->num_buckets; i++){
		index = myMap->map[i];
		printf("Bucket %d ================================================\n", i);
		while(index != NULL){
			index2 = index->nodeHead;

			printf("-----------%s-----------\n", index->word);
			while(index2 != NULL){
				printf("Occurrence in D%d: %d\n", index2->document_id, index2->num_occurrences);
				index2 = index2->nextNode;
			}
			index = index->nextWord;
			printf("------------------------\n");
		}
		printf("End ======================================================\n");
	}

	return;
}

//Helper method for printing out paths and document id for testing purposes
void printPaths(int document_id, char *paths){
	printf("ID: %d, Pathname: %s\n", document_id, paths);
	return;
}
