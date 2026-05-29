@echo off
setlocal

echo ========================================
echo Iniciando a compilacao da SimpleC...
echo ========================================

echo [1/4] Usando src/parser/parser.tab.c existente.
if not exist src/parser/parser.tab.c goto falta_parser

where win_flex >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [2/4] Gerando o Lexer com Flex...
    win_flex -o src/lexer/lex.yy.c src/lexer/lexer.l
    if errorlevel 1 goto erro
) else (
    echo [2/4] win_flex nao encontrado. Usando src/lexer/lex.yy.c existente.
    if not exist src/lexer/lex.yy.c goto falta_lexer
)

echo [3/4] Compilando o compilador com GCC...
g++ src/parser/parser.tab.c src/lexer/lex.yy.c src/ast/ast.cpp src/semantica/tabela_simbolos.cpp src/intermediario/codigo_intermediario.cpp src/otimizacao/otimizador.cpp src/codigo_final/gerador_codigo_final.cpp -o SimpleC.exe
if errorlevel 1 goto erro

echo [4/4] Compilando o demonstrador do ambiente de execucao...
g++ src/ambiente/main_ambiente.cpp src/semantica/tabela_simbolos.cpp -o AmbienteExecucao.exe
if errorlevel 1 goto erro

echo ========================================
echo Sucesso! SimpleC.exe e AmbienteExecucao.exe foram gerados.
echo ========================================
exit /b 0

:falta_parser
echo ERRO: src/parser/parser.tab.c nao existe e win_bison nao esta no PATH.
exit /b 1

:falta_lexer
echo ERRO: src/lexer/lex.yy.c nao existe e win_flex nao esta no PATH.
exit /b 1

:erro
echo ========================================
echo Erro durante a compilacao.
echo ========================================
exit /b 1
