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

O **SimpleC** é um mini-compilador desenvolvido como trabalho prático para a disciplina de Compiladores da UFT. O projeto implementa um **pipeline completo** de compilação:

- **Análise Léxica**: Tokenização do código-fonte com classificação de categorias (Flex)
- **Análise Sintática**: Verificação gramatical e construção da AST (Bison)
- **Árvore Sintática Abstrata (AST)**: Representação hierárquica do programa
- **Análise Semântica**: Tabela de símbolos com escopo, verificação de declarações e tipos
- **Geração de Código Intermediário**: Código de 3 endereços com variáveis temporárias
- **Otimização Independente de Máquina**: Simplificação algébrica, dobramento de constantes e eliminação de código morto
- **Geração de Código Final (Back-end)**: Assembly x86-64 real em sintaxe AT&T/GAS, salvo em `saida.asm`

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
        ┌────────▼──────────────────────┐
        │  OTIMIZAÇÃO INDEPENDENTE      │
        │  DE MÁQUINA (AST::otimizar)   │
        │  - Simplificação algébrica    │
        │  - Dobramento de constantes   │
        │  - Eliminação de código morto │
        └────────┬──────────────────────┘
                 │
        ┌────────▼──────────────────────┐
        │  GERAÇÃO DE CÓDIGO FINAL      │  ◄── BACK-END
        │  (AST::gerarAssembly)         │
        │  - Instruções x86-64 reais    │
        │  - Frame de pilha (prologue/  │
        │    epilogue)                  │
        │  - Convenção de chamada ABI   │
        │  - Arquivo saida.asm          │
        └────────┬──────────────────────┘
                 │
        ┌────────▼────────┐
        │   saida.asm     │  Assembly compilável com GCC
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
| `T_RETURN` | `return` | Palavra-chave retorno |
| `T_MAIN` | `main` | Palavra-chave função principal |
| `T_NUMERO` | `[0-9]+` | Literal inteiro |
| `T_NUMERO_FLOAT` | `[0-9]+"."[0-9]+` | Literal ponto flutuante |
| `T_ID` | `[a-zA-Z_][a-zA-Z0-9_]*` | Identificador (variáveis/funções) |
| `T_ATRIB` | `=` | Operador de atribuição |
| `T_MAIS` | `+` | Operador adição |
| `T_MENOS` | `-` | Operador subtração |
| `T_MULT` | `*` | Operador multiplicação |
| `T_DIV` | `/` | Operador divisão |
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
- Imprime cada token reconhecido com sua categoria, nome e lexema

**Exemplo de tokenização:**
```
Entrada: int x = 10;
Saída:   T_INT  T_ID("x")  T_ATRIB  T_NUMERO(10)  T_PONTOVIRGULA
```

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
2. Gera e imprime o código intermediário original (`raiz->gerarCodigo()`)
3. Executa as otimizações e exibe as transformações (`raiz->otimizar()`)
4. Gera e imprime o código intermediário otimizado
5. Inicializa o gerador de assembly (`inicializarGeradorAssembly()`) e chama `raiz->gerarAssembly()`
6. Exibe o assembly no console e grava `saida.asm`
7. Lista o conteúdo final da tabela de símbolos (`tabela.listarTodos()`)

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

A AST representa a estrutura hierárquica do programa. Cada nó implementa três responsabilidades: imprimir a subárvore, gerar código intermediário e aplicar otimizações.

**Interface base:**
```cpp
class No {
public:
    void imprimir();
    virtual void imprimirNo(const std::string& pre, bool isLast) = 0;
    virtual std::string gerarCodigo() = 0;
    virtual std::string paraExpressao() const = 0;
    virtual No* otimizar() { return this; }
    virtual void gerarAssembly() {}   // back-end: gera assembly x86-64
};
```

