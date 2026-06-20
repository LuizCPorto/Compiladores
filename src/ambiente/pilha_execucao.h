#ifndef PILHA_EXECUCAO_H
#define PILHA_EXECUCAO_H

#include "activation_record.h"
#include <vector>
#include <iostream>
#include <stdexcept>

class PilhaExecucao {
private:
    std::vector<RegistroAtivacao> pilha;

public:
    void chamarFuncao(const std::string& nomeFuncao,
                      const std::string& enderecoRetorno) {
        std::string chamador = pilha.empty() ? "SO" : pilha.back().nomeFuncao;

        std::cout << "\n[CALL] Criando AR para: " << nomeFuncao
                  << " | retorna para: " << enderecoRetorno
                  << " | chamado por: " << chamador << "\n";

        pilha.emplace_back(nomeFuncao, enderecoRetorno, chamador);
    }

    RegistroAtivacao& topo() {
        if (pilha.empty())
            throw std::runtime_error("Pilha vazia! Nenhuma funcao em execucao.");
        return pilha.back();
    }

    int retornarFuncao() {
        if (pilha.empty())
            throw std::runtime_error("Pilha vazia! Nao ha funcao para retornar.");

        RegistroAtivacao ar = pilha.back();
        int valorRet = ar.valorRetorno;

        std::cout << "\n[RETURN] Encerrando AR de: " << ar.nomeFuncao
                  << " | valor de retorno: " << valorRet
                  << " | voltando para: " << ar.enderecoRetorno << "\n";

        pilha.pop_back();
        return valorRet;
    }

    bool vazia() const {
        return pilha.empty();
    }

    int tamanho() const {
        return (int)pilha.size();
    }

    bool buscarVariavel(const std::string& nome, int& out) {
        for (int i = (int)pilha.size() - 1; i >= 0; --i) {
            if (pilha[i].obterValor(nome, out)) {
                return true;
            }
        }
        return false;
    }

    void imprimirEstadoPilha() const {
        std::cout << "\n  === ESTADO DA PILHA (topo -> base) ===\n";
        if (pilha.empty()) {
            std::cout << "  [ PILHA VAZIA ]\n";
        } else {
            for (int i = (int)pilha.size() - 1; i >= 0; --i) {
                if (i == (int)pilha.size() - 1)
                    std::cout << "  ^ Top of Stack\n";
                pilha[i].imprimir();
            }
        }
        std::cout << "  =======================================\n";
    }
};

#endif
