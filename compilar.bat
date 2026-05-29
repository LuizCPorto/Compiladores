@echo off
echo ========================================
echo Iniciando a compilacao da SimpleC...
echo ========================================

:: 1. Roda o Bison (gera parser dentro de src/parser/)
echo [1/3] Gerando o Parser (Bison)...
win_bison -d src/parser/parser.y -o src/parser/parser.tab.c

:: 2. Roda o Flex (gera lexer dentro de src/lexer/)
echo [2/3] Gerando o Lexer (Flex)...
win_flex -o src/lexer/lex.yy.c src/lexer/lexer.l

:: 3. Compila tudo com GCC
echo [3/3] Compilando o executavel com GCC...
g++ src/parser/parser.tab.c ^
    src/lexer/lex.yy.c ^
    src/ast/ast.cpp ^
    src/semantica/tabela_simbolos.cpp ^
    src/ambiente/main_ambiente.cpp ^
    -o compilador.exe

echo ========================================
echo Sucesso! O compilador "compilador.exe" foi gerado.
echo Para executar: compilador.exe
echo ========================================
