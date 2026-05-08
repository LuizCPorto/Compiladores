#include "ast.h"
#include <iostream>

// ==========================================
// Implementação da classe NoNumero
// ==========================================

NoNumero::NoNumero(int v) {
    valor = v;
}

void NoNumero::imprimir() {
    std::cout << valor;
}

// ==========================================
// Implementação da classe NoOperacao
// ==========================================

NoOperacao::NoOperacao(std::string o, No* e, No* d) {
    op = o;
    esq = e;
    dir = d;
}

void NoOperacao::imprimir() {
    std::cout << "(";
    
    if (esq != nullptr) {
        esq->imprimir();
    }
    
    std::cout << " " << op << " ";
    
    if (dir != nullptr) {
        dir->imprimir();
    }
    
    std::cout << ")";
}