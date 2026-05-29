#include "tabela_simbolos.h"

#include <iomanip>
#include <iostream>

TabelaSimbolos::TabelaSimbolos()
    : keywords({
          "if",
          "else",
          "while",
          "return",
          "int",
          "float",
          "void",
          "main"
      }) {}

std::string TabelaSimbolos::chave(const std::string& escopo, const std::string& nome) const {
    return escopo + "::" + nome;
}

bool TabelaSimbolos::inserirIdentificador(
    const std::string& nome,
    const std::string& tipo,
    const std::string& escopo,
    const std::string& categoria
) {
    std::string k = chave(escopo, nome);

    if (tabela.find(k) != tabela.end()) {
        std::cerr << "[ERRO SEMANTICO] Identificador '"
                  << nome
                  << "' ja declarado no escopo '"
                  << escopo
                  << "'.\n";

        return false;
    }

    int endereco = proximoEnderecoPorEscopo[escopo];
    int tamanho = tipo == "float" ? 8 : 4;

    EntradaSimbolo entrada;
    entrada.nome = nome;
    entrada.tipo = tipo;
    entrada.escopo = escopo;
    entrada.categoria = categoria;
    entrada.endereco = endereco;
    entrada.valor = 0;
    entrada.temValor = false;

    tabela[k] = entrada;
    ordemInsercao.push_back(k);
    proximoEnderecoPorEscopo[escopo] += tamanho;

    return true;
}

bool TabelaSimbolos::existeNoEscopo(const std::string& nome, const std::string& escopo) const {
    return tabela.find(chave(escopo, nome)) != tabela.end();
}

bool TabelaSimbolos::existe(const std::string& nome, const std::string& escopoAtual) const {
    return existeNoEscopo(nome, escopoAtual) || existeNoEscopo(nome, "global");
}

std::string TabelaSimbolos::obterTipo(const std::string& nome, const std::string& escopoAtual) const {
    auto it = tabela.find(chave(escopoAtual, nome));

    if (it != tabela.end())
        return it->second.tipo;

    it = tabela.find(chave("global", nome));

    if (it != tabela.end())
        return it->second.tipo;

    return "";
}

int TabelaSimbolos::obterEndereco(const std::string& nome, const std::string& escopoAtual) const {
    auto it = tabela.find(chave(escopoAtual, nome));

    if (it != tabela.end())
        return it->second.endereco;

    it = tabela.find(chave("global", nome));

    if (it != tabela.end())
        return it->second.endereco;

    return -1;
}

std::string TabelaSimbolos::obterCategoria(const std::string& nome, const std::string& escopoAtual) const {
    auto it = tabela.find(chave(escopoAtual, nome));

    if (it != tabela.end())
        return it->second.categoria;

    it = tabela.find(chave("global", nome));

    if (it != tabela.end())
        return it->second.categoria;

    return "";
}

void TabelaSimbolos::atualizarValor(
    const std::string& nome,
    const std::string& escopoAtual,
    double valor
) {
    auto it = tabela.find(chave(escopoAtual, nome));

    if (it == tabela.end())
        it = tabela.find(chave("global", nome));

    if (it != tabela.end()) {
        it->second.valor = valor;
        it->second.temValor = true;
    }
}

bool TabelaSimbolos::ehKeyword(const std::string& palavra) const {
    for (const std::string& kw : keywords) {
        if (kw == palavra)
            return true;
    }

    return false;
}

void TabelaSimbolos::listarTodos() const {
    std::cout << "\n========== TABELA DE SIMBOLOS ==========\n";
    std::cout << std::left
              << std::setw(14) << "Nome"
              << std::setw(10) << "Tipo"
              << std::setw(12) << "Escopo"
              << std::setw(12) << "Categoria"
              << std::setw(10) << "Endereco"
              << "Valor\n";

    for (const std::string& k : ordemInsercao) {
        const EntradaSimbolo& e = tabela.at(k);

        std::cout << std::left
                  << std::setw(14) << e.nome
                  << std::setw(10) << e.tipo
                  << std::setw(12) << e.escopo
                  << std::setw(12) << e.categoria
                  << std::setw(10) << e.endereco;

        if (e.temValor)
            std::cout << e.valor;
        else
            std::cout << "-";

        std::cout << "\n";
    }

    std::cout << "========================================\n";
}

void TabelaSimbolos::limpar() {
    tabela.clear();
    ordemInsercao.clear();
    proximoEnderecoPorEscopo.clear();
}
