#ifndef HASH_H
#define HASH_H
#include <stdbool.h>

typedef void* hashTable;

hashTable createHashTable(int size);
// Insere/parâmetro: associa `key` -> `value` na tabela
void hashPut(hashTable ht, const char* key, int value);
// Recupera o valor associado à `key`, retorna true se encontrado
bool hashGet(hashTable ht, const char* key, int* value);
void hashTableDestroy(hashTable ht);

#endif