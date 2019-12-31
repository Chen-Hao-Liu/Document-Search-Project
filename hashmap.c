#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hashmap.h"

struct hashmap* hm_create(int num_buckets){
	//Dynamically allocate space for hashmap
	struct hashmap *myMap = (struct hashmap*) malloc(sizeof(struct hashmap));

	//Dynamically allocate space for wordNode array.
	myMap->map = (struct wordNode**) malloc(num_buckets * sizeof(struct wordNode*));

	//Initialize to NULL to avoid malloc garbage causing infinite loop
	int i;
	for(i=0; i<num_buckets; i++){
		myMap->map[i] = NULL;
	}

	//Initialize num_buckets and num_elements
	myMap->num_buckets = num_buckets;
	myMap->num_elements = 0;

	return myMap;
}

int hm_get(struct hashmap* hm, char* word, int document_id){
	//Acquire hash value
	int myHash = hash_code(hm, word);
	struct wordNode* index;
	struct llnode* index2; 

	//Acquire bucket
	index = hm->map[myHash];

	//Parse through this bucket linked list
	while(index != NULL){
		//Check to see if the word matches
		if(strcmp(index->word, word) == 0){
			//printf("yes\n");
			index2 = index->nodeHead;

			//Parse through the linked list
			while(index2 != NULL){
				//check to see if document is correct
				if(index2->document_id == document_id){
					//When found, return num_occurrences
					return index2->num_occurrences;
				}
				//Increment index
				index2 = index2->nextNode;
			}
		}
		//Increment index
		index = index->nextWord;
	}

	//Error, element does not exist in hashmap
	return -1;
}

void hash_table_insert(struct hashmap* hm, char* word, int document_id, int num_occurrences){
	//Deep copy inputWord over to new inputString so changing the old
	//version doesn't change new one.
	char *inputString = strdup(word);

	//Acquire hash value
	int myHash = hash_code(hm, word);

	//Declare pointers for parsing through map
	struct wordNode* index = NULL;
	struct wordNode* prevIndex = NULL;
	struct llnode* index2 = NULL;
	struct llnode* prevIndex2 = NULL;

	//Acquire bucket
	index = hm->map[myHash];
	prevIndex = index;

	//Parse through this bucket linked list
	while(index != NULL){
		//Check to see if the word matches
		if(strcmp(index->word, word) == 0){
			//Free inputString if the word already exists
			free(inputString);

			//Declare index pointers to parse through linkedlist
			index2 = index->nodeHead;
			prevIndex2 = index2; 

			while(index2 != NULL){
				//Check to see if document_id matches
				if(index2->document_id == document_id){
					//Increment num_occurrences
					index2->num_occurrences = num_occurrences;
					
					//Job done, return
					return; 
				}
				//Increment pointers
				prevIndex2 = index2;
				index2 = index2->nextNode;
			}
			
			//set index equal to new struct and populate with given values
			index2 = (struct llnode*) malloc(sizeof(struct llnode));
			index2->document_id = document_id;
			index2->num_occurrences = num_occurrences;
			index2->nextNode = NULL;

			//If prevIndex2 == NULL, list was empty. Add index and increase num_elements
			if(prevIndex2 == NULL){
				hm->num_elements++;
				index->df++;
				index->nodeHead = index2; 	
			}else{
				hm->num_elements++;
				index->df++;
				prevIndex2->nextNode = index2; //Set next to index
			}

			//Job is done, return
			return;
		}

		//Keeps track of previous pointer for insertion
		prevIndex = index;
		index = index->nextWord;
	}

	//set index equal to new wordNode and populate values
	index = (struct wordNode*) malloc(sizeof(struct wordNode));
	index->word = inputString;
	index->df = 0;
	index->nextWord = NULL;
	index->nodeHead = NULL;

	//set index2 to new llnode and populate values
	index2 = (struct llnode*) malloc(sizeof(struct llnode));
	index2->document_id = document_id;
	index2->num_occurrences = num_occurrences;
	index2->nextNode = NULL;

	//set nodeHead to this new llnode
	index->nodeHead = index2;

	//If prevIndex == NULL, list was empty. Add index and increase num_elements and increase df
	if(prevIndex == NULL){
		hm->num_elements++;
		index->df++;
		hm->map[myHash] = index; 	
	}else{
		hm->num_elements++;
		index->df++;
		prevIndex->nextWord = index; //Set next to index
	}

	return;	
}

