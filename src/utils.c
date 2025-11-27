#include "utils.h"
#include <string.h>
#include <stdlib.h>

char* duplicar_string(const char* s) {
    // Retorna uma cópia alocada dinamicamente da string `s`.
    if(!s) return NULL;
    char* d = malloc(strlen(s)+1);
    strcpy(d, s);
    return d;
}