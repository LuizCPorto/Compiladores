#include "codigo_intermediario.h"

#include <iostream>

std::vector<std::string> CodigoIntermediario::instrucoes;
int CodigoIntermediario::contadorTemp = 1;

void CodigoIntermediario::limpar() {
    instrucoes.clear();
    reiniciarTemporarios();
}

void CodigoIntermediario::reiniciarTemporarios() {
    contadorTemp = 1;
}

std::string CodigoIntermediario::novoTemp() {
    return "t" + std::to_string(contadorTemp++);
}

void CodigoIntermediario::adicionar(const std::string& instrucao) {
    instrucoes.push_back(instrucao);
}

const std::vector<std::string>& CodigoIntermediario::obterInstrucoes() {
    return instrucoes;
}

void CodigoIntermediario::definirInstrucoes(const std::vector<std::string>& novasInstrucoes) {
    instrucoes = novasInstrucoes;
}

void CodigoIntermediario::imprimir(const std::string& titulo) {
    std::cout << "\n=== " << titulo << " ===\n";

    for (const std::string& instrucao : instrucoes)
        std::cout << instrucao << "\n";
}
