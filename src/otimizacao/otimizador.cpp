#include "otimizador.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <sstream>

static bool ehNumero(const std::string& s) {
    if (s.empty())
        return false;

    size_t i = (s[0] == '-') ? 1 : 0;
    bool viuDigito = false;
    bool viuPonto = false;

    for (; i < s.size(); i++) {
        if (std::isdigit(static_cast<unsigned char>(s[i]))) {
            viuDigito = true;
        } else if (s[i] == '.' && !viuPonto) {
            viuPonto = true;
        } else {
            return false;
        }
    }

    return viuDigito;
}

static bool ehTemp(const std::string& s) {
    if (s.size() < 2 || s[0] != 't')
        return false;

    for (size_t i = 1; i < s.size(); i++) {
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
            return false;
    }

    return true;
}

static std::vector<std::string> tokens(const std::string& linha) {
    std::istringstream iss(linha);
    std::vector<std::string> out;
    std::string tok;

    while (iss >> tok)
        out.push_back(tok);

    return out;
}

static std::string montarAtribuicao(const std::string& destino, const std::string& valor) {
    return destino + " = " + valor;
}

static std::string montarOperacao(
    const std::string& destino,
    const std::string& a,
    const std::string& op,
    const std::string& b
) {
    return destino + " = " + a + " " + op + " " + b;
}

static std::string substituirConstante(
    const std::string& valor,
    const std::map<std::string, std::string>& constantes
) {
    auto it = constantes.find(valor);

    if (it != constantes.end())
        return it->second;

    return valor;
}

static std::vector<std::string> propagarConstantes(const std::vector<std::string>& entrada) {
    std::vector<std::string> saida;
    std::map<std::string, std::string> constantes;

    for (const std::string& linha : entrada) {
        std::vector<std::string> t = tokens(linha);

        if (t.size() == 3 && t[1] == "=") {
            std::string valor = substituirConstante(t[2], constantes);
            saida.push_back(montarAtribuicao(t[0], valor));

            if (ehNumero(valor))
                constantes[t[0]] = valor;
            else
                constantes.erase(t[0]);
        } else if (t.size() == 5 && t[1] == "=") {
            std::string a = substituirConstante(t[2], constantes);
            std::string b = substituirConstante(t[4], constantes);
            saida.push_back(montarOperacao(t[0], a, t[3], b));
            constantes.erase(t[0]);
        } else {
            saida.push_back(linha);
        }
    }

    return saida;
}

static std::vector<std::string> simplificarAlgebra(const std::vector<std::string>& entrada) {
    std::vector<std::string> saida;

    for (const std::string& linha : entrada) {
        std::vector<std::string> t = tokens(linha);

        if (t.size() != 5 || t[1] != "=") {
            saida.push_back(linha);
            continue;
        }

        const std::string& destino = t[0];
        const std::string& a = t[2];
        const std::string& op = t[3];
        const std::string& b = t[4];

        if (op == "+" && b == "0")
            saida.push_back(montarAtribuicao(destino, a));
        else if (op == "+" && a == "0")
            saida.push_back(montarAtribuicao(destino, b));
        else if (op == "*" && b == "1")
            saida.push_back(montarAtribuicao(destino, a));
        else if (op == "*" && a == "1")
            saida.push_back(montarAtribuicao(destino, b));
        else if (op == "*" && (a == "0" || b == "0"))
            saida.push_back(montarAtribuicao(destino, "0"));
        else if (op == "-" && a == b)
            saida.push_back(montarAtribuicao(destino, "0"));
        else if (op == "/" && b == "1")
            saida.push_back(montarAtribuicao(destino, a));
        else
            saida.push_back(linha);
    }

    return saida;
}

static std::vector<std::string> eliminarSubexpressoes(const std::vector<std::string>& entrada) {
    std::vector<std::string> saida;
    std::map<std::string, std::string> expressoes;

    for (const std::string& linha : entrada) {
        std::vector<std::string> t = tokens(linha);

        if (t.size() == 5 && t[1] == "=") {
            std::string chave = t[2] + " " + t[3] + " " + t[4];
            auto it = expressoes.find(chave);

            if (it != expressoes.end())
                saida.push_back(montarAtribuicao(t[0], it->second));
            else {
                expressoes[chave] = t[0];
                saida.push_back(linha);
            }
        } else {
            saida.push_back(linha);
        }
    }

    return saida;
}

static std::vector<std::string> eliminarCodigoMorto(const std::vector<std::string>& entrada) {
    std::set<std::string> usados;

    for (const std::string& linha : entrada) {
        std::vector<std::string> t = tokens(linha);

        if (t.size() >= 3) {
            for (size_t i = 2; i < t.size(); i++) {
                if (t[i] != "+" && t[i] != "-" && t[i] != "*" && t[i] != "/" && !ehNumero(t[i]))
                    usados.insert(t[i]);
            }
        }
    }

    std::vector<std::string> saida;

    for (const std::string& linha : entrada) {
        std::vector<std::string> t = tokens(linha);

        if (t.size() >= 3 && t[1] == "=" && ehTemp(t[0]) && usados.find(t[0]) == usados.end())
            continue;

        saida.push_back(linha);
    }

    return saida;
}

std::vector<std::string> Otimizador::otimizar(const std::vector<std::string>& entrada) {
    std::vector<std::string> saida = entrada;
    saida = propagarConstantes(saida);
    saida = simplificarAlgebra(saida);
    saida = eliminarSubexpressoes(saida);
    saida = eliminarCodigoMorto(saida);
    return saida;
}
