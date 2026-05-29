# Documentação Completa do Projeto - SimpleC Compiler

## Índice

1. [Visão Geral](#visão-geral)
2. [Arquitetura do Projeto](#arquitetura-do-projeto)
3. [Componentes do Sistema](#componentes-do-sistema)
4. [Estrutura de Pastas](#estrutura-de-pastas)
5. [Detalhes de Implementação](#detalhes-de-implementação)
6. [Fluxo de Compilação](#fluxo-de-compilação)
7. [Equipe](#equipe)

---

## Visão Geral

O **SimpleC** é um mini-compilador desenvolvido como trabalho prático para a disciplina de Compiladores da UFT. O projeto implementa as seguintes etapas do processo de compilação:

- **Análise Léxica**: Tokenização do código-fonte (Flex)
- **Análise Sintática**: Verificação gramatical e construção da AST (Bison)
- **Árvore Sintática Abstrata (AST)**: Representação hierárquica do programa
- **Análise Semântica**: Tabela de símbolos com escopo, verificação de declarações e tipos
- **Geração de Código Intermediário**: Código de 3 endereços com variáveis temporárias
- **Ambiente de Execução** *(módulo auxiliar)*: Simulação de pilha de ativação e chamadas de função

O projeto utiliza as ferramentas:
- **Flex** (`win_flex`): Para gerar o analisador léxico
- **Bison** (`win_bison`): Para gerar o analisador sintático
- **GCC** (`g++`): Para compilar o código C/C++ final

A linguagem aceita pelo compilador usa a extensão `.sc` e suporta declarações de variáveis `int`/`float`, funções com parâmetros, atribuições, expressões aritméticas com precedência e instruções de retorno.

---

## Arquitetura do Projeto

```
┌─────────────────────────────────────────┐
│         Código-fonte (.sc)              │
└────────────────┬────────────────────────┘
                 │
        ┌────────▼────────┐
        │  ANÁLISE LÉXICA │  (Flex / lexer.l)
        │  Tokenização    │
        └────────┬────────┘
                 │ Tokens
        ┌────────▼────────────┐
        │ ANÁLISE SINTÁTICA   │  (Bison / parser.y)
        │  Gramática + AST    │
        └────────┬────────────┘
                 │ AST + verificações semânticas
        ┌────────▼─────────────────────┐
        │  ANÁLISE SEMÂNTICA           │
        │  (tabela_simbolos)           │
        │  - Escopo local/global       │
        │  - Redeclaração / uso antes  │
        │    da declaração             │
        │  - Alocação de endereços     │
        └────────┬─────────────────────┘
                 │
        ┌────────▼──────────────────────┐
        │  GERAÇÃO DE CÓDIGO            │
        │  INTERMEDIÁRIO (AST::gerar)   │
        │  - Código de 3 endereços      │
        │  - Variáveis temporárias t1.. │
        └────────┬──────────────────────┘
                 │
        ┌────────▼────────┐
        │   SimpleC.exe   │
        └─────────────────┘
```

---

## Componentes do Sistema

### 1. Análise Léxica (`src/lexer/lexer.l`)

O analisador léxico converte o código-fonte em tokens usando o Flex.

**Tokens Reconhecidos:**

| Token | Padrão / Símbolo | Descrição |
|-------|-----------------|-----------|
| `T_INT` | `int` | Palavra-chave tipo inteiro |
| `T_FLOAT` | `float` | Palavra-chave tipo float |
| `T_IF` | `if` | Palavra-chave condicional |
| `T_ELSE` | `else` | Palavra-chave alternativa |
| `T_WHILE` | `while` | Palavra-chave laço |
| `T_RETURN` | `return` | Palavra-chave retorno |
| `T_VOID` | `void` | Palavra-chave tipo void |
| `T_MAIN` | `main` | Palavra-chave função principal |
| `T_NUMERO` | `[0-9]+` | Literal inteiro |
| `T_NUMERO_FLOAT` | `[0-9]+"."[0-9]+` | Literal ponto flutuante |
| `T_ID` | `[a-zA-Z_][a-zA-Z0-9_]*` | Identificador (variáveis/funções) |
| `T_MAIS` | `+` | Operador adição |
| `T_MENOS` | `-` | Operador subtração |
| `T_MULT` | `*` | Operador multiplicação |
| `T_DIV` | `/` | Operador divisão |
| `T_ATRIB` | `=` | Operador de atribuição |
| `T_MENOR` | `<` | Comparação menor que |
| `T_MAIOR` | `>` | Comparação maior que |
| `T_MENOR_IGUAL` | `<=` | Comparação menor ou igual |
| `T_MAIOR_IGUAL` | `>=` | Comparação maior ou igual |
| `T_IGUAL_IGUAL` | `==` | Comparação igualdade |
| `T_DIFERENTE` | `!=` | Comparação diferença |
| `T_PONTOVIRGULA` | `;` | Terminador de instrução |
| `T_VIRGULA` | `,` | Separador de parâmetros |
| `T_ABRE_PAREN` | `(` | Abre parêntese |
| `T_FECHA_PAREN` | `)` | Fecha parêntese |
| `T_ABRE_CHAVE` | `{` | Abre bloco |
| `T_FECHA_CHAVE` | `}` | Fecha bloco |

**Comportamentos especiais:**
- Ignora espaços em branco, tabulações e quebras de linha
- Ignora comentários de linha (`// ...`) e de bloco (`/* ... */`)
- Reporta erro léxico para caracteres não reconhecidos

**Exemplo de tokenização:**
```
Entrada: int x = 10;
Saída:   T_INT  T_ID("x")  T_ATRIB  T_NUMERO(10)  T_PONTOVIRGULA
```

> **Observação:** Os tokens `T_IF`, `T_ELSE` e `T_WHILE` são reconhecidos pelo lexer mas as regras gramaticais correspondentes ainda não estão implementadas no parser.

---

### 2. Análise Sintática (`src/parser/parser.y`)

O analisador sintático valida a gramática e constrói a AST. Integra diretamente com a tabela de símbolos para verificações semânticas durante o parsing.

**Variáveis globais de contexto:**
```c
TabelaSimbolos tabela;          // instância única da tabela de símbolos
std::string escopoAtual;        // escopo corrente ("global" ou nome da função)
std::string funcaoTipoAtual;    // tipo de retorno da função em análise
std::string funcaoNomeAtual;    // nome da função em análise
```

**Gramática implementada (EBNF):**

```
programa         → lista_comandos

lista_comandos   → lista_comandos comando
                 | comando

comando          → declaracao
                 | atribuicao
                 | retorno
                 | funcao

tipo             → T_INT | T_FLOAT

declaracao       → tipo T_ID T_PONTOVIRGULA
                 | tipo T_ID T_ATRIB expressao T_PONTOVIRGULA

atribuicao       → T_ID T_ATRIB expressao T_PONTOVIRGULA

retorno          → T_RETURN expressao T_PONTOVIRGULA

nome_funcao      → T_ID | T_MAIN

lista_parametros → lista_parametros T_VIRGULA tipo T_ID
                 | tipo T_ID

parametros       → lista_parametros | ε

bloco            → T_ABRE_CHAVE lista_comandos T_FECHA_CHAVE
                 | T_ABRE_CHAVE T_FECHA_CHAVE

funcao           → tipo nome_funcao T_ABRE_PAREN parametros T_FECHA_PAREN bloco

expressao        → expressao T_MAIS  termo
                 | expressao T_MENOS termo
                 | termo

termo            → termo T_MULT fator
                 | termo T_DIV  fator
                 | fator

fator            → T_NUMERO
                 | T_NUMERO_FLOAT
                 | T_ID
                 | T_ABRE_PAREN expressao T_FECHA_PAREN
```

**Ações semânticas durante o parsing:**
- `declaracao`: chama `tabela.inserirIdentificador()` para registrar a variável no escopo atual
- `atribuicao`: chama `tabela.existe()` e emite erro semântico se a variável não foi declarada
- `inicio_funcao` (regra auxiliar): atualiza `escopoAtual` para o nome da função antes de processar parâmetros
- Ao finalizar `funcao`: restaura `escopoAtual = "global"`

**Saída do `programa` (regra raiz):**
1. Imprime a AST completa (`raiz->imprimir()`)
2. Gera e imprime o código intermediário (`raiz->gerarCodigo()`)
3. Lista o conteúdo final da tabela de símbolos (`tabela.listarTodos()`)

**Exemplos válidos e inválidos:**
```
✅ int x = 10;
✅ float taxa = 5.5;
✅ int soma(int a, int b) { return a + b; }
✅ int main() { int r = 2 * (3 + 1); return r; }
❌ x = 10;             (variável não declarada)
❌ int x = ;           (falta expressão)
❌ int 1var = 5;       (identificador inválido — erro léxico)
```

---

### 3. Árvore Sintática Abstrata (`src/ast/ast.h` e `src/ast/ast.cpp`)

A AST representa a estrutura hierárquica do programa. Cada nó, além de imprimir sua subárvore, é capaz de gerar código intermediário de 3 endereços.

**Interface base:**
```cpp
class No {
public:
    void imprimir();  // ponto de entrada público
    virtual void imprimirNo(const std::string& pre, bool isLast) = 0;
    virtual std::string gerarCodigo() = 0;
};
```

**Funções auxiliares para impressão:**
```cpp
inline std::string astRamo(bool isLast);   // "L-- " ou "|-- "
inline std::string astIndent(bool isLast); // "    " ou "|   "
```

**Classes de nó:**

| Classe | Campos principais | Exemplo de código-fonte |
|--------|------------------|------------------------|
| `NoNumero` | `int valor` | `10`, `42` |
| `NoFloat` | `float valor` | `3.14`, `5.5` |
| `NoIdentificador` | `std::string nome` | `x`, `resultado` |
| `NoOperacaoBinaria` | `std::string op`, `No* esq`, `No* dir` | `a + b * 2` |
| `NoDeclaracao` | `std::string tipo`, `std::string nome`, `No* valorInicial` | `int x = 5;` |
| `NoAtribuicao` | `std::string nome`, `No* expressao` | `x = a + b;` |
| `NoRetorno` | `No* expressao` | `return r;` |
| `NoBloco` | `std::vector<No*> comandos` | `{ cmd1; cmd2; }` |
| `NoFuncao` | `std::string tipo`, `std::string nome`, `NoBloco* corpo` | `int soma(...) { }` |

**Geração de código intermediário (`gerarCodigo`):**

`NoOperacaoBinaria` usa um contador estático para gerar variáveis temporárias únicas:
```
// Exemplo para: a + b * 2
t1 = b * 2
t2 = a + t1
```

**Exemplo de AST impressa:**
```
Bloco
|-- Declaracao(int x)
|   L-- Numero(10)
L-- Funcao(int, main)
    L-- Bloco
        L-- Retorno
            L-- Numero(0)
```

---

### 4. Análise Semântica (`src/semantica/`)

#### Estrutura `EntradaSimbolo`
```cpp
struct EntradaSimbolo {
    std::string nome;     // nome do identificador
    std::string tipo;     // tipo: "int" ou "float"
    std::string escopo;   // "global" ou nome da função
    int endereco;         // deslocamento de memória (incrementado de 4 em 4)
    int valor;            // valor corrente (usado em simulações)
};
```

#### Classe `TabelaSimbolos`

Internamente usa `std::unordered_map<std::string, EntradaSimbolo>` com chave composta `"escopo::nome"` para isolar escopos.

**Construtor:** registra as palavras reservadas da linguagem:
`INT`, `FLOAT`, `IF`, `ELSE`, `WHILE`, `RETURN`, `VOID`

**Métodos:**

| Método | Descrição | Retorno |
|--------|-----------|---------|
| `inserirIdentificador(nome, tipo, escopo)` | Insere símbolo; detecta redeclaração; aloca endereço | `bool` |
| `existe(nome, escopoAtual)` | Busca no escopo local e depois no global | `bool` |
| `obterTipo(nome, escopoAtual)` | Retorna o tipo com resolução de escopo | `std::string` |
| `obterEndereco(nome, escopoAtual)` | Retorna o endereço alocado ou -1 | `int` |
| `atualizarValor(nome, escopoAtual, valor)` | Atualiza o valor do símbolo | `void` |
| `ehKeyword(palavra)` | Verifica se é palavra reservada | `bool` |
| `listarTodos()` | Imprime a tabela completa formatada | `void` |

**Resolução de escopo:**
```
busca em "funcao::nome"  →  busca em "global::nome"  →  erro semântico
```

---

### 5. Ambiente de Execução (`src/ambiente/`)

Módulo auxiliar (independente do pipeline principal) que simula a execução em tempo de execução com pilha de ativação.

#### Estrutura `Variavel`
```cpp
struct Variavel {
    std::string nome;
    std::string tipo;
    int valor;
    std::string escopo;   // "local" ou "parametro"
};
```

#### Classe `RegistroAtivacao`

Representa o frame de uma chamada de função na pilha.

**Campos:** `nomeFuncao`, `enderecoRetorno`, `valorRetorno`, `linkDinamico` (função chamadora), `variaveis`

**Métodos:** `adicionarParametro()`, `adicionarVariavel()`, `atribuir()`, `obterValor()`, `imprimir()`

#### Classe `PilhaExecucao`

Gerencia a pilha de registros de ativação.

| Método | Descrição |
|--------|-----------|
| `chamarFuncao(nome, endRetorno)` | Cria e empilha novo AR |
| `topo()` | Retorna AR do topo (função em execução) |
| `retornarFuncao()` | Desempilha AR e retorna valor de retorno |
| `buscarVariavel(nome, out)` | Busca variável do topo para a base |
| `vazia()` / `tamanho()` | Estado da pilha |
| `imprimirEstadoPilha()` | Exibe a pilha visualmente |

**Exemplo de saída do módulo:**
```
[CALL] Criando AR para: soma | retorna para: ... | chamado por: main
  === ESTADO DA PILHA (topo -> base) ===
  ^ Top of Stack
  +----------------------------------+
  | AR de: soma
  | Link Dinamico -> main
  | Variaveis:
  |   [parametro] int a = 2
  |   [parametro] int b = 3
  +----------------------------------+
[RETURN] Encerrando AR de: soma | valor de retorno: 5
```

---

## Estrutura de Pastas

```
Compiladores/
│
├── README.md                    # Guia rápido do projeto
├── DOCUMENTACAO.md              # Este arquivo
├── COMO_EXECUTAR.md             # Guia de execução
├── compilar.bat                 # Script de compilação (Windows)
├── SimpleC.exe                  # Executável do compilador gerado
│
├── src/                         # Código-fonte principal
│   ├── lexer/
│   │   ├── lexer.l              # Regras Flex (análise léxica)
│   │   └── lex.yy.c             # Gerado automaticamente pelo Flex
│   │
│   ├── parser/
│   │   ├── parser.y             # Gramática Bison (análise sintática)
│   │   ├── parser.tab.c         # Gerado automaticamente pelo Bison
│   │   └── parser.tab.h         # Headers gerados pelo Bison
│   │
│   ├── ast/
│   │   ├── ast.h                # Declarações das classes AST
│   │   └── ast.cpp              # Implementação das classes AST
│   │
│   ├── semantica/
│   │   ├── tabela_simbolos.h    # Declaração da Tabela de Símbolos
│   │   └── tabela_simbolos.cpp  # Implementação da Tabela de Símbolos
│   │
│   └── ambiente/                # Módulo de ambiente de execução
│       ├── activation_record.h  # Registro de ativação (frame de função)
│       ├── pilha_execucao.h     # Pilha de execução (call stack)
│       └── main_ambiente.cpp    # Demo do ambiente de execução
│
└── exemplos/
    └── teste.sc                 # Código-exemplo para testes
```

---

## Detalhes de Implementação

### Fluxo de Dados

```
┌──────────────────┐
│  Código Fonte    │  exemplos/teste.sc  (entrada: stdin)
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  Lexer (Flex)    │  lexer.l  →  lex.yy.c
│                  │  Gera tokens via yylex()
└────────┬─────────┘
         │ stream de tokens
         ▼
┌──────────────────────────┐
│ Parser (Bison)           │  parser.y  →  parser.tab.c
│                          │  Valida gramática
│                          │  Aciona tabela_simbolos
│                          │  Constrói nós da AST
└────────┬─────────────────┘
         │ NoBloco* (raiz da AST)
         ▼
┌──────────────────────────┐
│ Impressão da AST         │  No::imprimir()
│ + Geração de Código      │  No::gerarCodigo()  →  código 3 endereços
│ + Listagem da Tabela     │  TabelaSimbolos::listarTodos()
└──────────────────────────┘
```

### Integração entre Componentes

**Lexer → Parser:**
- `lexer.l` inclui `parser.tab.h` para usar as constantes de token
- `yylex()` retorna um token de cada vez; `yylval` carrega o valor semântico

**Parser → AST:**
- A cada regra reconhecida, o parser instancia o nó AST correspondente
- O nó é propagado via `$$` (valor semântico do não-terminal)

**Parser → Tabela de Símbolos:**
- `inserirIdentificador()` chamado em `declaracao` e em `lista_parametros`
- `existe()` chamado em `atribuicao` para detectar variáveis não declaradas

**AST → Código Intermediário:**
- `gerarCodigo()` percorre a árvore em pós-ordem
- `NoOperacaoBinaria` cria temporários `t1`, `t2`, ... via contador estático

---

## Fluxo de Compilação

### Passo 1: Gerar Parser com Bison
```bat
win_bison -d src/parser/parser.y -o src/parser/parser.tab.c
```
- `-d`: gera `parser.tab.h` com as definições dos tokens
- Saídas: `parser.tab.c` e `parser.tab.h`

### Passo 2: Gerar Lexer com Flex
```bat
win_flex -o src/lexer/lex.yy.c src/lexer/lexer.l
```
- Saída: `lex.yy.c`

### Passo 3: Compilar com GCC
```bat
g++ src/parser/parser.tab.c src/lexer/lex.yy.c src/ast/ast.cpp src/semantica/tabela_simbolos.cpp -o SimpleC.exe
```
- Combina parser, lexer, AST e tabela de símbolos
- Gera o executável `SimpleC.exe`

### Executar o compilador
```bat
SimpleC.exe < exemplos\teste.sc
```

A saída exibe:
1. A AST em formato de árvore visual
2. O código intermediário de 3 endereços
3. A tabela de símbolos final com escopo, tipo, endereço e valor

---

## Equipe

- **Luiz**
- **Matheus**
- **Vrass**
- **Thalita**

---

## Notas Técnicas

### Dependências Externas
- **GCC / g++**: Compilador C++11 ou superior (testado com MinGW GCC)
- **Flex** (`win_flex`): Gerador de analisadores léxicos
- **Bison** (`win_bison`): Gerador de analisadores sintáticos

### Linguagens Utilizadas
- **C++11**: Classes da AST (`ast.h`/`ast.cpp`) e tabela de símbolos
- **C**: Código gerado pelo Flex e Bison (`lex.yy.c`, `parser.tab.c`)

### Plataforma
- **Windows**: Scripts em `.bat`; ferramentas `win_flex` e `win_bison`

### Funcionalidades Pendentes
- Regras gramaticais para `if/else` e `while` (tokens já reconhecidos pelo lexer)
- Verificação de tipo em expressões (int vs float)
- Geração de código para estruturas de controle
- Otimizações de código intermediário

---

**Documento atualizado em:** Maio de 2026  
**Versão:** 2.0