**Funções auxiliares:**
```cpp
inline std::string astRamo(bool isLast);                            // "L-- " ou "|-- "
inline std::string astIndent(bool isLast);                          // "    " ou "|   "
void resetarContadorAST();                                          // reseta contador de temporários
void inicializarGeradorAssembly(std::ostream& saida,               // inicializa o back-end
                                 TabelaSimbolos& tabela);
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

---

### 4. Análise Semântica (`src/semantica/`)

#### Estrutura `EntradaSimbolo`
```cpp
struct EntradaSimbolo {
    std::string nome;     // nome do identificador
    std::string tipo;     // tipo: "int" ou "float"
    std::string escopo;   // "global" ou nome da função
    int endereco;         // deslocamento de memória (incrementado de 4 em 4)
    int valor;            // valor corrente (capturado de literais simples)
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

### 5. Otimização Independente de Máquina (`src/ast/ast.cpp`)

A otimização é aplicada sobre a AST após a geração do código intermediário original, percorrendo a árvore em **pós-ordem** (filhos antes do pai). Três categorias de transformações são realizadas:

#### Simplificação Algébrica

Identidades e propriedades algébricas aplicadas em `NoOperacaoBinaria::otimizar()`:

| Regra | Antes | Depois |
|-------|-------|--------|
| Identidade aditiva | `x + 0` / `0 + x` | `x` |
| Identidade subtrativa | `x - 0` | `x` |
| Cancelamento | `x - x` | `0` |
| Identidade multiplicativa | `x * 1` / `1 * x` | `x` |
| Anulação multiplicativa | `x * 0` / `0 * x` | `0` |
| Identidade divisória | `x / 1` | `x` |

#### Dobramento de Constantes

Expressões com dois operandos literais são avaliadas em tempo de compilação:
```
(3 + 4)  →  7
(2 * 5)  →  10
```

#### Eliminação de Código Morto

Em `NoBloco::otimizar()`, instruções após um `return` são removidas:
```
return x;
int y = 5;   ← removido (código morto)
```

**Exemplo completo:**
```
Entrada:  r = (a + 0) * (c - c) + (1 * (f + g) - (h / 1)) + ((a + b) * (f + g))
Saída:    r = ((f + g) - h) + ((a + b) * (f + g))

Otimizações aplicadas:
  [SIMPL. ALGEBRICA] (a + 0)           ->  a          [x + 0 = x]
  [SIMPL. ALGEBRICA] (c - c)           ->  0          [x - x = 0]
  [SIMPL. ALGEBRICA] (a * 0)           ->  0          [x * 0 = 0]
  [SIMPL. ALGEBRICA] (1 * (f + g))     ->  (f + g)    [1 * x = x]
  [SIMPL. ALGEBRICA] (h / 1)           ->  h          [x / 1 = x]
  [SIMPL. ALGEBRICA] (0 + ((f+g) - h)) ->  (f+g) - h  [0 + x = x]
```

**Impacto no código intermediário:**
- Antes da otimização: **12 temporários** (`t1`…`t12`)
- Após a otimização: **6 temporários** (`t1`…`t6`) — redução de 50%

---

### 6. Geração de Código Final — Back-end (`src/ast/ast.cpp`)

O back-end percorre a AST **já otimizada** e emite instruções assembly x86-64 em sintaxe AT&T/GAS. A saída é capturada em um `std::ostringstream`, exibida no console e gravada em `saida.asm`.

#### Inicialização

```cpp
void inicializarGeradorAssembly(std::ostream& saida, TabelaSimbolos& tabela);
```

Configura o estado estático do gerador antes de chamar `raiz->gerarAssembly()`.

#### Estado interno (variáveis estáticas em `ast.cpp`)

| Variável | Tipo | Papel |
|----------|------|-------|
| `asm_saida` | `std::ostream*` | stream de saída (console ou arquivo) |
| `asm_tabela` | `TabelaSimbolos*` | consulta de endereços e escopos |
| `asm_escopo` | `std::string` | função em geração ou `"global"` |
| `asm_offsets` | `map<string,int>` | `nome → offset` negativo relativo a `%rbp` |
| `asm_ret_label` | `std::string` | label de epílogo (ex: `.Lmain_ret`) |

#### Estratégia de avaliação de expressões

`NoOperacaoBinaria::gerarAssembly()` usa a pilha de hardware como área de rascunho:

```
1. Avalia operando esquerdo  → resultado em %eax
2. pushq %rax                → salva na pilha
3. Avalia operando direito   → resultado em %eax
4. popq %rcx                 → recupera operando esquerdo
5. Aplica operação           → resultado em %eax
```

| Operador | Instrução gerada |
|----------|-----------------|
| `+` | `addl %ecx, %eax` |
| `-` | `subl %eax, %ecx` / `movl %ecx, %eax` |
| `*` | `imull %ecx, %eax` |
| `/` | `xchgl %eax, %ecx` / `cdq` / `idivl %ecx` |

#### Layout do frame de pilha

Variáveis locais são ordenadas pelo campo `endereco` da tabela de símbolos e mapeadas para offsets negativos em relação a `%rbp`:

```
endereco 0  → -4(%rbp)
endereco 4  → -8(%rbp)
endereco 4N → -(4N+4)(%rbp)
```

O tamanho do frame (`subq $N, %rsp`) é arredondado para múltiplo de 16 bytes (requisito do ABI x86-64).

#### Parâmetros formais (System V AMD64 ABI)

Parâmetros são identificados como variáveis do escopo sem `NoDeclaracao` correspondente no corpo. No prólogo, são despejados dos registradores para a pilha:

```asm
movl  %edi, -4(%rbp)   # 1º parâmetro inteiro
movl  %esi, -8(%rbp)   # 2º parâmetro inteiro
# ...até 6 parâmetros (%edi %esi %edx %ecx %r8d %r9d)
```

#### Seções geradas

| Situação | Seção emitida |
|----------|--------------|
| Variáveis globais | `.section .data` com `.long` / `.float` |
| Funções e `main` | `.section .text` com prólogo, corpo e epílogo |

#### Exemplo de saída (trecho de `teste03.sc`)

```asm
    .section    .text

    .globl  main
main:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $32, %rsp           # 7 variavel(is) * 4 bytes
    # int a = 10
    movl    $10, -4(%rbp)
    # int r = (((f + g) - h) + ((a + b) * (f + g)))
    movl    -16(%rbp), %eax     # f
    pushq   %rax
    movl    -20(%rbp), %eax     # g
    popq    %rcx
    addl    %ecx, %eax          # f + g
    ...
    # return r
    movl    -28(%rbp), %eax
    jmp     .Lmain_ret
.Lmain_ret:
    movq    %rbp, %rsp
    popq    %rbp
    ret
```

Para compilar e executar o assembly gerado (Linux/WSL):

```bash
gcc -no-pie saida.asm -o programa
./programa
echo $?   # valor de retorno de main
```

---

### 7. Ambiente de Execução (`src/ambiente/`) *(módulo auxiliar)*

Módulo independente do pipeline principal que implementa as estruturas de pilha de chamadas para fins didáticos.

#### Classe `RegistroAtivacao`
Representa o frame de uma chamada de função. Armazena: `nomeFuncao`, `enderecoRetorno`, `valorRetorno`, `linkDinamico` e mapa de variáveis locais/parâmetros.

#### Classe `PilhaExecucao`
Gerencia a pilha de registros de ativação com operações `chamarFuncao()` / `retornarFuncao()` e busca de variáveis do topo para a base.

---

## Estrutura de Pastas

```
Compiladores/
│
├── README.md                    # Guia rápido do projeto
├── DOCUMENTACAO.md              # Este arquivo
├── COMO_EXECUTAR.md             # Guia de execução
├── relatorioexec.md             # Relatório de execução passo a passo
├── compilar.bat                 # Script de compilação (Windows)
├── compilador.exe               # Executável do compilador gerado
│
├── src/                         # Código-fonte principal
│   ├── lexer/
│   │   ├── lexer.l              # Regras Flex (análise léxica)
│   │   └── lex.yy.c             # Gerado automaticamente pelo Flex
│   │
│   ├── parser/
│   │   ├── parser.y             # Gramática Bison (análise sintática + otimização)
│   │   ├── parser.tab.c         # Gerado automaticamente pelo Bison
│   │   └── parser.tab.h         # Headers gerados pelo Bison
│   │
│   ├── ast/
│   │   ├── ast.h                # Declarações das classes AST
│   │   └── ast.cpp              # Implementação: impressão, código, otimização
│   │
│   ├── semantica/
│   │   ├── tabela_simbolos.h    # Declaração da Tabela de Símbolos
│   │   └── tabela_simbolos.cpp  # Implementação da Tabela de Símbolos
│   │
│   └── ambiente/                # Módulo auxiliar (não integrado ao pipeline)
│       ├── activation_record.h  # Registro de ativação (frame de função)
│       ├── pilha_execucao.h     # Pilha de execução (call stack)
│       └── main_ambiente.cpp    # Ponto de entrada do compilador
│
└── exemplos/
    ├── teste.sc                 # Teste básico (funções e expressões)
    └── teste03.sc               # Teste de otimizações algébricas
```

---

## Detalhes de Implementação

### Fluxo de Dados

```
┌──────────────────┐
│  Código Fonte    │  exemplos/teste03.sc
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  Lexer (Flex)    │  lexer.l  →  lex.yy.c
│                  │  Imprime tokens + retorna constantes via yylex()
└────────┬─────────┘
         │ stream de tokens
         ▼
┌──────────────────────────┐
│ Parser (Bison)           │  parser.y  →  parser.tab.c
│                          │  Valida gramática, aciona tabela_simbolos
│                          │  Constrói nós da AST
└────────┬─────────────────┘
         │ NoBloco* (raiz da AST)
         ▼
┌──────────────────────────┐
│ AST: imprimir()              │  Exibe a árvore hierárquica
│ AST: gerarCodigo()           │  Código intermediário original (t1, t2...)
│ AST: otimizar()              │  Aplica transformações, exibe regras usadas
│ AST: gerarCodigo() x2        │  Código intermediário otimizado (contador reset)
│ AST: gerarAssembly()         │  Assembly x86-64 → console + saida.asm
│ Tabela: listarTodos()        │  Tabela de símbolos final
└──────────────────────────────┘
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

**AST → Otimização:**
- `otimizar()` também percorre em pós-ordem (filhos antes do pai)
- Retorna `this` se o nó foi modificado in-place, ou um nó novo se foi substituído
- O contador de temporários é resetado via `resetarContadorAST()` antes de gerar o código otimizado

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
g++ src/parser/parser.tab.c src/lexer/lex.yy.c src/ast/ast.cpp ^
    src/semantica/tabela_simbolos.cpp src/ambiente/main_ambiente.cpp ^
    -o compilador.exe
```
- Combina parser, lexer, AST, tabela de símbolos e ponto de entrada
- Gera o executável `compilador.exe`

### Executar o compilador
```bat
compilador.exe
```
O arquivo de entrada é definido diretamente em `main_ambiente.cpp` (padrão: `exemplos/teste03.sc`).

**A saída exibe, em sequência:**
1. Análise léxica — tabela de tokens identificados
2. AST — árvore hierárquica do programa
3. Código intermediário original (3 endereços)
4. Otimizações aplicadas (regras e transformações)
5. Código intermediário otimizado
6. **Assembly x86-64** — instruções reais geradas pelo back-end
7. Tabela de símbolos final

O arquivo **`saida.asm`** é gravado automaticamente a cada execução.

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
- Regras gramaticais para `if/else` e `while` no parser (e geração de assembly correspondente com labels de desvio)
- Verificação de compatibilidade de tipos em expressões (`int` vs `float`) com conversão implícita
- Geração de assembly correta para floats (atualmente truncados para inteiro)
- Suporte a chamadas de função no código-fonte (hoje apenas definições são suportadas)

---

**Documento atualizado em:** Junho de 2026
**Versão:** 4.0
