#ifndef LISTA_H
#define LISTA_H
#include <stdbool.h>

typedef void* Elemento;
typedef void* Lista;
typedef void* Iterador;

Lista lista_cria();
// Lista encadeada simples: operações básicas
void lista_insere(Lista lista, Elemento elemento);
void lista_libera(Lista lista);
int lista_tamanho(Lista l);
Elemento lista_get_por_indice(Lista lista, int indice);
Elemento lista_remove_primeiro(Lista lista);
bool lista_vazia(Lista lista);

Iterador lista_iterador(Lista l);
bool iterador_tem_proximo(Iterador it);
Elemento iterador_proximo(Iterador it);
void iterador_destroi(Iterador it);

#endif