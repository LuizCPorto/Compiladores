#ifndef PILHA_EXECUCAO_H
#define PILHA_EXECUCAO_H

#include "activation_record.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

class PilhaExecucao {
private:
    std::vector<RegistroAtivacao> pilha;

public:
    void chamarFuncao(const std::string& nomeFuncao, const std::string& enderecoRetorno) {
        std::string chamador = pilha.empty() ? "SO" : pilha.back().nomeFuncao;

        std::cout << "[stack] cria AR de " << nomeFuncao << "\n";
        pilha.emplace_back(nomeFuncao, enderecoRetorno, chamador);
    }

    RegistroAtivacao& topo() {
        if (pilha.empty())
            throw std::runtime_error("Pilha vazia.");

        return pilha.back();
    }

    int retornarFuncao() {
        if (pilha.empty())
            throw std::runtime_error("Pilha vazia.");

        RegistroAtivacao ar = pilha.back();
        int valorRetorno = ar.valorRetorno;

        std::cout << "[stack] remove AR de " << ar.nomeFuncao << "\n";
        pilha.pop_back();

        return valorRetorno;
    }

    bool vazia() const {
        return pilha.empty();
    }

    void imprimirEstadoPilha() const {
        std::cout << "\n[stack] topo -> base\n";

        if (pilha.empty()) {
            std::cout << "  pilha vazia\n";
            return;
        }

        for (int i = static_cast<int>(pilha.size()) - 1; i >= 0; i--)
            pilha[i].imprimir();
    }
};

#endif
