#include "../parser/parser.tab.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

static int lerChar() {
    return std::getchar();
}

static void devolverChar(int c) {
    if (c != EOF)
        std::ungetc(c, stdin);
}

static char* duplicar(const std::string& s) {
    char* out = static_cast<char*>(std::malloc(s.size() + 1));
    std::strcpy(out, s.c_str());
    return out;
}

static void ignorarComentarioLinha() {
    int c;

    while ((c = lerChar()) != EOF && c != '\n') {}
}

static void ignorarComentarioBloco() {
    int anterior = 0;
    int c;

    while ((c = lerChar()) != EOF) {
        if (anterior == '*' && c == '/')
            return;

        anterior = c;
    }
}

int yylex() {
    int c;

    while ((c = lerChar()) != EOF) {
        if (std::isspace(c))
            continue;

        if (std::isalpha(c) || c == '_') {
            std::string texto;
            texto += static_cast<char>(c);

            while ((c = lerChar()) != EOF && (std::isalnum(c) || c == '_'))
                texto += static_cast<char>(c);

            devolverChar(c);

            if (texto == "int" || texto == "INT")
                return T_INT;
            if (texto == "float" || texto == "FLOAT")
                return T_FLOAT;
            if (texto == "void" || texto == "VOID")
                return T_VOID;
            if (texto == "if" || texto == "IF")
                return T_IF;
            if (texto == "else" || texto == "ELSE")
                return T_ELSE;
            if (texto == "while" || texto == "WHILE")
                return T_WHILE;
            if (texto == "return" || texto == "RETURN")
                return T_RETURN;
            if (texto == "main" || texto == "MAIN")
                return T_MAIN;

            yylval.texto = duplicar(texto);
            return T_ID;
        }

        if (std::isdigit(c)) {
            std::string numero;
            bool ehFloat = false;
            numero += static_cast<char>(c);

            while ((c = lerChar()) != EOF && std::isdigit(c))
                numero += static_cast<char>(c);

            if (c == '.') {
                ehFloat = true;
                numero += '.';

                while ((c = lerChar()) != EOF && std::isdigit(c))
                    numero += static_cast<char>(c);
            }

            devolverChar(c);

            if (ehFloat) {
                yylval.valorFloat = static_cast<float>(std::atof(numero.c_str()));
                return T_NUMERO_FLOAT;
            }

            yylval.valorInteiro = std::atoi(numero.c_str());
            return T_NUMERO;
        }

        if (c == '/') {
            int prox = lerChar();

            if (prox == '/') {
                ignorarComentarioLinha();
                continue;
            }

            if (prox == '*') {
                ignorarComentarioBloco();
                continue;
            }

            devolverChar(prox);
            return T_DIV;
        }

        if (c == '<') {
            int prox = lerChar();
            if (prox == '=')
                return T_MENOR_IGUAL;

            devolverChar(prox);
            return T_MENOR;
        }

        if (c == '>') {
            int prox = lerChar();
            if (prox == '=')
                return T_MAIOR_IGUAL;

            devolverChar(prox);
            return T_MAIOR;
        }

        if (c == '=') {
            int prox = lerChar();
            if (prox == '=')
                return T_IGUAL_IGUAL;

            devolverChar(prox);
            return T_ATRIB;
        }

        if (c == '!') {
            int prox = lerChar();
            if (prox == '=')
                return T_DIFERENTE;

            devolverChar(prox);
            std::printf("Erro lexico: caractere '!' nao reconhecido\n");
            continue;
        }

        switch (c) {
        case '+': return T_MAIS;
        case '-': return T_MENOS;
        case '*': return T_MULT;
        case ';': return T_PONTOVIRGULA;
        case ',': return T_VIRGULA;
        case '(': return T_ABRE_PAREN;
        case ')': return T_FECHA_PAREN;
        case '{': return T_ABRE_CHAVE;
        case '}': return T_FECHA_CHAVE;
        default:
            std::printf("Erro lexico: caractere '%c' nao reconhecido\n", c);
            break;
        }
    }

    return YYEOF;
}

int yywrap() {
    return 1;
}
