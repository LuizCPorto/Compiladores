#include <iostream>
#include <string>
#include "activation_record.h"
#include "pilha_execucao.h"
#include "tabela_simbolos.h"

// ============================================================
// Simula a execução de:
//
//   INT soma(INT a, INT b) {
//       INT r = a + b;
//       return r;
//   }
//   INT main() {
//       INT x = soma(2, 3);
//   }
//
// Demonstrando: criação/destruição de ARs, pilha de execução,
// tabela de símbolos com escopos, e geração de código intermediário.
// ============================================================

TabelaSimbolos tabela;
PilhaExecucao  pilha;

// Gera código intermediário (três endereços)
static int contadorTemp = 1;
std::string novoTemp() {
    return "t" + std::to_string(contadorTemp++);
}

// ----------------------------------------------------------
// Simula a função soma(a, b)
// ----------------------------------------------------------
int executarSoma(int valA, int valB) {
    // === CALL: empilha AR de soma ===
    pilha.chamarFuncao("soma", "main() linha apos chamada");

    RegistroAtivacao& ar = pilha.topo();

    // Registra parâmetros na tabela de símbolos e no AR
    tabela.inserirIdentificador("a", "INT", "soma");
    tabela.inserirIdentificador("b", "INT", "soma");
    ar.adicionarParametro("a", "INT", valA);
    ar.adicionarParametro("b", "INT", valB);

    // Registra variável local r
    tabela.inserirIdentificador("r", "INT", "soma");
    ar.adicionarVariavel("r", "INT", 0);

    // === CÓDIGO INTERMEDIÁRIO: r = a + b ===
    std::cout << "\n--- CODIGO INTERMEDIARIO (soma) ---\n";
    std::string t1 = novoTemp();
    int valA_ar, valB_ar;
    ar.obterValor("a", valA_ar);
    ar.obterValor("b", valB_ar);
    std::cout << t1 << " = " << valA_ar << " + " << valB_ar << "\n";
    int resultadoSoma = valA_ar + valB_ar;
    std::cout << "r = " << t1 << "  =>  r = " << resultadoSoma << "\n";
    std::cout << "-----------------------------------\n";

    // Atualiza r no AR e na tabela
    ar.atribuir("r", resultadoSoma);
    tabela.atualizarValor("r", "soma", resultadoSoma);

    // Valor de retorno
    ar.valorRetorno = resultadoSoma;

    // === RETURN: desempilha AR de soma ===
    int retorno = pilha.retornarFuncao();
    return retorno;
}

// ----------------------------------------------------------
// Simula main()
// ----------------------------------------------------------
void executarMain() {
    // === CALL: empilha AR de main ===
    pilha.chamarFuncao("main", "SO (sistema operacional)");

    RegistroAtivacao& arMain = pilha.topo();

    // Declara variável local x em main
    tabela.inserirIdentificador("x", "INT", "main");
    arMain.adicionarVariavel("x", "INT", 0);

    // === Chamada de soma(2, 3) ===
    std::cout << "\n[COMPUTE] Chamando soma(2, 3)...\n";
    int resultado = executarSoma(2, 3);

    // === Atribuição x = resultado ===
    std::cout << "\n--- CODIGO INTERMEDIARIO (main) ---\n";
    std::string t2 = novoTemp();
    std::cout << t2 << " = retorno de soma  =>  " << t2 << " = " << resultado << "\n";
    std::cout << "x = " << t2 << "  =>  x = " << resultado << "\n";
    std::cout << "-----------------------------------\n";

    // Atualiza x no AR e na tabela
    arMain.atribuir("x", resultado);
    tabela.atualizarValor("x", "main", resultado);

    std::cout << "\n[ASSIGN] x = " << resultado << "\n";

    // Exibe estado final do AR de main
    std::cout << "\n[INFO] Estado final do AR de main:\n";
    pilha.topo().imprimir();

    // === Fim de main: desempilha ===
    pilha.retornarFuncao();

    std::cout << "\n[END] main() encerrou. Pilha esvaziada.\n";
}

// ----------------------------------------------------------
// main do próprio ambiente de execução
// ----------------------------------------------------------
int main() {
    std::cout << "========================================\n";
    std::cout << "  AMBIENTE DE EXECUCAO - SimpleC\n";
    std::cout << "  Semana 7 - Compiladores\n";
    std::cout << "========================================\n";

    std::cout << "\n[LOAD] Programa carregado na memoria.\n";
    std::cout << "[LOAD] Regioes: Codigo | Dados Globais | Heap | Stack\n";

    // Executa a simulação
    executarMain();

    // Exibe tabela de símbolos final com escopos e endereços
    tabela.listarTodos();

    std::cout << "========================================\n";
    std::cout << "  Execucao concluida com sucesso!\n";
    std::cout << "========================================\n";

    return 0;
}
