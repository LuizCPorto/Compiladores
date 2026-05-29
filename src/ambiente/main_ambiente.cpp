#include <iostream>

#include "pilha_execucao.h"
#include "../semantica/tabela_simbolos.h"

static TabelaSimbolos tabela;
static PilhaExecucao pilha;

static int executarSoma(int valA, int valB) {
    std::cout << "[call] soma(a=" << valA << ",b=" << valB << ")\n";

    pilha.chamarFuncao("soma", "main:apos_soma");

    tabela.inserirIdentificador("a", "int", "soma", "parametro");
    tabela.inserirIdentificador("b", "int", "soma", "parametro");
    tabela.inserirIdentificador("r", "int", "soma", "local");

    pilha.topo().adicionarParametro("a", "int", valA, tabela.obterEndereco("a", "soma"));
    pilha.topo().adicionarParametro("b", "int", valB, tabela.obterEndereco("b", "soma"));
    pilha.topo().adicionarVariavel("r", "int", 0, tabela.obterEndereco("r", "soma"));

    int a = 0;
    int b = 0;
    pilha.topo().obterValor("a", a);
    pilha.topo().obterValor("b", b);

    int resultado = a + b;
    std::cout << "[compute] r = a + b = " << resultado << "\n";

    pilha.topo().atribuir("r", resultado);
    tabela.atualizarValor("r", "soma", resultado);

    pilha.topo().valorRetorno = resultado;
    std::cout << "[return] soma = " << resultado << "\n";

    return pilha.retornarFuncao();
}

static void executarMain() {
    std::cout << "[call] main()\n";
    pilha.chamarFuncao("main", "SO");

    tabela.inserirIdentificador("x", "int", "main", "local");
    pilha.topo().adicionarVariavel("x", "int", 0, tabela.obterEndereco("x", "main"));

    int resultado = executarSoma(2, 3);

    pilha.topo().atribuir("x", resultado);
    tabela.atualizarValor("x", "main", resultado);
    std::cout << "[assign] x = " << resultado << "\n";

    pilha.imprimirEstadoPilha();
    pilha.retornarFuncao();

    std::cout << "[end] main()\n";
}

int main() {
    std::cout << "=== AMBIENTE DE EXECUCAO ===\n";
    executarMain();
    tabela.listarTodos();
    return 0;
}
