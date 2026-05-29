#include <iostream>
#include <string>

#include "activation_record.h"
#include "pilha_execucao.h"
#include "../semantica/tabela_simbolos.h"

// ============================================================
// Ambiente global
// ============================================================

TabelaSimbolos tabela;
PilhaExecucao pilha;

// ============================================================
// Temporários para código intermediário
// ============================================================

static int contadorTemp = 1;

std::string novoTemp() {
    return "t" + std::to_string(contadorTemp++);
}

// ============================================================
// Simula soma(a, b)
// ============================================================

int executarSoma(int valA, int valB) {

    // CALL
    pilha.chamarFuncao("soma", "main() linha apos chamada");

    RegistroAtivacao& ar = pilha.topo();

    // parâmetros
    tabela.inserirIdentificador("a", "INT", "soma");
    tabela.inserirIdentificador("b", "INT", "soma");

    ar.adicionarParametro("a", "INT", valA);
    ar.adicionarParametro("b", "INT", valB);

    tabela.atualizarValor("a", "soma", valA);
    tabela.atualizarValor("b", "soma", valB);

    // variável local
    tabela.inserirIdentificador("r", "INT", "soma");

    ar.adicionarVariavel("r", "INT", 0);

    // código intermediário
    std::cout << "\n--- CODIGO INTERMEDIARIO (soma) ---\n";

    std::string t1 = novoTemp();

    int a;
    int b;

    ar.obterValor("a", a);
    ar.obterValor("b", b);

    std::cout << t1 << " = " << a << " + " << b << "\n";

    int resultado = a + b;

    std::cout << "r = " << t1
              << "  =>  r = "
              << resultado << "\n";

    std::cout << "-----------------------------------\n";

    // atualiza r
    ar.atribuir("r", resultado);

    tabela.atualizarValor("r", "soma", resultado);

    // retorno
    ar.valorRetorno = resultado;

    return pilha.retornarFuncao();
}

// ============================================================
// Simula main()
// ============================================================

void executarMain() {

    // CALL
    pilha.chamarFuncao(
        "main",
        "SO (sistema operacional)"
    );

    // variável x
    tabela.inserirIdentificador(
        "x",
        "INT",
        "main"
    );

    pilha.topo().adicionarVariavel(
        "x",
        "INT",
        0
    );

    // chama soma
    std::cout << "\n[COMPUTE] Chamando soma(2, 3)...\n";

    int resultado = executarSoma(2, 3);

    // verifica variável declarada
    if (!tabela.existe("x", "main")) {

        std::cerr
            << "[ERRO SEMANTICO] Variavel 'x' nao declarada!\n";

        return;
    }

    // código intermediário
    std::cout << "\n--- CODIGO INTERMEDIARIO (main) ---\n";

    std::string t2 = novoTemp();

    std::cout
        << t2
        << " = retorno de soma  =>  "
        << t2
        << " = "
        << resultado
        << "\n";

    std::cout
        << "x = "
        << t2
        << "  =>  x = "
        << resultado
        << "\n";

    std::cout << "-----------------------------------\n";

    // atribui x
    pilha.topo().atribuir("x", resultado);

    tabela.atualizarValor(
        "x",
        "main",
        resultado
    );

    std::cout << "\n[ASSIGN] x = " << resultado << "\n";

    // estado final
    std::cout << "\n[INFO] Estado final do AR de main:\n";

    pilha.topo().imprimir();

    // RETURN
    pilha.retornarFuncao();

    std::cout
        << "\n[END] main() encerrou. Pilha esvaziada.\n";
}

// ============================================================
// Main do ambiente
// ============================================================

int main() {

    std::cout << "========================================\n";
    std::cout << "  AMBIENTE DE EXECUCAO - SimpleC\n";
    std::cout << "  Semana 7 - Compiladores\n";
    std::cout << "========================================\n";

    std::cout << "\n[LOAD] Programa carregado na memoria.\n";

    std::cout
        << "[LOAD] Regioes: Codigo | Dados Globais | Heap | Stack\n";

    executarMain();

    tabela.listarTodos();

    std::cout << "========================================\n";
    std::cout << "  Execucao concluida com sucesso!\n";
    std::cout << "========================================\n";

    return 0;
}