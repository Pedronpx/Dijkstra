#include "lista.h"
#include <stdlib.h>

// Implementação de Lista Ligada (Linked List)
// Baseado nos conceitos de Sedgewick: estrutura fundamental para representar
// coleções de dados com inserção/remoção em O(1) no início
typedef struct no {
    Elemento dado;  // Armazena o dado (polimórfico via void*)
    struct no* prox; // Referência para o próximo nó da lista
} No;

typedef struct {
    No* prim;  // Ponteiro para o primeiro nó (cabeça da lista)
    No* ult;   // Ponteiro para o último nó (cauda da lista) - otimiza inserção O(1)
    int tam;   // Tamanho atual - permite acesso em O(1) ao invés de percorrer
} ListaImpl;

typedef struct {
    No* atual;
} IteradorImpl;

Lista lista_cria() {
    // Aloca a estrutura da lista vazia
    // Padrão Sedgewick: encapsular a implementação com tipo opaco (void*)
    ListaImpl* l = malloc(sizeof(ListaImpl));
    l->prim = NULL;
    l->ult = NULL;
    l->tam = 0;
    return l;
}

void lista_insere(Lista lista, Elemento elemento) {
    // Inserção no final - O(1) operação fundamental em Sedgewick
    // Manter ponteiro para o último nó permite inserção constante
    ListaImpl* l = (ListaImpl*)lista;
    No* novo = malloc(sizeof(No));
    novo->dado = elemento;
    novo->prox = NULL;
    if (l->ult) l->ult->prox = novo;  // Encadeia ao final
    else l->prim = novo;               // Primeiro elemento
    l->ult = novo;                     // Atualiza cauda
    l->tam++;
}

Elemento lista_remove_primeiro(Lista lista) {
    // Remove o primeiro nó da lista - O(1)
    // Operação fundamental em deques segundo Sedgewick (algoritmos em C)
    ListaImpl* l = (ListaImpl*)lista;
    if (!l->prim) return NULL;
    No* rem = l->prim;
    Elemento dado = rem->dado;
    l->prim = rem->prox;
    if (!l->prim) l->ult = NULL;  // Lista ficou vazia
    free(rem);
    l->tam--;
    return dado;
}

Elemento lista_get_por_indice(Lista lista, int indice) {
    // Acesso por índice em O(n) - desvantagem de listas ligadas
    // vs arrays que oferecem acesso O(1) conforme discutido em Sedgewick
    ListaImpl* l = (ListaImpl*)lista;
    No* atual = l->prim;
    for(int i=0; i<indice && atual; i++) atual = atual->prox;
    return atual ? atual->dado : NULL;
}

int lista_tamanho(Lista l) { return ((ListaImpl*)l)->tam; }
bool lista_vazia(Lista lista) { return ((ListaImpl*)lista)->tam == 0; }

void lista_libera(Lista lista) {
    // Remove todos os elementos e libera a estrutura.
    while(lista_tamanho(lista) > 0) lista_remove_primeiro(lista);
    free(lista);
}

Iterador lista_iterador(Lista l) {
    // Padrão de Iterator para percorrer a lista
    // Conceito clássico em Sedgewick para abstrair acesso sequencial
    // evitando acoplamento com a implementação interna
    IteradorImpl* it = malloc(sizeof(IteradorImpl));
    it->atual = ((ListaImpl*)l)->prim;
    return it;
}
bool iterador_tem_proximo(Iterador it) { return ((IteradorImpl*)it)->atual != NULL; }
Elemento iterador_proximo(Iterador it) {
    IteradorImpl* i = (IteradorImpl*)it;
    if (!i->atual) return NULL;
    Elemento d = i->atual->dado;
    i->atual = i->atual->prox;
    return d;
}
void iterador_destroi(Iterador it) { free(it); }