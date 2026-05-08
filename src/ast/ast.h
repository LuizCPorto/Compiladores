#ifndef AST_H
#define AST_H

#include <string>

// Classe base (Interface) para qualquer nó da árvore
class No {
public:
    virtual ~No() {}
    virtual void imprimir() = 0;
};

// Nó para representar números inteiros
class NoNumero : public No {
public:
    int valor;
    
    NoNumero(int v);
    void imprimir() override;
};

// Nó para representar operações (soma, subtração, etc.)
class NoOperacao : public No {
public:
    std::string op;
    No* esq;
    No* dir;
    
    NoOperacao(std::string o, No* e, No* d);
    void imprimir() override;
};

#endif