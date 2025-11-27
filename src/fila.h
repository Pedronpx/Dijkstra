#ifndef FILA_H
#define FILA_H
#include <stdbool.h>

typedef void* Fila;
typedef void* Elemento;

Fila fila_cria();
// Insere no fim da fila
void fila_insere(Fila f, Elemento e);
// Remove e retorna o primeiro elemento (FIFO)
Elemento fila_remove(Fila f);
bool fila_vazia(Fila f);
void fila_libera(Fila f);

#endif