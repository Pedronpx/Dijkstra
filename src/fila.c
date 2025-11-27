#include "fila.h"
#include <stdlib.h>

typedef struct NoFila {
    Elemento dado;
    struct NoFila* prox;
} NoFila;

typedef struct {
    NoFila* inicio;
    NoFila* fim;
} FilaImpl;

Fila fila_cria() {
    FilaImpl* f = calloc(1, sizeof(FilaImpl));
    return f;
}

void fila_insere(Fila f, Elemento e) {
    FilaImpl* fila = (FilaImpl*)f;
    NoFila* novo = malloc(sizeof(NoFila));
    novo->dado = e;
    novo->prox = NULL;
    
    if (fila->fim) fila->fim->prox = novo;
    else fila->inicio = novo;
    fila->fim = novo;
}

Elemento fila_remove(Fila f) {
    FilaImpl* fila = (FilaImpl*)f;
    if (!fila->inicio) return NULL;
    
    NoFila* temp = fila->inicio;
    Elemento dado = temp->dado;
    fila->inicio = temp->prox;
    if (!fila->inicio) fila->fim = NULL;
    
    free(temp);
    return dado;
}

bool fila_vazia(Fila f) {
    return ((FilaImpl*)f)->inicio == NULL;
}

void fila_libera(Fila f) {
    // Esvazia a fila liberando elementos e então libera a estrutura
    while(!fila_vazia(f)) fila_remove(f);
    free(f);
}