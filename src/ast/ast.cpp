#include "ast.h"
#include <iostream>

static int contador_temp = 1;

// ===== NoNumero =====

NoNumero::NoNumero(int v) {
    valor = v;
}

void NoNumero::imprimir() {
    std::cout << valor;
}

std::string NoNumero::gerarCodigo() {
    return std::to_string(valor);
}

// ===== NoOperacao =====

NoOperacao::NoOperacao(std::string o, No* e, No* d) {
    op = o;
    esq = e;
    dir = d;
}

void NoOperacao::imprimir() {
    std::cout << "(";
    if (esq) esq->imprimir();
    std::cout << " " << op << " ";
    if (dir) dir->imprimir();
    std::cout << ")";
}

std::string NoOperacao::gerarCodigo() {
    if (op == "=") {
        std::string dir_temp = dir != nullptr ? dir->gerarCodigo() : esq->gerarCodigo();
        std::cout << "variavel_alvo = " << dir_temp << "\n";
        return "";
    }

    std::string esq_temp = esq != nullptr ? esq->gerarCodigo() : "";
    std::string dir_temp = dir != nullptr ? dir->gerarCodigo() : "";
    
    std::string novo_temp = "t" + std::to_string(contador_temp++);
    std::cout << novo_temp << " = " << esq_temp << " " << op << " " << dir_temp << "\n";
    
    return novo_temp;
}