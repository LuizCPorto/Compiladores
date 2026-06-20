#include <iostream>
#include <cstdio>

#include "../semantica/tabela_simbolos.h"

TabelaSimbolos tabela;

extern int   yyparse();
extern FILE* yyin;

int main() {
    const char* arquivo = "exemplos/teste03.sc";

    yyin = fopen(arquivo, "r");
    if (!yyin) {
        std::cerr << "[AVISO] Nao foi possivel abrir '" << arquivo << "'.\n";
        std::cerr << "        Lendo da entrada padrao (stdin)...\n\n";
    }

    std::cout << "========================================\n";
    std::cout << "  COMPILADOR SimpleC\n";
    std::cout << "========================================\n";

    yyparse();

    if (yyin && yyin != stdin) {
        fclose(yyin);
        yyin = nullptr;
    }

    std::cout << "\n========================================\n";
    std::cout << "  Compilacao concluida!\n";
    std::cout << "========================================\n";

    return 0;
}