//Removes the key value pair in the HashMap that is associated with the given key
void hm_remove(struct hashmap* hm, char* word, int document_id){
	//Acquire hash value
	int myHash = hash_code(hm, word);
	struct wordNode* index = NULL;
	struct wordNode* prevIndex = NULL;
	struct llnode* index2 = NULL;
	struct llnode* prevIndex2 = NULL;

	//Determines if index is first in list
	int firstWord = 1;
	int firstNode = 1;

	//Acquire bucket
	index = hm->map[myHash];
	prevIndex = index;

	//Check for NULL to avoid seg fault
	while(index != NULL){
		//Found the right wordNode
		if(strcmp(index->word, word) == 0){
			//Pointers for incrementing through llnode
			index2 = index->nodeHead;
			prevIndex2 = index2;

			//Test for NULL to avoid seg fault
			while(index2 != NULL){
				//Test for matching document_id
				if(index2->document_id == document_id){
					//Tests if this is the first node in list
					if(firstNode == 1){
						//Set nodeHead to next node
						index->nodeHead = index2->nextNode;
					}else{
						//Skip over index2
						prevIndex2->nextNode = index2->nextNode;
					}

					//Free index
					free(index2);
					//Adjust num_elements
					hm->num_elements--;
					//Adust df
					index->df--;

					//If after removal, df of word is zero, remove word
					if(index->df == 0){
						//Tests if this is the first node in list
						if(firstWord == 1){
							//If word is first node of list, set bucket pointer to nextWord
							hm->map[myHash] = hm->map[myHash]->nextWord;
						}else{
							//Skip over index
							prevIndex->nextWord = index->nextWord;
						}
						
						//Free malloc allocated word
						free(index->word);
						//Free index itself
						free(index);
					}

					//Job is done, return
					return;
				}

				//We are no longer at the first node index
				firstNode++;
				//Keeps track of previous pointer for insertion
				prevIndex2 = index2;
				index2 = index2->nextNode;
			}
		}

		//We are no longer at the first word index
		firstWord++;
		//Keeps track of previous pointer for insertion
		prevIndex = index;
		index = index->nextWord;
	}

	//Node to be removed is not found
	return;
}

void hm_destroy(struct hashmap* hm){
	int num = hm->num_buckets;
	int i;
	//Declare pointers for parsing through lists
	struct wordNode* index = NULL;
	struct wordNode* prevIndex = NULL;
	struct llnode* index2 = NULL;
	struct llnode* prevIndex2 = NULL;
	
	//Iterate through hashmap
	for(i=0; i<num; i++){
		//Initialize index and prevIndex
		index = hm->map[i];
		
		//Increment through linkedlist
		while(index != NULL){
			//Set prevIndex to index
			prevIndex = index;

			//Initialize our index pointer to parse through list
			index2 = prevIndex->nodeHead; 

			while(index2 != NULL){
				//Set prevIndex2 to index2 
				prevIndex2 = index2;
				//Increment index2 to avoid losing pointer
				index2 = index2->nextNode;

				//Free current index;
				free(prevIndex2);
			}

			//Set nodeHead to NULL
			prevIndex->nodeHead = NULL;
			//Increment index to avoid losing pointer
			index = index->nextWord;

			//Word is malloc allocated heap due to strdup, so also needs to be freed
			free(prevIndex->word);
			free(prevIndex);
		}

		//Set map[i] to NULL
		hm->map[i] = NULL;
	}

	//Free map array of linkedlists and then free hashmap
	free(hm->map);
	free(hm);

	//Be sure to set map pointer to NULL to avoid problems
	hm = NULL;
}

int hash_code(struct hashmap* hm, char* word){
	//Initialize Ascii value counter to 0
	int totalAscii = 0;
	//Acquire char index
	int i = 0;

	//Loop through word until hits end string character
	while(word[i] != '\0'){
		//Add current char's ascii value onto total
		totalAscii = totalAscii + (int) word[i];

		//Increment char pointer
		i++;
	}

	//Return the totalAscii value modulo num_buckets as hash value
	return totalAscii % (hm->num_buckets);
}
