#include "geo.h"
#include "utils.h" // Para duplicar_string se precisar
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Lista processaGeo(const char* caminho_geo) {
    FILE* f = fopen(caminho_geo, "r");
    if (!f) {
        printf("[GEO] Erro ao abrir %s\n", caminho_geo);
        return NULL;
    }
    
    Lista l = lista_cria();
    char linha[256];
    char cmd[10];
    
    while(fgets(linha, sizeof(linha), f)) {
        sscanf(linha, "%s", cmd);
        if (strcmp(cmd, "q") == 0) {
            Quadra* q = malloc(sizeof(Quadra));
            // Formato esperado: q cep x y w h
            sscanf(linha, "q %s %lf %lf %lf %lf", q->cep, &q->x, &q->y, &q->w, &q->h);
            // Cores padrao
            strcpy(q->cfill, "lightgray");
            strcpy(q->cstrk, "black");
            // Insere a quadra na lista (passeio / subgrafo visual)
            lista_insere(l, q);
        }
    }
    
    fclose(f);
    printf("[GEO] Carregou quadras de %s\n", caminho_geo);
    return l;
}