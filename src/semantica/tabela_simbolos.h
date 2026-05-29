#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

#include <string>
#include <unordered_map>
#include <vector>

struct EntradaSimbolo {
    std::string nome;
    std::string tipo;
    std::string escopo;
    std::string categoria;
    int endereco;
    double valor;
    bool temValor;
};

class TabelaSimbolos {
private:
    std::unordered_map<std::string, EntradaSimbolo> tabela;
    std::unordered_map<std::string, int> proximoEnderecoPorEscopo;
    std::vector<std::string> ordemInsercao;
    std::vector<std::string> keywords;

    std::string chave(const std::string& escopo, const std::string& nome) const;

public:
    TabelaSimbolos();

    bool inserirIdentificador(
        const std::string& nome,
        const std::string& tipo,
        const std::string& escopo = "global",
        const std::string& categoria = "variavel"
    );

    bool existeNoEscopo(const std::string& nome, const std::string& escopo) const;
    bool existe(const std::string& nome, const std::string& escopoAtual) const;

    std::string obterTipo(const std::string& nome, const std::string& escopoAtual) const;
    int obterEndereco(const std::string& nome, const std::string& escopoAtual) const;
    std::string obterCategoria(const std::string& nome, const std::string& escopoAtual) const;

    void atualizarValor(const std::string& nome, const std::string& escopoAtual, double valor);

    bool ehKeyword(const std::string& palavra) const;
    void listarTodos() const;
    void limpar();
};

#endif
