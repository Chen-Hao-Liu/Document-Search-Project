#ifndef HASHMAP_H
#define HASHMAP_H

struct wordNode {
	char* word;
	int df; 
	struct wordNode* nextWord; 
	struct llnode* nodeHead;
};

struct llnode {
    int document_id;
    int num_occurrences;
    struct llnode* nextNode;
};

struct hashmap {
    struct wordNode** map;
    int num_buckets;
    int num_elements;
};

struct hashmap* hm_create(int num_buckets);
int hm_get(struct hashmap* hm, char* word, int document_id);
void hash_table_insert(struct hashmap* hm, char* word, int document_id, int num_occurrences);
void hm_remove(struct hashmap* hm, char* word, int document_id);
void hm_destroy(struct hashmap* hm);
int hash_code(struct hashmap* hm, char* word);


#endif
