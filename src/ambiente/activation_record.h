#ifndef ACTIVATION_RECORD_H
#define ACTIVATION_RECORD_H

#include <string>
#include <unordered_map>
#include <iostream>

struct Variavel {
    std::string nome;
    std::string tipo;
    int valor;
    std::string escopo;
};

class RegistroAtivacao {
public:
    std::string nomeFuncao;
    std::string enderecoRetorno;
    int valorRetorno;
    std::string linkDinamico;

    std::unordered_map<std::string, Variavel> variaveis;

    RegistroAtivacao(std::string funcao, std::string retorno, std::string chamador)
        : nomeFuncao(funcao),
          enderecoRetorno(retorno),
          valorRetorno(0),
          linkDinamico(chamador) {}

    void adicionarParametro(std::string nome, std::string tipo, int valor) {
        variaveis[nome] = {nome, tipo, valor, "parametro"};
    }

    void adicionarVariavel(std::string nome, std::string tipo, int valor) {
        variaveis[nome] = {nome, tipo, valor, "local"};
    }

    bool atribuir(std::string nome, int valor) {
        if (variaveis.find(nome) != variaveis.end()) {
            variaveis[nome].valor = valor;
            return true;
        }
        return false;
    }

    bool obterValor(std::string nome, int& out) const {
        auto it = variaveis.find(nome);
        if (it != variaveis.end()) {
            out = it->second.valor;
            return true;
        }
        return false;
    }

    void imprimir() const {
        std::cout << "  +----------------------------------+\n";
        std::cout << "  | AR de: " << nomeFuncao << "\n";
        std::cout << "  | Link Dinamico -> " << linkDinamico << "\n";
        std::cout << "  | Endereco Retorno: " << enderecoRetorno << "\n";
        std::cout << "  | Valor de Retorno: " << valorRetorno << "\n";
        if (!variaveis.empty()) {
            std::cout << "  | Variaveis:\n";
            for (const auto& par : variaveis) {
                const Variavel& v = par.second;
                std::cout << "  |   [" << v.escopo << "] "
                          << v.tipo << " " << v.nome
                          << " = " << v.valor << "\n";
            }
        }
        std::cout << "  +----------------------------------+\n";
    }
};

#endif
