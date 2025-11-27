// ============================================================
// INCLUDES
// ============================================================
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdint.h>
#include "graph.h"
#include "via.h"
#include "lista.h"
#include "utils.h"

// ============================================================
// DEFINIÇÕES GLOBAIS DE TELA E CORES
// ============================================================
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define COR_FUNDO (Color){20, 24, 35, 255}

// Limites do Mapa (Prevenção de sair da área visível)
#define MAP_MIN_X -200
#define MAP_MAX_X 1600
#define MAP_MIN_Y -200
#define MAP_MAX_Y 1200

// ============================================================
// ESTRUTURAS DE DADOS
// ============================================================

// Tipos de eventos que podem ocorrer em uma rua
typedef enum {
    EV_NENHUM = 0,
    EV_ACIDENTE,      // Bloqueio total (Dijkstra evita)
    EV_OBRA,          // Bloqueio total
    EV_INTERDITADO,   // Bloqueio total
    EV_TRAFEGO,       // Bloqueio total
    EV_RADAR,         // Apenas visual (afeta velocidade animação)
    EV_NO_RADAR       // Apenas visual (afeta velocidade animação)
} TipoEvento;

// Estrutura para segurar as texturas carregadas
typedef struct {
    Texture2D car_icon;
    Texture2D flag_icon;
    Texture2D seta_local;
    Texture2D trafic_icon;
    Texture2D interditado_icon;
    Texture2D radar_padrao;
    Texture2D obra_icon;
    Texture2D acidente_icon;
    Texture2D no_radar;
} Assets;

// Registro de onde os eventos estão no grafo
typedef struct {
    int u, v;
    TipoEvento tipo;
} EventoVia;

#define MAX_EVENTOS 300
EventoVia eventos[MAX_EVENTOS];
int numEventos = 0;

// Estado Global da Aplicação (Contexto)
typedef struct {
    Graph g;            // O Grafo da cidade
    Lista rota;         // A lista de nós do caminho atual
    bool navegando;     // Se o carro está em movimento
    bool chegou;        // Se chegou ao destino
    int indiceRota;     // Em qual nó da rota o carro está agora
    Vector2 posCarro;   // Posição visual exata (x,y)
    float anguloCarro;  // Rotação do sprite
    Node origem;        // Nó de partida
    Node destino;       // Nó de chegada
    Camera2D cam;       // Câmera do Raylib
    float distanciaKm;  // Distância calculada da rota
} AppState;

// Struct interna das arestas (InfoV) para manipular pesos
typedef struct { 
    char *n; char *cd; char *ce; 
    double len; 
    double vel; 
} InfoV;

// ============================================================
// FUNÇÕES AUXILIARES E LÓGICA
// ============================================================

// Recupera a posição (x,y) de um nó do grafo de forma segura
Vector2 GetNodePos(Graph g, Node n) {
    typedef struct { double x; double y; } C;
    C* c = (C*)getNodeInfo(g, n);
    if (!c) return (Vector2){0,0};
    return (Vector2){(float)c->x, (float)c->y};
}

// Busca a informação interna da aresta entre u e v
InfoV* GetInfoAresta(Graph g, int u, int v) {
    Lista viz = lista_cria();
    adjacentEdges(g, u, viz);
    InfoV* targetInfo = NULL;
    while(!lista_vazia(viz)) {
        Edge e = lista_remove_primeiro(viz);
        if (getToNode(g, e) == v) targetInfo = (InfoV*)getEdgeInfo(g, e);
    }
    lista_libera(viz);
    return targetInfo;
}

// Adiciona um evento ao mapa e altera o peso da aresta no grafo
void AdicionarEvento(Graph g, int u, int v, TipoEvento t) {
    if (numEventos < MAX_EVENTOS-2) {
        eventos[numEventos++] = (EventoVia){u, v, t};
        eventos[numEventos++] = (EventoVia){v, u, t};
        
        InfoV* infoIda = GetInfoAresta(g, u, v);
        InfoV* infoVolta = GetInfoAresta(g, v, u);
        
        if (infoIda && infoVolta) {
            switch (t) {
                case EV_ACIDENTE: case EV_OBRA: case EV_INTERDITADO: case EV_TRAFEGO:
                    infoIda->vel = 0.1; infoVolta->vel = 0.1; break;
                default: break; 
            }
        }
    }
}

