#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// Entrada da tabela de símbolos com escopo e endereço
struct EntradaSimbolo {
    std::string nome;
    std::string tipo;
    std::string escopo;   // "global" ou nome da função (ex: "main", "soma")
    int endereco;         // endereço simulado na memória
    int valor;
};

class TabelaSimbolos {
private:
    // Contador para simular endereços de memória
    int proximoEndereco = 100;

    // Keywords da linguagem
    std::vector<std::string> keywords = {
        "IF", "ELSE", "IF ELSE", "WHILE", "INT", "STRING", "DECIMAL", "NULL", "VOID"
    };

    // Mapa principal: "escopo::nome" -> EntradaSimbolo
    std::unordered_map<std::string, EntradaSimbolo> tabela;

    std::string chave(const std::string& escopo, const std::string& nome) const {
        return escopo + "::" + nome;
    }

public:
    // ===== INSERÇÃO COM ESCOPO =====

    bool inserirIdentificador(const std::string& nome, const std::string& tipo,
                               const std::string& escopo = "global") {
        std::string k = chave(escopo, nome);
        if (tabela.find(k) != tabela.end()) {
            std::cerr << "[ERRO SEMANTICO] Variavel '" << nome
                      << "' ja declarada no escopo '" << escopo << "'!\n";
            return false;
        }
        EntradaSimbolo entrada = {nome, tipo, escopo, proximoEndereco, 0};
        proximoEndereco += 4; // simula 4 bytes por int
        tabela[k] = entrada;

        std::cout << "[TABELA] Inserido: " << tipo << " " << nome
                  << " | escopo: " << escopo
                  << " | endereco: 0x" << std::hex << entrada.endereco
                  << std::dec << "\n";
        return true;
    }

    // ===== BUSCA COM ESCOPO =====

    // Busca primeiro no escopo local, depois no global
    bool existe(const std::string& nome, const std::string& escopoAtual) const {
        if (tabela.find(chave(escopoAtual, nome)) != tabela.end()) return true;
        if (tabela.find(chave("global", nome)) != tabela.end()) return true;
        return false;
    }

    std::string obterTipo(const std::string& nome, const std::string& escopoAtual) const {
        auto it = tabela.find(chave(escopoAtual, nome));
        if (it != tabela.end()) return it->second.tipo;
        it = tabela.find(chave("global", nome));
        if (it != tabela.end()) return it->second.tipo;
        return "";
    }

    int obterEndereco(const std::string& nome, const std::string& escopoAtual) const {
        auto it = tabela.find(chave(escopoAtual, nome));
        if (it != tabela.end()) return it->second.endereco;
        it = tabela.find(chave("global", nome));
        if (it != tabela.end()) return it->second.endereco;
        return -1;
    }

    void atualizarValor(const std::string& nome, const std::string& escopoAtual, int valor) {
        auto it = tabela.find(chave(escopoAtual, nome));
        if (it != tabela.end()) { it->second.valor = valor; return; }
        it = tabela.find(chave("global", nome));
        if (it != tabela.end()) it->second.valor = valor;
    }

    // ===== KEYWORDS =====

    bool ehKeyword(const std::string& palavra) const {
        for (const auto& kw : keywords)
            if (kw == palavra) return true;
        return false;
    }

    // ===== LISTAGEM =====

    void listarTodos() const {
        std::cout << "\n========== TABELA DE SIMBOLOS ==========\n";
        std::cout << "  Nome       | Tipo | Escopo   | Endereco | Valor\n";
        std::cout << "  -----------+------+----------+----------+------\n";
        for (const auto& par : tabela) {
            const EntradaSimbolo& e = par.second;
            std::cout << "  " << e.nome
                      << "          | " << e.tipo
                      << "  | " << e.escopo
                      << "  | 0x" << std::hex << e.endereco << std::dec
                      << "      | " << e.valor << "\n";
        }
        std::cout << "=========================================\n\n";
    }

    void limpar() {
        tabela.clear();
        proximoEndereco = 100;
    }
};

#endif
