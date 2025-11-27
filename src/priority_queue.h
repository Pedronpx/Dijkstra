#ifndef PQ_H
#define PQ_H
#include <stdbool.h>

typedef void* priorityQueue;
priorityQueue createPriorityQueue(int cap);
// Insere item com prioridade `prio`.
void pq_insert(priorityQueue pq, int item, double prio);
// Extrai o item de menor prioridade (menor `prio`). Retorna -1 se vazio.
int pq_extract_min(priorityQueue pq);
bool pq_empty(priorityQueue pq);
void pq_destroy(priorityQueue pq);

#endif