// Retorna qual evento existe entre dois nós (para desenhar o ícone)
TipoEvento GetEvento(int u, int v) {
    for(int i=0; i<numEventos; i++) {
        if (eventos[i].u == u && eventos[i].v == v) return eventos[i].tipo;
    }
    return EV_NENHUM;
}

// Função de Custo usada pelo Dijkstra (Critério TEMPO)
double CustoInteligente(Info info, int crit) {
    InfoV* i = (InfoV*)info;
    return (i->len / i->vel);
}

// Carrega imagem com filtro bilinear (suaviza pixelização) - melhorar qualidade visual
Texture2D CarregarSafe(const char* path) {
    Texture2D t = LoadTexture(path);
    if (t.id == 0) printf("[ERRO] Falha ao carregar: %s\n", path);
    else {
        GenTextureMipmaps(&t);
        SetTextureFilter(t, TEXTURE_FILTER_BILINEAR);
    }
    return t;
}

// Calcula a distância total da rota em KM
float CalcularDistanciaRota(Graph g, Lista rota) {
    if (!rota || lista_tamanho(rota) < 2) return 0.0f;
    float distTotal = 0;
    int tam = lista_tamanho(rota);
    
    for (int i = 0; i < tam - 1; i++) {
        Node u = (Node)(intptr_t)lista_get_por_indice(rota, i);
        Node v = (Node)(intptr_t)lista_get_por_indice(rota, i+1);
        InfoV* info = GetInfoAresta(g, u, v);
        if (info) distTotal += info->len;
    }
    // Converte pixels para km (escala arbitrária: 100px = 1km)
    return distTotal / 100.0f;
}

// Penaliza a rota atual para forçar o Dijkstra a achar outro caminho
void PenalizarRotaAtual(Graph g, Lista rotaAtual) {
    if (!rotaAtual) return;
    int tam = lista_tamanho(rotaAtual);
    for (int i = 0; i < tam - 1; i++) {
        Node u = (Node)(intptr_t)lista_get_por_indice(rotaAtual, i);
        Node v = (Node)(intptr_t)lista_get_por_indice(rotaAtual, i+1);
        InfoV* info = GetInfoAresta(g, u, v);
        if (info && info->vel > 1.0) info->vel *= 0.5;
    }
}

// ============================================================
// GERAÇÃO PROCEDURAL DA CIDADE
// ============================================================
void CriarCidade(Graph g, AppState* app) {
    int cols = 10; int rows = 8; float spacing = 100.0f;
    int nodeCount = 0;
    
    // 1. Grade Central
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            typedef struct { double x; double y; } C; C* c = malloc(sizeof(C));
            c->x = (x * spacing) + 200 + GetRandomValue(-5, 5); 
            c->y = (y * spacing) + 150 + GetRandomValue(-5, 5);
            char nome[20]; sprintf(nome, "N%d", nodeCount++); 
            addNode(g, nome, c);
        }
    }
    // 2. Conexões Internas
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            int u = y * cols + x;
            if (x < cols - 1) { 
                int v = u + 1; 
                InfoV* i = malloc(sizeof(InfoV)); *i = (InfoV){"Rua",0,0, spacing, 60}; 
                addEdge(g, u, v, i); addEdge(g, v, u, i); 
            }
            if (y < rows - 1) { 
                int v = u + cols; 
                InfoV* i = malloc(sizeof(InfoV)); *i = (InfoV){"Av",0,0, spacing, 60}; 
                addEdge(g, u, v, i); addEdge(g, v, u, i); 
            }
        }
    }

    // 3. Rodovias Externas (Conexões Rápidas)
    typedef struct { double x; double y; } C; 
    InfoV* iRod = malloc(sizeof(InfoV)); *iRod = (InfoV){"Rodovia",0,0, 400, 110};

    int norte = addNode(g, "Norte", (C*)malloc(sizeof(C))); ((C*)getNodeInfo(g, norte))->x = 650; ((C*)getNodeInfo(g, norte))->y = -100;
    addEdge(g, norte, 4, iRod); addEdge(g, 4, norte, iRod);

    int sul = addNode(g, "Sul", (C*)malloc(sizeof(C))); ((C*)getNodeInfo(g, sul))->x = 650; ((C*)getNodeInfo(g, sul))->y = 1000;
    addEdge(g, sul, 74, iRod); addEdge(g, 74, sul, iRod);

    int oeste = addNode(g, "Oeste", (C*)malloc(sizeof(C))); ((C*)getNodeInfo(g, oeste))->x = -150; ((C*)getNodeInfo(g, oeste))->y = 400;
    addEdge(g, oeste, 30, iRod); addEdge(g, 30, oeste, iRod);

    int leste = addNode(g, "Leste", (C*)malloc(sizeof(C))); ((C*)getNodeInfo(g, leste))->x = 1450; ((C*)getNodeInfo(g, leste))->y = 400;
    addEdge(g, leste, 39, iRod); addEdge(g, 39, leste, iRod);

    // 4. Inserção de Eventos (Desafios para o Dijkstra)
    AdicionarEvento(g, 12, 13, EV_ACIDENTE);
    AdicionarEvento(g, 66, 67, EV_OBRA);
    AdicionarEvento(g, 18, 28, EV_INTERDITADO);
    AdicionarEvento(g, 44, 45, EV_RADAR);
    AdicionarEvento(g, 70, 71, EV_RADAR);
    AdicionarEvento(g, 30, 31, EV_RADAR);
    AdicionarEvento(g, 4, 14, EV_NO_RADAR);

    app->origem = oeste;
}

