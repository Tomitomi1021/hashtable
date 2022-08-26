#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

struct HashTable;
typedef struct HashTable HashTable;

HashTable* HashTable_new(uint32_t size);

void HashTable_del(HashTable* table);

void* HashTable_get(
		HashTable* table, uint8_t* key,size_t key_size);

void* HashTable_set(
		HashTable* table, uint8_t* key, size_t key_size,void* value);

#endif
