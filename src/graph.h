#ifndef GRAPH_H
#define GRAPH_H
#include "lista.h"
#include <stdbool.h>

typedef void* Graph;
typedef int Node;
typedef void* Info;
typedef void* Edge;

#define CRITERIO_DISTANCIA 0
#define CRITERIO_TEMPO 1

typedef double (*CalculaCustoAresta)(Info info, int criterio);

Graph createGraph(int n, bool dir, char* nome);
// Cria um grafo com capacidade para `n` nós.
// `dir` indica se o grafo é dirigido (true) ou não.
Node addNode(Graph g, char* nome, Info info);
// Adiciona um nó ao grafo e retorna seu índice.
Edge addEdge(Graph g, Node u, Node v, Info info);
// Adiciona uma aresta do nó `u` para `v` associando `info`.
Node getNode(Graph g, char* nome);
// Retorna o índice do nó com nome dado, ou -1 se não existir.
Info getNodeInfo(Graph g, Node n);
// Retorna o ponteiro `Info` armazenado no nó `n`.
int getTotalNodes(Graph g);

// --- CORREÇÃO AQUI ---
// Preenche `l` com as estruturas de aresta adjacentes ao nó `n`.
void adjacentEdges(Graph g, Node n, Lista l);
// Preenche `l` com os nós adjacentes (IDs) de `n`.
void adjacentNodes(Graph g, Node n, Lista l);
// ---------------------

Node getToNode(Graph g, Edge e);
Info getEdgeInfo(Graph g, Edge e);

Node findNearestNode(Graph g, double x, double y);
// Encontra um caminho mínimo entre `start` e `end` segundo `crit`.
// Implementa Dijkstra usando a função de custo `f`.
Lista findPath(Graph g, Node start, Node end, int crit, CalculaCustoAresta f);

#endif