// ============================================================
// FUNÇÕES DE RENDERIZAÇÃO
// ============================================================

void DrawMap(AppState* app, Assets* ass) {
    int total = getTotalNodes(app->g);
    Lista viz = lista_cria();
    
    for (int i = 0; i < total; i++) {
        Vector2 p1 = GetNodePos(app->g, i);
        adjacentNodes(app->g, i, viz);
        while (!lista_vazia(viz)) {
            Node v = (Node)(intptr_t)lista_remove_primeiro(viz);
            if (i < v) {
                Vector2 p2 = GetNodePos(app->g, v);
                float len = Vector2Distance(p1, p2);
                
                // Construção das Vias
                Color cor = (len > 150) ? ORANGE : (Color){60, 70, 80, 255};
                float w = (len > 150) ? 8.0f : 5.0f;
                
                DrawLineEx(p1, p2, w, cor);
                if (len > 150) DrawLineEx(p1, p2, 2.0f, YELLOW); 

                // Ícones 
                TipoEvento ev = GetEvento(i, v);
                Texture2D icon = {0};
                switch(ev) {
                    case EV_ACIDENTE: icon = ass->acidente_icon; break;
                    case EV_OBRA: icon = ass->obra_icon; break;
                    case EV_INTERDITADO: icon = ass->interditado_icon; break;
                    case EV_RADAR: icon = ass->radar_padrao; break;
                    case EV_TRAFEGO: icon = ass->trafic_icon; break;
                    case EV_NO_RADAR: icon = ass->no_radar; break;
                    default: break;
                }
                if (icon.id > 0) {
                    Vector2 mid = Vector2Scale(Vector2Add(p1, p2), 0.5f);
                    DrawTexturePro(icon, (Rectangle){0,0,icon.width,icon.height},
                        (Rectangle){mid.x, mid.y, 50, 50}, (Vector2){25,25}, 0, WHITE);
                }
            }
        }
    }
    lista_libera(viz);
}

void DrawRoute(Graph g, Lista rota) {
    if (!rota || lista_tamanho(rota) < 2) return;
    int tam = lista_tamanho(rota);
    for (int i = 0; i < tam - 1; i++) {
        Node u = (Node)(intptr_t)lista_get_por_indice(rota, i);
        Node v = (Node)(intptr_t)lista_get_por_indice(rota, i+1);
        Vector2 p1 = GetNodePos(g, u);
        Vector2 p2 = GetNodePos(g, v);
        // Rota Azul Neon
        DrawLineEx(p1, p2, 8.0f, (Color){0, 150, 255, 150});
        DrawLineEx(p1, p2, 4.0f, (Color){150, 220, 255, 255});
    }
}

