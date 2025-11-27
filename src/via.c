#include "via.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { double x, y; } Coord;
typedef struct { char *n, *cd, *ce; double len, vel; } InfoVia;

double calculaCustoAresta(Info info, int crit) {
    InfoVia* i = (InfoVia*)info;
    if (crit == 0) return i->len; // Distancia
    return (i->vel > 0) ? i->len / i->vel : 999999; // Tempo
}

Graph carregarGrafoDeArquivoVia(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return NULL;
    
    int n;
    fscanf(f, "%d", &n);
    Graph g = createGraph(n, true, "Londrina");
    
    char type[10];
    while(fscanf(f, "%s", type) != EOF) {
        if (strcmp(type, "v") == 0) {
            char nome[50]; double x, y;
            fscanf(f, "%s %lf %lf", nome, &x, &y);
            Coord* c = malloc(sizeof(Coord)); c->x=x; c->y=y;
            addNode(g, nome, c);
        } else if (strcmp(type, "e") == 0) {
            char u[50], v[50], l[50], r[50], rua[100]; double len, vel;
            fscanf(f, "%s %s %s %s %lf %lf", u, v, l, r, &len, &vel);
            fgets(rua, 100, f); // Ler resto da linha (nome rua)
            
            int idU = getNode(g, u);
            int idV = getNode(g, v);
            if (idU != -1 && idV != -1) {
                InfoVia* iv = malloc(sizeof(InfoVia));
                iv->len = len; iv->vel = vel; iv->n = duplicar_string(rua);
                addEdge(g, idU, idV, iv);
            }
        }
    }
    fclose(f);
    return g;
}