#include "gerador_codigo_final.h"

#include <fstream>
#include <iostream>
#include <sstream>

static std::vector<std::string> tokens(const std::string& linha) {
    std::istringstream iss(linha);
    std::vector<std::string> out;
    std::string tok;

    while (iss >> tok)
        out.push_back(tok);

    return out;
}

static std::string instrucaoOperador(const std::string& op) {
    if (op == "+")
        return "ADD";
    if (op == "-")
        return "SUB";
    if (op == "*")
        return "MUL";
    if (op == "/")
        return "DIV";

    return "OP";
}

std::vector<std::string> GeradorCodigoFinal::gerar(const std::vector<std::string>& intermediario) {
    std::vector<std::string> saida;
    int registrador = 1;

    for (const std::string& linha : intermediario) {
        std::vector<std::string> t = tokens(linha);

        if (t.empty())
            continue;

        if (t[0] == "func" && t.size() >= 3) {
            saida.push_back("LABEL " + t[2]);
            continue;
        }

        if (t[0] == "endfunc") {
            saida.push_back("END");
            continue;
        }

        if (t[0] == "return" && t.size() == 2) {
            saida.push_back("LOAD R1, " + t[1]);
            saida.push_back("RET R1");
            continue;
        }

        if (t.size() == 3 && t[1] == "=") {
            std::string r = "R" + std::to_string(registrador++);
            if (registrador > 3)
                registrador = 1;

            saida.push_back("LOAD " + r + ", " + t[2]);
            saida.push_back("STORE " + t[0] + ", " + r);
            continue;
        }

        if (t.size() == 5 && t[1] == "=") {
            std::string r = "R" + std::to_string(registrador++);
            if (registrador > 3)
                registrador = 1;

            saida.push_back("LOAD " + r + ", " + t[2]);
            saida.push_back(instrucaoOperador(t[3]) + " " + r + ", " + t[4]);
            saida.push_back("STORE " + t[0] + ", " + r);
        }
    }

    return saida;
}

void GeradorCodigoFinal::salvar(const std::string& caminho, const std::vector<std::string>& codigoFinal) {
    std::ofstream arquivo(caminho.c_str());

    for (const std::string& linha : codigoFinal)
        arquivo << linha << "\n";
}

void GeradorCodigoFinal::imprimir(const std::vector<std::string>& codigoFinal) {
    std::cout << "\n=== CODIGO FINAL (PSEUDO-ASSEMBLY) ===\n";

    for (const std::string& linha : codigoFinal)
        std::cout << linha << "\n";
}
