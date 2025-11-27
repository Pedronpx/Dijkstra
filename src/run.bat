@echo off
echo Configurando ambiente Raylib...
:: Adiciona o compilador correto ao inicio do PATH temporariamente
set PATH=C:\raylib\w64devkit\bin;%PATH%

echo Compilando projeto...
gcc main.c graph.c via.c lista.c priority_queue.c utils.c geo.c svg.c qry.c hash.c smutreap.c fila.c -o waze_app.exe -O1 -Wall -std=c99 -Wno-missing-braces -I. -L. -lraylib -lopengl32 -lgdi32 -lwinmm

if %errorlevel% neq 0 (
    echo [ERRO] Falha na compilacao.
    pause
    exit /b
)

echo [SUCESSO] Executando Wazir App...
waze_app.exe
pause