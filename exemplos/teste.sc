// ==========================================
// Arquivo: teste.sc
// Teste completo para o Mini-Compilador
// ==========================================

/* 1. Teste de Escopo Global e tipos diferentes
*/
int global_x = 100;
float taxa = 5.5;

/* 2. Teste de Funcao, parametros e escopo local
*/
int calcula_dobro(int valor) {
    int dobro = valor * 2;
    return dobro;
}

/* 3. Funcao Main e estruturas de controle
*/
int main() {
    // Variaveis locais (devem ir para o AR do main)
    int a = 10;
    int b = 20;
    int resultado;

    // Teste de precedencia matematica (AST deve colocar * abaixo do +)
    resultado = a + b * 2; 

    // Teste de laco de repeticao
    int contador = 0;
    while (contador < 3) {
        contador = contador + 1;
    }

    // Teste de desvio condicional
    if (resultado >= 50) {
        resultado = 0;
    } else {
        resultado = 1;
    }

    return 0;
}