#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

struct EntradaSimbolo {
    std::string nome;
    std::string tipo;
    std::string escopo;
    int endereco;
    int valor;
};

class TabelaSimbolos {
private:
    int proximoEndereco;

    std::vector<std::string> keywords;

    std::unordered_map<std::string, EntradaSimbolo> tabela;

    std::string chave(const std::string& escopo,
                      const std::string& nome) const;

public:
    TabelaSimbolos();

    bool inserirIdentificador(const std::string& nome,
                              const std::string& tipo,
                              const std::string& escopo = "global");

    bool existe(const std::string& nome,
                const std::string& escopoAtual) const;

    std::string obterTipo(const std::string& nome,
                          const std::string& escopoAtual) const;

    int obterEndereco(const std::string& nome,
                      const std::string& escopoAtual) const;

    void atualizarValor(const std::string& nome,
                        const std::string& escopoAtual,
                        int valor);

    bool ehKeyword(const std::string& palavra) const;

    void listarTodos() const;

    // Retorna todos os simbolos de um escopo especifico
    std::vector<EntradaSimbolo> obterPorEscopo(const std::string& escopo) const;

    // Retorna nomes de escopos de funcoes (exclui "global")
    std::vector<std::string> listarEscopos() const;
};

#endif