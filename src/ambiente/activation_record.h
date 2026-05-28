#ifndef ACTIVATION_RECORD_H
#define ACTIVATION_RECORD_H

#include <string>
#include <unordered_map>
#include <iostream>

// Representa uma variável dentro de um registro de ativação
struct Variavel {
    std::string nome;
    std::string tipo;
    int valor;
    std::string escopo; // "local" ou "parametro"
};

// Registro de Ativação (Activation Record)
// Criado na pilha sempre que uma função é chamada
class RegistroAtivacao {
public:
    std::string nomeFuncao;         // Nome da função dona deste registro
    std::string enderecoRetorno;    // Para onde voltar após o retorno
    int valorRetorno;               // Valor devolvido pela função
    std::string linkDinamico;       // Nome do registro chamador (AR pai)

    // Variáveis e parâmetros locais desta função
    std::unordered_map<std::string, Variavel> variaveis;

    RegistroAtivacao(std::string funcao, std::string retorno, std::string chamador)
        : nomeFuncao(funcao),
          enderecoRetorno(retorno),
          valorRetorno(0),
          linkDinamico(chamador) {}

    // Adiciona um parâmetro formal ao registro
    void adicionarParametro(std::string nome, std::string tipo, int valor) {
        variaveis[nome] = {nome, tipo, valor, "parametro"};
    }

    // Adiciona uma variável local ao registro
    void adicionarVariavel(std::string nome, std::string tipo, int valor) {
        variaveis[nome] = {nome, tipo, valor, "local"};
    }

    // Atualiza o valor de uma variável existente
    bool atribuir(std::string nome, int valor) {
        if (variaveis.find(nome) != variaveis.end()) {
            variaveis[nome].valor = valor;
            return true;
        }
        return false;
    }

    // Busca o valor de uma variável
    bool obterValor(std::string nome, int& out) const {
        auto it = variaveis.find(nome);
        if (it != variaveis.end()) {
            out = it->second.valor;
            return true;
        }
        return false;
    }

    // Exibe o conteúdo do registro para fins de log/debug
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
