# Trabalho de Compiladores

Bem-vindos ao repositório do nosso mini-compilador, desenvolvido como trabalho prático para a disciplina de Compiladores (UFT).

Este projeto implementa um **pipeline completo de compilação** — da análise léxica até a geração de código assembly x86-64 — usando Flex, Bison e C++.

**Equipe:** Luiz, Matheus, Vrass e Thalita.

---

## Pipeline de Compilação

O compilador executa **7 fases em sequência**:

| # | Fase | Ferramenta |
|---|------|-----------|
| 1 | Análise Léxica | Flex (`lexer.l`) |
| 2 | Análise Sintática + construção da AST | Bison (`parser.y`) |
| 3 | Análise Semântica (tabela de símbolos) | integrada ao parser |
| 4 | Geração de Código Intermediário (3 endereços) | `ast.cpp` |
| 5 | Otimização Independente de Máquina | `ast.cpp` |
| 6 | **Geração de Código Final (Assembly x86-64)** | `ast.cpp` |
| 7 | Tabela de Símbolos (saída final) | `tabela_simbolos.cpp` |

---

## Estrutura do Projeto

```text
/
├── src/
│   ├── lexer/
│   │   └── lexer.l              # Regras do Analisador Léxico (Tokens e Regex)
│   ├── parser/
│   │   └── parser.y             # Gramática EBNF + orquestração das fases
│   ├── ast/
│   │   ├── ast.h                # Hierarquia de nós da AST
│   │   └── ast.cpp              # Código intermediário, otimização e assembly
│   ├── semantica/
│   │   ├── tabela_simbolos.h
│   │   └── tabela_simbolos.cpp  # Tabela de símbolos com escopo
│   └── ambiente/
│       └── main_ambiente.cpp    # Ponto de entrada do compilador
├── exemplos/
│   ├── teste01.sc               # Variáveis globais
│   ├── teste02.sc               # Função com parâmetro
│   └── teste03.sc               # Expressão complexa com otimizações
├── saida.asm                    # Assembly gerado (criado ao rodar o compilador)
├── compilar.bat                 # Script de automação (Windows)
└── README.md                    # Este arquivo
```

---

## Pre-requisitos

Você precisa do GCC, Flex e Bison. A forma mais fácil no Windows é via **Chocolatey**:

```powershell
# Abra o PowerShell como Administrador
choco install mingw -y
choco install winflexbison3 -y
```

---

## Como Compilar

```powershell
.\compilar.bat
```

O script executa Bison, Flex e GCC automaticamente, gerando `compilador.exe`.

---

## Como Executar

```powershell
.\compilador.exe
```

O arquivo de entrada padrão é `exemplos/teste03.sc` (definido em `main_ambiente.cpp`).

### Saída produzida

Ao executar, o compilador exibe no console **e** grava arquivos:

1. **Tokens identificados** — tabela léxica
2. **AST** — árvore sintática abstrata com indentação visual
3. **Código intermediário original** — 3 endereços (ex: `t1 = a + b`)
4. **Otimizações aplicadas** — regras algébricas, dobramento de constantes, código morto
5. **Código intermediário otimizado** — com contagem de temporários reduzida
6. **Assembly x86-64** — instruções reais para o processador (AT&T/GAS syntax)
7. **Tabela de símbolos** — nome, tipo, escopo, endereço e valor de cada variável

O assembly também é salvo automaticamente em **`saida.asm`**, pronto para ser compilado:

```bash
# Em Linux ou WSL:
gcc -no-pie saida.asm -o programa
./programa
echo $?   # exibe o valor de retorno de main
```
