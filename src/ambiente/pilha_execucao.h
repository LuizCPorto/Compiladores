#ifndef PILHA_EXECUCAO_H
#define PILHA_EXECUCAO_H

#include "activation_record.h"
#include <vector>
#include <iostream>
#include <stdexcept>

// Simula a Pilha de Execução (Stack) do programa
// Cada chamada de função empilha um AR; cada retorno desempilha
class PilhaExecucao {
private:
    std::vector<RegistroAtivacao> pilha;

public:
    // Cria e empilha um novo Activation Record (chamada de função)
    void chamarFuncao(const std::string& nomeFuncao,
                      const std::string& enderecoRetorno) {
        std::string chamador = pilha.empty() ? "SO" : pilha.back().nomeFuncao;

        std::cout << "\n[CALL] Criando AR para: " << nomeFuncao
                  << " | retorna para: " << enderecoRetorno
                  << " | chamado por: " << chamador << "\n";

        pilha.emplace_back(nomeFuncao, enderecoRetorno, chamador);
        imprimirEstadoPilha();
    }

    // Retorna o AR do topo (função atual em execução)
    RegistroAtivacao& topo() {
        if (pilha.empty())
            throw std::runtime_error("Pilha vazia! Nenhuma funcao em execucao.");
        return pilha.back();
    }

    // Remove o AR do topo (retorno de função)
    int retornarFuncao() {
        if (pilha.empty())
            throw std::runtime_error("Pilha vazia! Nao ha funcao para retornar.");

        RegistroAtivacao ar = pilha.back();
        int valorRet = ar.valorRetorno;

        std::cout << "\n[RETURN] Encerrando AR de: " << ar.nomeFuncao
                  << " | valor de retorno: " << valorRet
                  << " | voltando para: " << ar.enderecoRetorno << "\n";

        pilha.pop_back();
        imprimirEstadoPilha();
        return valorRet;
    }

    // Verifica se a pilha está vazia
    bool vazia() const {
        return pilha.empty();
    }

    // Tamanho atual da pilha
    int tamanho() const {
        return (int)pilha.size();
    }

    // Busca uma variável subindo pela pilha (escopo léxico simplificado)
    bool buscarVariavel(const std::string& nome, int& out) {
        // Busca do topo para a base
        for (int i = (int)pilha.size() - 1; i >= 0; --i) {
            if (pilha[i].obterValor(nome, out)) {
                return true;
            }
        }
        return false;
    }

    // Imprime o estado visual da pilha (topo para base)
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
