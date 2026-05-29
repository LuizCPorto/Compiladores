#ifndef ACTIVATION_RECORD_H
#define ACTIVATION_RECORD_H

#include <iostream>
#include <string>
#include <unordered_map>

struct Variavel {
    std::string nome;
    std::string tipo;
    int valor;
    int endereco;
    std::string categoria;
};

class RegistroAtivacao {
public:
    std::string nomeFuncao;
    std::string enderecoRetorno;
    int valorRetorno;
    std::string linkDinamico;
    std::unordered_map<std::string, Variavel> variaveis;

    RegistroAtivacao(const std::string& funcao, const std::string& retorno, const std::string& chamador)
        : nomeFuncao(funcao),
          enderecoRetorno(retorno),
          valorRetorno(0),
          linkDinamico(chamador) {}

    void adicionarParametro(const std::string& nome, const std::string& tipo, int valor, int endereco) {
        variaveis[nome] = {nome, tipo, valor, endereco, "parametro"};
    }

    void adicionarVariavel(const std::string& nome, const std::string& tipo, int valor, int endereco) {
        variaveis[nome] = {nome, tipo, valor, endereco, "local"};
    }

    bool atribuir(const std::string& nome, int valor) {
        auto it = variaveis.find(nome);

        if (it == variaveis.end())
            return false;

        it->second.valor = valor;
        return true;
    }

    bool obterValor(const std::string& nome, int& out) const {
        auto it = variaveis.find(nome);

        if (it == variaveis.end())
            return false;

        out = it->second.valor;
        return true;
    }

    void imprimir() const {
        std::cout << "  AR(" << nomeFuncao << ") retorno=" << enderecoRetorno
                  << " link=" << linkDinamico
                  << " valorRetorno=" << valorRetorno << "\n";

        for (const auto& par : variaveis) {
            const Variavel& v = par.second;
            std::cout << "    [" << v.categoria << "] "
                      << v.tipo << " " << v.nome
                      << " endereco=" << v.endereco
                      << " valor=" << v.valor << "\n";
        }
    }
};

#endif
