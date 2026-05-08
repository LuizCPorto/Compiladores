@echo off
echo ========================================
echo Inciando a compilacao da SimpleC...
echo ========================================

:: 1. Roda o Bison (Gera os arquivos dentro de src/parser/)
echo [1/3] Gerando o Parser (Bison)...
win_bison -d src/parser/parser.y -o src/parser/parser.tab.c

:: 2. Roda o Flex (Gera o arquivo dentro de src/lexer/)
echo [2/3] Gerando o Lexer (Flex)...
win_flex -o src/lexer/lex.yy.c src/lexer/lexer.l

:: 3. Junta tudo e cria o executável
echo [3/3] Compilando o executavel com GCC...
g++ src/parser/parser.tab.c src/lexer/lex.yy.c -o SimpleC.exe

echo ========================================
echo Sucesso! O compilador "SimpleC.exe" foi gerado.
echo ========================================