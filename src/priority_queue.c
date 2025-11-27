// priority_queue.c
#include "priority_queue.h"
#include <stdlib.h>

// Implementação via Binary Heap (Min-Heap)
typedef struct { int id; double p; } HeapNode;
typedef struct { HeapNode* data; int size; int cap; } PQImpl;

priorityQueue createPriorityQueue(int cap) {
    PQImpl* pq = malloc(sizeof(PQImpl));
    pq->data = malloc((cap+1) * sizeof(HeapNode));
    pq->size = 0;
    pq->cap = cap;
    return pq;
}

// Insere mantendo a propriedade do Heap (Sobe o elemento)
void pq_insert(priorityQueue pq, int item, double prio) {
    PQImpl* p = (PQImpl*)pq;
    int i = ++p->size;
    while (i > 1 && p->data[i/2].p > prio) {
        p->data[i] = p->data[i/2];
        i /= 2;
    }
    p->data[i].id = item;
    p->data[i].p = prio;
}

// Remove o menor elemento (raiz) e reorganiza (Desce o elemento)
int pq_extract_min(priorityQueue pq) {
    PQImpl* p = (PQImpl*)pq;
    if (p->size == 0) return -1;
    int minItem = p->data[1].id;
    HeapNode last = p->data[p->size--];
    int i = 1, child;
    while (2*i <= p->size) {
        child = 2*i;
        if (child != p->size && p->data[child+1].p < p->data[child].p) child++;
        if (last.p > p->data[child].p) p->data[i] = p->data[child];
        else break;
        i = child;
    }
    p->data[i] = last;
    return minItem;
}

bool pq_empty(priorityQueue pq) { return ((PQImpl*)pq)->size == 0; }
void pq_destroy(priorityQueue pq) { free(((PQImpl*)pq)->data); free(pq); }