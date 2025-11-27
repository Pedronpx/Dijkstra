# Makefile para Waze Clone (ED II + Raylib)

# --- CONFIGURAÇÃO DE CAMINHOS (CRÍTICO) ---
# O instalador padrao do Raylib coloca tudo em C:/raylib
# Se voce instalou em outro lugar, MUDE ESTA LINHA:
RAYLIB_PATH = C:/raylib/raylib

# Compilador
CC = gcc

# Flags de Compilação (Include)
# -I diz para o GCC: "Procure arquivos .h aqui tambem"
CFLAGS = -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -g -I. -I$(RAYLIB_PATH)/src

# Flags do Linker (Bibliotecas)
# -L diz para o GCC: "Procure arquivos .a (libs) aqui tambem"
LDFLAGS = -L$(RAYLIB_PATH)/src -lraylib -lopengl32 -lgdi32 -lwinmm

# --- ARQUIVOS DO PROJETO ---
SRCS = main.c \
       graph.c \
       via.c \
       geo.c \
       svg.c \
       qry.c \
       utils.c \
       priority_queue.c \
       hash.c \
       smutreap.c \
       lista.c \
       fila.c

# Gera lista de objetos (.o) automaticamente
OBJS = $(SRCS:.c=.o)

# Nome do executável final
TARGET = waze_app.exe

# --- REGRAS ---

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Ligando arquivos..."
	$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS)
	@echo "========================================"
	@echo " SUCESSO! Execute digitando: make run"
	@echo "========================================"

# Regra genérica para compilar cada .c em .o
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Limpar arquivos temporários
clean:
	del *.o $(TARGET)

# Atalho para rodar
run: $(TARGET)
	$(TARGET)