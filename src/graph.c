#include "graph.h"
#include "priority_queue.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <stdint.h> // <--- Include para função intptr_t

// Struct de implementação do grafo
typedef struct { double x, y; } Coord; 
typedef struct { Node dest; Info info; } EdgeImpl;
typedef struct { char* nome; Info info; Lista edges; } NodeImpl;
typedef struct { NodeImpl* nodes; int max; int count; } GraphImpl;

Graph createGraph(int n, bool dir, char* nome) {
    GraphImpl* g = calloc(1, sizeof(GraphImpl));
    g->nodes = calloc(n, sizeof(NodeImpl));
    g->max = n;
    return g;
}

Node addNode(Graph g, char* nome, Info info) {
    GraphImpl* G = (GraphImpl*)g;
    if (G->count >= G->max) return -1;
    int id = G->count++;
    G->nodes[id].nome = duplicar_string(nome);
    G->nodes[id].info = info;
    G->nodes[id].edges = lista_cria();
    return id;
}

Edge addEdge(Graph g, Node u, Node v, Info info) {
    GraphImpl* G = (GraphImpl*)g;
    EdgeImpl* e = malloc(sizeof(EdgeImpl));
    e->dest = v;
    e->info = info;
    lista_insere(G->nodes[u].edges, e);
    return e;
}

int getTotalNodes(Graph g) { return ((GraphImpl*)g)->count; }
Info getNodeInfo(Graph g, Node n) { return ((GraphImpl*)g)->nodes[n].info; }

Node getNode(Graph g, char* nome) {
    GraphImpl* G = (GraphImpl*)g;
    for(int i=0; i<G->count; i++) 
        if(strcmp(G->nodes[i].nome, nome)==0) return i;
    return -1;
}

void adjacentEdges(Graph g, Node n, Lista l) {
    GraphImpl* G = (GraphImpl*)g;
    Lista edges = G->nodes[n].edges;
    Iterador it = lista_iterador(edges);
    while(iterador_tem_proximo(it)) 
        lista_insere(l, iterador_proximo(it));
    iterador_destroi(it);
}

// --- 
void adjacentNodes(Graph g, Node n, Lista l) {
    GraphImpl* G = (GraphImpl*)g;
    Lista edges = G->nodes[n].edges;
    Iterador it = lista_iterador(edges);
    while(iterador_tem_proximo(it)) {
        EdgeImpl* e = (EdgeImpl*)iterador_proximo(it);
        // Insere o índice do nó destino na lista.
        // Usa cast através de `intptr_t` para armazenar inteiros em `void*`.
        lista_insere(l, (void*)(intptr_t)e->dest);
    }
    iterador_destroi(it);
}
// ---------------------------------------------

Node getToNode(Graph g, Edge e) { return ((EdgeImpl*)e)->dest; }
Info getEdgeInfo(Graph g, Edge e) { return ((EdgeImpl*)e)->info; }

Node findNearestNode(Graph g, double x, double y) {
    GraphImpl* G = (GraphImpl*)g;
    int best = -1;
    double minD = DBL_MAX;
    
    for(int i=0; i<G->count; i++) {
        Coord* c = (Coord*)G->nodes[i].info;
        double d = (c->x - x)*(c->x - x) + (c->y - y)*(c->y - y);
        if (d < minD) { minD = d; best = i; }
    }
    return best;
}

// ============================================================================
// ALGORITMO DE DIJKSTRA - Caminho mais curto em grafos com pesos positivos
// ============================================================================
// Entrada: grafo g, nó de início (start), nó de fim (end),
//          critério de custo (crit), função para calcular peso da aresta (f)
// Saída: lista com nós do caminho mais curto (ou vazia se sem caminho)
// Complexidade: O((V + E) log V) onde V=vértices, E=arestas
// Baseado em Sedgewick - Algoritmos em C, seção 21 (Shortest Paths)
// ============================================================================
Lista findPath(Graph g, Node start, Node end, int crit, CalculaCustoAresta f) {
    GraphImpl* G = (GraphImpl*)g;
    int n = G->count;
    
    // Pré-processamento: inicialização de distâncias e predecessores
    double* dist = malloc(n * sizeof(double));  // dist[v] = menor distância de start até v
    int* pai = malloc(n * sizeof(int));         // pai[v] = nó anterior no caminho ótimo
    for(int i=0; i<n; i++) { 
        dist[i] = DBL_MAX;  // Infinito: vértice ainda não alcançável
        pai[i] = -1;        // Sem predecessor
    }
    
    // Cria fila de prioridade para selecionar vértice com menor distância
    // Essencial para eficiência: sem PQ seria O(V²)
    priorityQueue pq = createPriorityQueue(n);
    dist[start] = 0;        // Distância ao próprio start é zero
    pq_insert(pq, start, 0); // Insere start na PQ com prioridade 0
    
    // ===== FASE 1: RELAXAÇÃO DE ARESTAS (Core do Dijkstra) =====
    // Invariante: dist[] mantém a menor distância conhecida de start até cada vértice
    // Repetidamente processamos o vértice não visitado com menor dist[]
    while(!pq_empty(pq)) {
        // Extrai vértice com menor distância (greedy choice - Dijkstra's key insight)
        int u = pq_extract_min(pq);
        
        // Otimizações de parada
        if (u == end) break;           // Encontramos o destino (early termination)
        if (dist[u] == DBL_MAX) break; // Resto do grafo desconexo, impossível chegar
        
        // Relaxação de arestas: para cada vizinho v de u
        // Relaxar = tentar melhorar o caminho mais curto até v passando por u
        Lista adjs = G->nodes[u].edges;  // Obtém arestas saindo de u
        Iterador it = lista_iterador(adjs);
        
        while(iterador_tem_proximo(it)) {
            EdgeImpl* e = (EdgeImpl*)iterador_proximo(it);
            int v = e->dest;
            
            // Calcula o peso (distância/tempo/custo) da aresta u->v usando critério
            double peso = f(e->info, crit);
            
            // RELAXAÇÃO: se encontramos caminho mais curto até v via u, atualiza
            // Condição: dist[u] + peso < dist[v]
            if (dist[u] + peso < dist[v]) {
                dist[v] = dist[u] + peso;  // Nova melhor distância
                pai[v] = u;                 // Registra que v vem de u no caminho ótimo
                pq_insert(pq, v, dist[v]); // Re-insere v na PQ com nova prioridade
            }
        }
        iterador_destroi(it);
    }
    
    // ===== FASE 2: RECONSTRUÇÃO DO CAMINHO (Backtracking) =====
    // O array pai[] contém os predecessores, usamos para rastrear de end até start
    Lista path = lista_cria();
    
    if (dist[end] != DBL_MAX) {  // Verifica se destino é alcançável
        // Percorre backwards: end -> start seguindo o array pai[]
        int curr = end;
        int* temp = malloc(n * sizeof(int));  // Array temporário para reverter ordem
        int count = 0;
        
        // Coleta nós do caminho em ordem reversa (end -> start)
        while(curr != -1) { 
            temp[count++] = curr; 
            curr = pai[curr];  // Salta para o predecessor
        }
        
        // Insere na lista em ordem correta: start -> end
        for(int i=count-1; i>=0; i--) 
            lista_insere(path, (void*)(intptr_t)temp[i]);
        
        free(temp);
    }
    // Se dist[end] == DBL_MAX, nenhum caminho existe, path fica vazia
    
    // Libera estruturas auxiliares
    free(dist); 
    free(pai); 
    pq_destroy(pq);
    return path;
}