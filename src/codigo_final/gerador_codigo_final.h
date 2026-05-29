#ifndef GERADOR_CODIGO_FINAL_H
#define GERADOR_CODIGO_FINAL_H

#include <string>
#include <vector>

class GeradorCodigoFinal {
public:
    static std::vector<std::string> gerar(const std::vector<std::string>& intermediario);
    static void salvar(const std::string& caminho, const std::vector<std::string>& codigoFinal);
    static void imprimir(const std::vector<std::string>& codigoFinal);
};

#endif
