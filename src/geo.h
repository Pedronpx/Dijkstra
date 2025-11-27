#ifndef GEO_H
#define GEO_H
#include "lista.h"

// Struct compatível com o cast feito no main.c
typedef struct {
    char cep[50];
    double x, y, w, h;
    char cfill[25], cstrk[25], sw[8];
} Quadra;

Lista processaGeo(const char* caminho_geo);

#endif