// ============================================================
// MAIN FUNCTION
// ============================================================
int main(int argc, char* argv[]) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wazir - GPS Dijkstra");
    SetTargetFPS(60);
    
    // 1. Assets
    Assets assets = {0};
    assets.car_icon = CarregarSafe("img/car_icon.png");
    assets.flag_icon = CarregarSafe("img/flag_icon.jpg");
    assets.seta_local = CarregarSafe("img/seta_local.png");
    assets.trafic_icon = CarregarSafe("img/trafic_icon.png");
    assets.interditado_icon = CarregarSafe("img/interditado_icon.png");
    assets.radar_padrao = CarregarSafe("img/radar_padrao.png");
    assets.obra_icon = CarregarSafe("img/obra_icon.png");
    assets.acidente_icon = CarregarSafe("img/acidente_icon.png");
    assets.no_radar = CarregarSafe("img/no_radar.png");

    // 2. Inicializar Grafo
    AppState app = {0};
    app.g = createGraph(300, true, "Cidade");
    CriarCidade(app.g, &app);

    // Configurações Iniciais
    app.destino = getTotalNodes(app.g) - 5;
    app.posCarro = GetNodePos(app.g, app.origem);
    app.chegou = false;
    
    app.cam.zoom = 0.6f;
    app.cam.offset = (Vector2){SCREEN_WIDTH/2, SCREEN_HEIGHT/2};
    app.cam.target = (Vector2){700, 500};
    
    // Rota inicial
    app.rota = findPath(app.g, app.origem, app.destino, CRITERIO_TEMPO, CustoInteligente);
    app.distanciaKm = CalcularDistanciaRota(app.g, app.rota);

    // Botões 
    Rectangle btnGo = {SCREEN_WIDTH - 150, SCREEN_HEIGHT - 80, 120, 50};
    Rectangle btnNovo = {SCREEN_WIDTH - 150, SCREEN_HEIGHT - 140, 120, 50};
    Rectangle btnAlt = {SCREEN_WIDTH - 150, SCREEN_HEIGHT - 200, 120, 50}; // Botão Alternativa

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        // --- CONTROLE DE CÂMERA LIMITADO ---
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            app.cam.zoom += wheel * 0.1f;
            if (app.cam.zoom < 0.4f) app.cam.zoom = 0.4f; 
            if (app.cam.zoom > 2.0f) app.cam.zoom = 2.0f; 
        }
        
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 d = GetMouseDelta();
            d = Vector2Scale(d, -1.0f/app.cam.zoom);
            
            // Nova posição alvo com Limites
            Vector2 newTarget = Vector2Add(app.cam.target, d);
            if (newTarget.x < MAP_MIN_X) newTarget.x = MAP_MIN_X;
            if (newTarget.x > MAP_MAX_X) newTarget.x = MAP_MAX_X;
            if (newTarget.y < MAP_MIN_Y) newTarget.y = MAP_MIN_Y;
            if (newTarget.y > MAP_MAX_Y) newTarget.y = MAP_MAX_Y;
            
            app.cam.target = newTarget;
        }
        
        // --- DEFINIR DESTINO ---
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            // Verifica se não clicou em cima de um botão
            Vector2 mPos = GetMousePosition();
            if (!CheckCollisionPointRec(mPos, btnGo) && !CheckCollisionPointRec(mPos, btnNovo) && !CheckCollisionPointRec(mPos, btnAlt)) {
                if (!app.navegando && !app.chegou) {
                    Vector2 mWorld = GetScreenToWorld2D(mPos, app.cam);
                    Node n = findNearestNode(app.g, mWorld.x, mWorld.y);
                    if (n != -1) {
                        app.destino = n;
                        if (app.rota) lista_libera(app.rota);
                        app.rota = findPath(app.g, app.origem, app.destino, CRITERIO_TEMPO, CustoInteligente);
                        app.distanciaKm = CalcularDistanciaRota(app.g, app.rota);
                    }
                }
            }
        }

        // --- AÇÃO DOS BOTÕES ---
        if (CheckCollisionPointRec(GetMousePosition(), btnGo) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (!app.navegando && !app.chegou && app.rota && lista_tamanho(app.rota) > 1) {
                app.navegando = true; app.indiceRota = 0; 
                Node start = (Node)(intptr_t)lista_get_por_indice(app.rota, 0);
                app.posCarro = GetNodePos(app.g, start);
            }
        }
        
        if (CheckCollisionPointRec(GetMousePosition(), btnNovo) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            app.navegando = false; app.chegou = false;
            app.origem = app.destino; 
            app.posCarro = GetNodePos(app.g, app.origem);
            if (app.rota) { lista_libera(app.rota); app.rota = NULL; }
            app.distanciaKm = 0;
        }
        
        if (CheckCollisionPointRec(GetMousePosition(), btnAlt) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (!app.navegando && !app.chegou && app.rota) {
                PenalizarRotaAtual(app.g, app.rota);
                lista_libera(app.rota);
                app.rota = findPath(app.g, app.origem, app.destino, CRITERIO_TEMPO, CustoInteligente);
                app.distanciaKm = CalcularDistanciaRota(app.g, app.rota);
            }
        }

        // --- LÓGICA DE MOVIMENTO E ANIMAÇÃO ---
        if (app.navegando && app.rota && app.indiceRota < lista_tamanho(app.rota)-1) {
            Node curr = (Node)(intptr_t)lista_get_por_indice(app.rota, app.indiceRota);
            Node next = (Node)(intptr_t)lista_get_por_indice(app.rota, app.indiceRota+1);
            Vector2 target = GetNodePos(app.g, next);
            Vector2 dir = Vector2Subtract(target, app.posCarro);
            float dist = Vector2Length(dir);
            
            if (dist < 5.0f) {
                app.posCarro = target; app.indiceRota++; app.origem = next;
                if (app.indiceRota >= lista_tamanho(app.rota)-1) { app.navegando = false; app.chegou = true; }
            } else {
                dir = Vector2Normalize(dir);
                float speed = 300.0f; 
                TipoEvento ev = GetEvento(curr, next);
                if (ev == EV_RADAR) speed = 80.0f; 
                if (ev == EV_NO_RADAR) speed = 550.0f; 
                
                app.posCarro = Vector2Add(app.posCarro, Vector2Scale(dir, speed * dt));
                app.anguloCarro = atan2f(dir.y, dir.x) * RAD2DEG + 90;
            }
            app.cam.target = Vector2Lerp(app.cam.target, app.posCarro, 0.1f);
        }

        // --- Construção visual ---
        BeginDrawing();
        ClearBackground(COR_FUNDO);
        BeginMode2D(app.cam);
            DrawMap(&app, &assets);
            DrawRoute(app.g, app.rota);
            
            Vector2 pDes = GetNodePos(app.g, app.destino);
            // Bandeira ancorada corretamente no chão
            if (app.chegou && assets.flag_icon.id > 0) 
                DrawTexturePro(assets.flag_icon, (Rectangle){0,0,assets.flag_icon.width,assets.flag_icon.height}, (Rectangle){pDes.x, pDes.y, 50, 50}, (Vector2){25,50}, 0, WHITE);
            else if (!app.chegou && !app.navegando && assets.seta_local.id > 0 && app.rota) 
                DrawTexturePro(assets.seta_local, (Rectangle){0,0,assets.seta_local.width,assets.seta_local.height}, (Rectangle){pDes.x, pDes.y, 40, 40}, (Vector2){20,40}, 0, WHITE);

            if (assets.car_icon.id > 0) 
                DrawTexturePro(assets.car_icon, (Rectangle){0,0,assets.car_icon.width,assets.car_icon.height}, (Rectangle){app.posCarro.x, app.posCarro.y, 40, 80}, (Vector2){20,40}, app.anguloCarro, WHITE);
            else DrawCircleV(app.posCarro, 12, ORANGE);
        EndMode2D();
        
        // --- INTERFACE ---
        DrawRectangleRec(btnGo, GREEN); DrawText("INICIAR", btnGo.x+25, btnGo.y+15, 20, WHITE);
        DrawRectangleRec(btnNovo, BLUE); DrawText("NOVO", btnNovo.x+35, btnNovo.y+15, 20, WHITE);
        DrawRectangleRec(btnAlt, ORANGE); DrawText("ALTERN.", btnAlt.x+25, btnAlt.y+15, 20, WHITE);
        
        DrawText("GPS DIJKSTRA", 10, 10, 20, WHITE);
        DrawText("Dir: Mover | Scroll: Zoom | Esq: Destino", 10, 35, 10, LIGHTGRAY); // Legendas 
        
        if (app.chegou) DrawText("VOCÊ CHEGOU AO SEU DESTINO!\nCLIQUE EM NOVO PARA INICIAR UMA NOVA ROTA.", 10, 60, 20, YELLOW); //ALERT 
        else if (app.navegando) DrawText("NAVEGANDO...", 10, 60, 20, GREEN);
        else {
            char distText[50];
            sprintf(distText, "Destino: %.1f km", app.distanciaKm);
            DrawText(distText, 10, 60, 20, LIGHTGRAY);
        }
        
        EndDrawing();
    }
    
    UnloadTexture(assets.car_icon); UnloadTexture(assets.flag_icon);
    CloseWindow();
    return 0;
}