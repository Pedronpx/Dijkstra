#include "hash.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct HashNode {
    char* key;
    int value;
    struct HashNode* next;
} HashNode;

typedef struct {
    HashNode** buckets;
    int size;
} HashImpl;

// Função de Hash simples (DJB2)
unsigned long hashFunction(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

hashTable createHashTable(int size) {
    HashImpl* ht = malloc(sizeof(HashImpl));
    ht->buckets = calloc(size, sizeof(HashNode*));
    ht->size = size;
    return ht;
}

void hashPut(hashTable ht, const char* key, int value) {
    HashImpl* h = (HashImpl*)ht;
    unsigned long idx = hashFunction(key) % h->size;
    
    HashNode* node = h->buckets[idx];
    while(node) {
        if (strcmp(node->key, key) == 0) {
            node->value = value; // Atualiza
            return;
        }
        node = node->next;
    }
    
    // Novo nó
    HashNode* novo = malloc(sizeof(HashNode));
    novo->key = duplicar_string(key);
    novo->value = value;
    novo->next = h->buckets[idx];
    h->buckets[idx] = novo;
}

bool hashGet(hashTable ht, const char* key, int* value) {
    HashImpl* h = (HashImpl*)ht;
    unsigned long idx = hashFunction(key) % h->size;
    
    HashNode* node = h->buckets[idx];
    while(node) {
        if (strcmp(node->key, key) == 0) {
            if (value) *value = node->value;
            return true;
        }
        node = node->next;
    }
    return false;
}

void hashTableDestroy(hashTable ht) {
    HashImpl* h = (HashImpl*)ht;
    for(int i=0; i<h->size; i++) {
        HashNode* node = h->buckets[i];
        while(node) {
            HashNode* temp = node;
            node = node->next;
            // Libera chave duplicada e nó
            free(temp->key);
            free(temp);
        }
    }
    free(h->buckets);
    free(h);
}