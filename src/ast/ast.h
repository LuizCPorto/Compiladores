#ifndef AST_H
#define AST_H

#include <string>
#include <vector>

/* =========================================================
   Funções livres de apoio para imprimir a árvore.
   São inline para que estejam disponíveis em qualquer .cpp
   que inclua este cabeçalho.
   ========================================================= */
inline std::string astRamo(bool isLast)   { return isLast ? "L-- " : "|-- "; }
inline std::string astIndent(bool isLast) { return isLast ? "    " : "|   "; }

/* =========================================================
   Classe base — interface comum a todos os nós da AST.
   ========================================================= */
class No {
public:
    virtual ~No() {}

    // Ponto de entrada público: imprime a árvore a partir deste nó
    void imprimir() { imprimirNo("", true); }

    // Cada subclasse implementa este método (chamado internamente)
    virtual void imprimirNo(const std::string& pre, bool isLast) = 0;

    virtual std::string gerarCodigo() = 0;
};

/* ---------------------------------------------------------
   Literal inteiro: 5, 10, 123
   --------------------------------------------------------- */
class NoNumero : public No {
public:
    int valor;
    explicit NoNumero(int v);
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
};

/* ---------------------------------------------------------
   Literal float: 3.14, 2.5
   --------------------------------------------------------- */
class NoFloat : public No {
public:
    float valor;
    explicit NoFloat(float v);
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
};

/* ---------------------------------------------------------
   Variável: x, soma, resultado
   --------------------------------------------------------- */
class NoIdentificador : public No {
public:
    std::string nome;
    explicit NoIdentificador(const std::string& n);
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
};

/* ---------------------------------------------------------
   Operação aritmética: +  -  *  /
   --------------------------------------------------------- */
class NoOperacaoBinaria : public No {
public:
    std::string op;
    No* esq;
    No* dir;
    NoOperacaoBinaria(const std::string& o, No* e, No* d);
    ~NoOperacaoBinaria() override;
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
};

/* ---------------------------------------------------------
   Declaração de variável:
     int x;
     float y = 3.14;
   --------------------------------------------------------- */
class NoDeclaracao : public No {
public:
    std::string tipo;
    std::string nome;
    No* valorInicial; // nullptr se sem inicializacao
    NoDeclaracao(const std::string& tipo, const std::string& nome, No* val = nullptr);
    ~NoDeclaracao() override;
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
};

/* ---------------------------------------------------------
   Atribuicao:
     x = 10;
     x = a + b;
   --------------------------------------------------------- */
class NoAtribuicao : public No {
public:
    std::string nome;
    No* expressao;
    NoAtribuicao(const std::string& nome, No* expr);
    ~NoAtribuicao() override;
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
};

/* ---------------------------------------------------------
   Retorno:
     return r;
     return a + b;
   --------------------------------------------------------- */
class NoRetorno : public No {
public:
    No* expressao;
    explicit NoRetorno(No* expr);
    ~NoRetorno() override;
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
};

/* ---------------------------------------------------------
   Bloco / lista de comandos: { cmd1; cmd2; ... }
   --------------------------------------------------------- */
class NoBloco : public No {
public:
    std::vector<No*> comandos;
    void adicionar(No* cmd);
    ~NoBloco() override;
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
};

/* ---------------------------------------------------------
   Funcao:
     int soma(int a, int b) { ... }
   --------------------------------------------------------- */
class NoFuncao : public No {
public:
    std::string tipo;
    std::string nome;
    NoBloco* corpo;
    NoFuncao(const std::string& tipo, const std::string& nome, NoBloco* corpo);
    ~NoFuncao() override;
    void imprimirNo(const std::string& pre, bool isLast) override;
    std::string gerarCodigo() override;
};

#endif
