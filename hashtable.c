#include<stdlib.h>
#include<stdint.h>
#include<string.h>

#include"hashtable.h"

typedef struct HashTableEntry{
	struct HashTableEntry* next;
	uint64_t hash;
	uint8_t* key;
	size_t key_size;
	void*  value;
} HashTableEntry;

struct HashTable{
	uint32_t size;
	HashTableEntry* entries[0];
};

#define HASHTABLE_ACCESS_CREATE 1

HashTable* HashTable_new(uint32_t size){
	HashTable* table = (HashTable*)malloc( 
				sizeof(HashTable)+sizeof(HashTableEntry*)*size);

	if(table==NULL){
		return NULL;
	}

	table->size = size;
	for(int i=0;i<size;i++){
		table->entries[i] = NULL;
	}

	return table;
}

void HashTable_del(HashTable* table){
	for(int i=0;i<table->size;i++){
		HashTableEntry* head = table->entries[i];
		while(head){
			HashTableEntry* next = head->next;
			free(head->key);
			free(head);
			head = next;
		}
	}
	free(table);
}


uint64_t HashTable_hash(uint8_t* key,size_t key_size){
	const uint64_t FNV_OFFSET_BASIS = 14695981039346656037U;
	const uint64_t FNV_PRIME = 1099511628211LLU;
	uint64_t hash;

	hash = FNV_OFFSET_BASIS;

	for( size_t i = 0; i < key_size; i++){
		hash = (FNV_PRIME * hash) ^ (key[i]);
	}

	return hash;
}

HashTableEntry* HashTable_new_entry(
		HashTable* table,uint8_t* key,size_t key_size){

	uint64_t hash = HashTable_hash(key,key_size);

	HashTableEntry* new_entry = malloc(sizeof(HashTableEntry));

	if(new_entry==NULL){
		return NULL;
	}

	new_entry->next = table->entries[hash%table->size];
	new_entry->hash = hash;
	new_entry->key  = malloc(key_size);
	new_entry->key_size = key_size;
	new_entry->value = NULL;

	if(new_entry->key == NULL){
		free(new_entry);
		return NULL;
	}

	memcpy(new_entry->key,key,key_size);

	table->entries[hash%table->size] = new_entry;

	return new_entry;
}


HashTableEntry* HashTable_access(
		HashTable* table,uint8_t* key,size_t key_size,uint32_t flag){

	uint64_t hash = HashTable_hash(key,key_size);

	HashTableEntry* head = table->entries[hash%table->size];

	while(head){
		if(head->hash == hash){
			if(head->key_size == key_size){
				if(!memcmp(head->key,key,key_size)){
					return head;
				}
			}
		}
		head = head->next;
	}

	if(flag&HASHTABLE_ACCESS_CREATE){
		return HashTable_new_entry(table,key,key_size);
	}else{
		return NULL;
	}
}

void* HashTable_get(
		HashTable* table, uint8_t* key,size_t key_size){
	HashTableEntry* entry = HashTable_access(table,key,key_size,0);

	if(entry == NULL){
		return NULL;
	}else{
		return entry->value;
	}
}

void* HashTable_set(
		HashTable* table, uint8_t* key, size_t key_size,void* value){
	HashTableEntry* entry = 
		HashTable_access(table,key,key_size,HASHTABLE_ACCESS_CREATE);

	void* old_value = entry->value;

	entry->value = value;

	return old_value;
}


#include<stdio.h>

//テスト用
//このファイルは単独で実行することもでき、
//実行した場合はこのモジュールのデモが実行される。
int __attribute__((__weak__)) 
	main(){

	HashTable* table = NULL;

	char cmd[32];

	while(1){
		printf("> ");
		scanf("%s",cmd);

		if(!strcmp(cmd,"help")){

			printf("help\n");
			printf("new %%d\n");
			printf("get %%s\n");
			printf("set %%s %%s\n");

		}else if(!strcmp(cmd,"new")){

			size_t size;
			scanf("%ld",&size);
			if(table!=NULL){
				printf("ERROR: HashTable has already been created.\n");
			}else{
				table = HashTable_new(size);
			}

		}else if(!strcmp(cmd,"quit")){

			if(table!=NULL){
				HashTable_del(table);
				table = NULL;
			}
			return 0;

		}else if(!strcmp(cmd,"get")){
			char  key[128];
			scanf("%s",key);

			if(table == NULL){
				printf("ERROR: No HashTable.\n");
				continue;
			}

			char* value = HashTable_get(table,key,strlen(key));
			printf("table[%s] = %s\n",key,value);

		}else if(!strcmp(cmd,"set")){

			char  key[128];
			char value[128];
			scanf("%s %s",key,value);

			if(table == NULL){
				printf("ERROR: No HashTable.\n");
				continue;
			}


			char* v = malloc(128);
			if(value == NULL){
				printf("FATAL: malloc failed.\n");
				continue;
			}
			strcpy(v,value);

			void* old_value = HashTable_set(table,key,strlen(key),v);
			if(old_value != NULL){
				free(old_value);
			}

		}
	}
	return 0;
}
