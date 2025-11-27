#ifndef VIA_H
#define VIA_H
#include "graph.h"

Graph carregarGrafoDeArquivoVia(const char* path);
// Função de cálculo de custo usada pelo Dijkstra (distância/tempo)
double calculaCustoAresta(Info info, int crit);

#endif