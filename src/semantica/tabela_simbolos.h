#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class TabelaSimbolos {
private:
    // KeyWords: palavras reservadas da linguagem
    std::vector<std::string> keywords = {
        "IF", "ELSE", "IF ELSE", "WHILE", "INT", "STRING", "DECIMAL", "NULL", "VOID"
    };
    
    // Identificadores: nomes de variáveis e funções
    std::unordered_map<std::string, std::string> identificadores; // nome -> tipo
    
    // Operadores: +, -, *, /
    std::vector<std::string> operadores = {
        "+", "-", "*", "/"
    };
    
    // Atribuição: =
    std::vector<std::string> atribuicao = {
        "="
    };
    
    // Delimitadores: ;, {, ), (, }, |
    std::vector<std::string> delimitadores = {
        ";", "{", ")", "(", "}", "|"
    };

public:
    // ===== KEYWORDS =====
    bool ehKeyword(std::string palavra);
    std::vector<std::string> getKeywords();
    
    // ===== IDENTIFICADORES =====
    bool inserirIdentificador(std::string nome, std::string tipo);
    bool existeIdentificador(std::string nome);
    std::string obterTipoIdentificador(std::string nome);
    std::unordered_map<std::string, std::string> getIdentificadores();
    
    // ===== OPERADORES =====
    bool ehOperador(std::string op);
    std::vector<std::string> getOperadores();
    
    // ===== ATRIBUIÇÃO =====
    bool ehAtribuicao(std::string op);
    std::vector<std::string> getAtribuicao();
    
    // ===== DELIMITADORES =====
    bool ehDelimitador(std::string delim);
    std::vector<std::string> getDelimitadores();
    
    // ===== UTILITÁRIOS =====
    void listarTodos();
    void limpar();
};

#endif