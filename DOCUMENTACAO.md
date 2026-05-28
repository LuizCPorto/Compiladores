# 📚 Documentação Completa do Projeto - SimpleC Compiler

## 📋 Índice

1. [Visão Geral](#visão-geral)
2. [Arquitetura do Projeto](#arquitetura-do-projeto)
3. [Componentes do Sistema](#componentes-do-sistema)
4. [Estrutura de Pastas](#estrutura-de-pastas)
5. [Detalhes de Implementação](#detalhes-de-implementação)
6. [Fluxo de Compilação](#fluxo-de-compilação)
7. [Equipe](#equipe)

---

## 🎯 Visão Geral

O **SimpleC** é um mini-compilador desenvolvido como trabalho prático para a disciplina de Compiladores da UFT. O projeto implementa as etapas principais do processo de compilação:

- **Análise Léxica**: Tokenização do código-fonte
- **Análise Sintática**: Verificação gramatical
- **Estrutura Abstrata (AST)**: Representação em árvore do código

O projeto utiliza as ferramentas clássicas:
- **Flex**: Para gerar o analisador léxico
- **Bison**: Para gerar o analisador sintático
- **GCC**: Para compilar o código C/C++ final

---

## 🏗️ Arquitetura do Projeto

```
┌─────────────────────────────────────────┐
│         Código-fonte (.sc)              │
└────────────────┬────────────────────────┘
                 │
        ┌────────▼────────┐
        │  ANÁLISE LÉXICA │ (Flex)
        │   (lexer.l)     │
        └────────┬────────┘
                 │
              Tokens
                 │
        ┌────────▼────────────┐
        │ ANÁLISE SINTÁTICA   │ (Bison)
        │  (parser.y)         │
        └────────┬────────────┘
                 │
              AST (Árvore de Sintaxe Abstrata)
                 │
        ┌────────▼─────────────────────┐
        │  ANÁLISE SEMÂNTICA          │
        │  (Tabela de Símbolos)       │
        │  - Verificação de tipos     │
        │  - Gerenciamento de escopo  │
        └────────┬─────────────────────┘
                 │
        ┌────────▼────────┐
        │   GCC Compile   │
        │  (Linking)      │
        └────────┬────────┘
                 │
        ┌────────▼────────┐
        │ SimpleC.exe     │
        └─────────────────┘
```

---

## 🔧 Componentes do Sistema

### 1️⃣ **Análise Léxica (`src/lexer/lexer.l`)**

O analisador léxico é responsável por converter o código-fonte em tokens.

**Tokens Reconhecidos:**
| Token | Símbolo | Descrição |
|-------|---------|-----------|
| `T_INT` | `INT` | Palavra-chave para declaração de inteiro |
| `T_ID` | `[a-zA-Z]+` | Identificadores (nomes de variáveis) |
| `T_NUMERO` | `[0-9]+` | Números inteiros |
| `T_ATRIB` | `=` | Operador de atribuição |
| `T_PONTOVIRGULA` | `;` | Terminador de instrução |

**Funcionalidades:**
- Ignora espaços em branco e quebras de linha
- Reporta erros léxicos quando encontra caracteres inválidos
- Retorna tokens para o parser processar

**Exemplo de Tokenização:**
```
Entrada: INT x = 10;
Saída:   T_INT T_ID T_ATRIB T_NUMERO T_PONTOVIRGULA
```

---

### 2️⃣ **Análise Sintática (`src/parser/parser.y`)**

O analisador sintático verifica se a sequência de tokens segue as regras gramaticais definidas.

**Regras Gramaticais (EBNF):**
```
programa    → declaracao
declaracao  → T_INT T_ID T_ATRIB T_NUMERO T_PONTOVIRGULA
```

**Funcionalidades:**
- Define a estrutura válida de um programa
- Valida sequências de tokens
- Emite mensagens de sucesso ou erro
- Faz interface com o AST para construir a árvore de sintaxe

**Exemplo de Validação:**
```
✅ INT x = 10;      (VÁLIDO)
❌ INT x =;         (INVÁLIDO - falta número)
❌ x = 10;          (INVÁLIDO - falta tipo)
```

---

### 3️⃣ **Árvore Sintática Abstrata (`src/ast/ast.h` e `src/ast/ast.cpp`)**

A AST representa a estrutura hierárquica do programa de forma abstrata.

**Classes Definidas:**

#### `No` (Classe Base)
```cpp
class No {
public:
    virtual ~No() {}
    virtual void imprimir() = 0;  // Método abstrato
};
```
- Interface base para todos os nós da árvore
- Define o contrato que todos os nós devem seguir

#### `NoNumero` (Nó para Números)
```cpp
class NoNumero : public No {
public:
    int valor;
    NoNumero(int v);
    void imprimir() override;
};
```
- Representa literais numéricos
- Armazena o valor inteiro
- Implementa impressão do valor

#### `NoOperacao` (Nó para Operações)
```cpp
class NoOperacao : public No {
public:
    std::string op;   // Operador (+, -, *, /, etc.)
    No* esq;          // Subárvore esquerda
    No* dir;          // Subárvore direita
    
    NoOperacao(std::string o, No* e, No* d);
    void imprimir() override;
};
```
- Representa operações binárias
- Mantém referências para subárvores esquerda e direita
- Suporta expressões matemáticas

**Exemplo de AST:**
```
Código:     5 + 3
AST:        
            NoOperacao(+)
           /             \
       NoNumero(5)    NoNumero(3)
```

---

## 📁 Estrutura de Pastas

```
Compiladores/
│
├── 📄 README.md                    # Guia rápido do projeto
├── 📄 DOCUMENTACAO.md              # Este arquivo
├── 📄 COMO_EXECUTAR.md             # Guia de execução
├── 📄 compilar.bat                 # Script de compilação (Windows)
│
├── src/                            # Código-fonte principal
│   ├── lexer/
│   │   ├── lexer.l                 # Regras do Flex (análise léxica)
│   │   └── lex.yy.c                # Saída gerada pelo Flex
│   │
│   ├── parser/
│   │   ├── parser.y                # Regras do Bison (análise sintática)
│   │   ├── parser.tab.c            # Saída gerada pelo Bison
│   │   └── parser.tab.h            # Headers gerados pelo Bison
│   │
│   ├── ast/
│   │   ├── ast.h                   # Declaração das classes AST (header)
│   │   └── ast.cpp                 # Implementação das classes AST
│   │
│   └── semantica/
│       ├── tabela_simbolos.h       # Declaração da Tabela de Símbolos
│       └── tabela_simbolos.cpp     # Implementação da Tabela de Símbolos
│
├── exemplos/                       # Arquivos de teste
│   └── teste.sc                    # Código-exemplo para testes
│
└── .git/                           # Repositório Git
    .vscode/                        # Configurações VS Code
```

---

## 🔍 Detalhes de Implementação

### Fluxo de Dados

```
┌──────────────────┐
│  Código Fonte    │  Arquivo: exemplos/teste.sc
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  Lexer (Flex)    │  Lê arquivo → Gera tokens
└────────┬─────────┘
         │
    Tokens: [T_INT, T_ID, T_ATRIB, T_NUMERO, T_PONTOVIRGULA]
         │
         ▼
┌──────────────────┐
│ Parser (Bison)   │  Valida gramática → Cria AST
└────────┬─────────┘
         │
      AST Node
         │
         ▼
┌──────────────────────────┐
│ Análise Semântica        │  Verifica tipos e semântica
│ (Tabela de Símbolos)     │  Detecta erros semânticos
└────────┬─────────────────┘
         │
┌────────▼──────────┐
│ Processamento     │  Executa/Compila resultado
│    Final          │
└───────────────────┘
```

### Integração entre Componentes

**Lexer → Parser:**
- O Lexer gera tokens usando as regras em `lexer.l`
- Cada token é retornado como uma constante definida no `parser.tab.h`
- O Parser consome esses tokens seguindo a gramática em `parser.y`

**Parser → AST:**
- Quando o Parser reconhece uma regra gramatical, cria nós da AST
- Os nós são instâncias das classes definidas em `ast.h`
- A árvore é navegada pelo `main()` do programa

---

## 🔨 Fluxo de Compilação

### Passo 1: Gerar Parser com Bison
```bash
win_bison -d src/parser/parser.y -o src/parser/parser.tab.c
```
- `-d`: Gera arquivo de headers (`parser.tab.h`)
- Saídas: `parser.tab.c` e `parser.tab.h`

### Passo 2: Gerar Lexer com Flex
```bash
win_flex -o src/lexer/lex.yy.c src/lexer/lexer.l
```
- Saída: `lex.yy.c` (código do analisador léxico)

### Passo 3: Compilar com GCC
```bash
g++ src/parser/parser.tab.c src/lexer/lex.yy.c src/ast/ast.cpp -o SimpleC.exe
```
- Combina todos os arquivos compilados
- Gera executável final: `SimpleC.exe`

---

### 4️⃣ **Análise Semântica e Tabela de Símbolos (`src/semantica/`)**

A análise semântica verifica o significado e a validade do código além da sintaxe, incluindo verificação de tipos e gerenciamento de variáveis.

#### `TabelaSimbolos` (Classe Principal)

A tabela de símbolos organiza todos os elementos lexicais da linguagem em 5 categorias principais:

| Categoria | Exemplos | Descrição |
|-----------|----------|-----------|
| **KeyWords** | `IF`, `ELSE`, `WHILE`, `INT`, `STRING`, `DECIMAL`, `NULL`, `VOID` | Palavras reservadas da linguagem |
| **Identifier** | `soma`, `valor`, `x` | Nomes de variáveis e funções |
| **Operator** | `+`, `-`, `*`, `/`, `>`, `<` | Operadores matemáticos e lógicos |
| **Assignment** | `=` | Operador de atribuição |
| **Delimiter** | `;`, `(`, `)`, `{`, `}`, `\|` | Delimitadores de código |

Essa tabela organiza os símbolos básicos da linguagem e serve como referência para o lexer e o parser durante a análise léxica e sintática. Ela separa palavras reservadas, identificadores, operadores, atribuição e delimitadores, facilitando a validação e a leitura do código.

---

## 👥 Equipe

- **Luiz**
- **Matheus**
- **Vrass**
- **Thalita**

---

## 📝 Notas Técnicas

### Dependências Externas
- **GCC**: Compilador C/C++
- **Flex** (win_flex): Gerador de analisadores léxicos
- **Bison** (win_bison): Gerador de analisadores sintáticos

### Versão da Linguagem
- **C++**: Utilizado para as classes da AST (C++11+)
- **C**: Utilizado pelo Flex e Bison

### Plataforma
- **Windows**: Scripts em `.bat` para automação
- Testado com MinGW GCC

### Próximas Extensões Possíveis
- ➕ Suporte a mais tipos de dados (FLOAT, CHAR, STRING)
- ➕ Operações matemáticas completas (+, -, *, /)
- ➕ Estruturas de controle (if, while, for)
- ➕ Funções e procedimentos
- ➕ Otimizações de código intermediário

---

**Documento atualizado em:** Maio de 2026  
**Versão:** 1.0
