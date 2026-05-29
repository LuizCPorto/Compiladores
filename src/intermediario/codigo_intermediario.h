#ifndef CODIGO_INTERMEDIARIO_H
#define CODIGO_INTERMEDIARIO_H

#include <string>
#include <vector>

class CodigoIntermediario {
private:
    static std::vector<std::string> instrucoes;
    static int contadorTemp;

public:
    static void limpar();
    static void reiniciarTemporarios();
    static std::string novoTemp();
    static void adicionar(const std::string& instrucao);
    static const std::vector<std::string>& obterInstrucoes();
    static void definirInstrucoes(const std::vector<std::string>& novasInstrucoes);
    static void imprimir(const std::string& titulo);
};

#endif
