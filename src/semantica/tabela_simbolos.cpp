#include "tabela_simbolos.h"

// =====================================================
// CONSTRUTOR
// =====================================================

TabelaSimbolos::TabelaSimbolos() {
    proximoEndereco = 0;

    keywords = {
        "INT", "FLOAT", "IF", "ELSE",
        "WHILE", "RETURN", "VOID"
    };
}

// =====================================================
// FUNÇÃO AUXILIAR
// =====================================================

std::string TabelaSimbolos::chave(
    const std::string& escopo,
    const std::string& nome) const {

    return escopo + "::" + nome;
}

// =====================================================
// INSERÇÃO
// =====================================================

bool TabelaSimbolos::inserirIdentificador(
    const std::string& nome,
    const std::string& tipo,
    const std::string& escopo) {

    std::string k = chave(escopo, nome);

    if (tabela.find(k) != tabela.end()) {
        std::cerr << "[ERRO SEMANTICO] Variavel '"
                  << nome
                  << "' ja declarada no escopo '"
                  << escopo
                  << "'!\n";

        return false;
    }

    EntradaSimbolo entrada;

    entrada.nome = nome;
    entrada.tipo = tipo;
    entrada.escopo = escopo;
    entrada.endereco = proximoEndereco;
    entrada.valor = 0;

    tabela[k] = entrada;

    proximoEndereco += 4;

    return true;
}

// =====================================================
// EXISTE
// =====================================================

bool TabelaSimbolos::existe(
    const std::string& nome,
    const std::string& escopoAtual) const {

    if (tabela.find(chave(escopoAtual, nome)) != tabela.end())
        return true;

    if (tabela.find(chave("global", nome)) != tabela.end())
        return true;

    return false;
}

// =====================================================
// TIPO
// =====================================================

std::string TabelaSimbolos::obterTipo(
    const std::string& nome,
    const std::string& escopoAtual) const {

    auto it = tabela.find(chave(escopoAtual, nome));

    if (it != tabela.end())
        return it->second.tipo;

    it = tabela.find(chave("global", nome));

    if (it != tabela.end())
        return it->second.tipo;

    return "";
}

// =====================================================
// ENDEREÇO
// =====================================================

int TabelaSimbolos::obterEndereco(
    const std::string& nome,
    const std::string& escopoAtual) const {

    auto it = tabela.find(chave(escopoAtual, nome));

    if (it != tabela.end())
        return it->second.endereco;

    it = tabela.find(chave("global", nome));

    if (it != tabela.end())
        return it->second.endereco;

    return -1;
}

// =====================================================
// ATUALIZA VALOR
// =====================================================

void TabelaSimbolos::atualizarValor(
    const std::string& nome,
    const std::string& escopoAtual,
    int valor) {

    auto it = tabela.find(chave(escopoAtual, nome));

    if (it != tabela.end()) {
        it->second.valor = valor;
        return;
    }

    it = tabela.find(chave("global", nome));

    if (it != tabela.end()) {
        it->second.valor = valor;
    }
}

// =====================================================
// KEYWORDS
// =====================================================

bool TabelaSimbolos::ehKeyword(
    const std::string& palavra) const {

    for (const auto& kw : keywords) {
        if (kw == palavra)
            return true;
    }

    return false;
}

// =====================================================
// LISTAR
// =====================================================

void TabelaSimbolos::listarTodos() const {

    std::cout << "\n========== TABELA DE SIMBOLOS ==========\n";

    std::cout << "Nome\tTipo\tEscopo\tEndereco\tValor\n";

    for (const auto& par : tabela) {

        const EntradaSimbolo& e = par.second;

        std::cout << e.nome << "\t"
                  << e.tipo << "\t"
                  << e.escopo << "\t"
                  << e.endereco << "\t\t"
                  << e.valor << "\n";
    }

    std::cout << "========================================\n";
}

// =====================================================
// OBTER POR ESCOPO
// =====================================================

std::vector<EntradaSimbolo> TabelaSimbolos::obterPorEscopo(
    const std::string& escopo) const {

    std::vector<EntradaSimbolo> resultado;
    for (const auto& par : tabela) {
        if (par.second.escopo == escopo)
            resultado.push_back(par.second);
    }
    return resultado;
}

// =====================================================
// LISTAR ESCOPOS DE FUNCOES
// =====================================================

std::vector<std::string> TabelaSimbolos::listarEscopos() const {
    std::vector<std::string> escopos;
    std::unordered_map<std::string, bool> vistos;
    for (const auto& par : tabela) {
        const std::string& e = par.second.escopo;
        if (e != "global" && !vistos[e]) {
            vistos[e] = true;
            escopos.push_back(e);
        }
    }
    return escopos;
}