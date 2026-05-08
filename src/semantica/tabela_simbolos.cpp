#include "tabela_simbolos.h"

// ===== KEYWORDS =====

bool TabelaSimbolos::ehKeyword(std::string palavra) {
    for (const auto& kw : keywords) {
        if (kw == palavra) return true;
    }
    return false;
}

std::vector<std::string> TabelaSimbolos::getKeywords() {
    return keywords;
}

// ===== IDENTIFICADORES =====

bool TabelaSimbolos::inserirIdentificador(std::string nome, std::string tipo) {
    // Primeiro verificamos se a variável já existe
    if (existeIdentificador(nome)) {
        return false; // Erro Semântico! Não insere.
    }
    
    // Se não existir, guarda na memória
    identificadores[nome] = tipo;
    return true; // Sucesso!
}

bool TabelaSimbolos::existeIdentificador(std::string nome) {
    // O método 'find' procura a chave. Se for diferente de 'end()', encontrou.
    return identificadores.find(nome) != identificadores.end();
}

std::string TabelaSimbolos::obterTipoIdentificador(std::string nome) {
    if (existeIdentificador(nome)) {
        return identificadores[nome]; // Devolve o tipo (ex: "INT")
    }
    return ""; // Retorna vazio se a variável não existir
}

std::unordered_map<std::string, std::string> TabelaSimbolos::getIdentificadores() {
    return identificadores;
}

// ===== OPERADORES =====

bool TabelaSimbolos::ehOperador(std::string op) {
    for (const auto& o : operadores) {
        if (o == op) return true;
    }
    return false;
}

std::vector<std::string> TabelaSimbolos::getOperadores() {
    return operadores;
}

// ===== ATRIBUIÇÃO =====

bool TabelaSimbolos::ehAtribuicao(std::string op) {
    for (const auto& a : atribuicao) {
        if (a == op) return true;
    }
    return false;
}

std::vector<std::string> TabelaSimbolos::getAtribuicao() {
    return atribuicao;
}

// ===== DELIMITADORES =====

bool TabelaSimbolos::ehDelimitador(std::string delim) {
    for (const auto& d : delimitadores) {
        if (d == delim) return true;
    }
    return false;
}

std::vector<std::string> TabelaSimbolos::getDelimitadores() {
    return delimitadores;
}

// ===== UTILITÁRIOS =====

void TabelaSimbolos::listarTodos() {
    std::cout << "\n========== TABELA DE SÍMBOLOS ==========" << std::endl;
    
    std::cout << "\n[KEYWORDS]" << std::endl;
    for (const auto& kw : keywords) {
        std::cout << "  - " << kw << std::endl;
    }
    
    std::cout << "\n[IDENTIFICADORES]" << std::endl;
    if (identificadores.empty()) {
        std::cout << "  (nenhum)" << std::endl;
    } else {
        for (const auto& id : identificadores) {
            std::cout << "  - " << id.first << " (" << id.second << ")" << std::endl;
        }
    }
    
    std::cout << "\n[OPERADORES]" << std::endl;
    for (const auto& op : operadores) {
        std::cout << "  - " << op << std::endl;
    }
    
    std::cout << "\n[ATRIBUIÇÃO]" << std::endl;
    for (const auto& a : atribuicao) {
        std::cout << "  - " << a << std::endl;
    }
    
    std::cout << "\n[DELIMITADORES]" << std::endl;
    for (const auto& d : delimitadores) {
        std::cout << "  - " << d << std::endl;
    }
    
    std::cout << "\n========================================\n" << std::endl;
}

void TabelaSimbolos::limpar() {
    identificadores.clear();
}