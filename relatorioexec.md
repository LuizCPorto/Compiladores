# Relatório de Execução — SimpleC Compiler

Este documento explica, passo a passo, como cada parte do compilador SimpleC funciona e como elas se conectam para produzir a saída observada ao executar `compilador.exe` com o arquivo `exemplos/teste03.sc`.

---

## Visão da Execução Completa

Ao rodar `compilador.exe`, o programa percorre **6 fases em sequência**:

```
teste03.sc
   │
   ▼
[1] ANÁLISE LÉXICA            → identifica e classifica tokens
   │
   ▼
[2] ANÁLISE SINTÁTICA         → valida gramática e constrói a AST
   │  (integrada com)
[3] ANÁLISE SEMÂNTICA         → preenche a tabela de símbolos
   │
   ▼
[4] GERAÇÃO DE CÓDIGO         → código intermediário de 3 endereços (original)
   │
   ▼
[5] OTIMIZAÇÃO                → simplificação algébrica, dobramento de
   │                            constantes, eliminação de código morto
   ▼
[6] TABELA DE SÍMBOLOS        → lista todos os símbolos coletados
```

---

## Fase 1 — Análise Léxica (`src/lexer/lexer.l`)

### O que acontece

O arquivo `lexer.l` contém as **regras do Flex**. Quando o Bison (parser) precisa do próximo token, ele chama `yylex()`. A cada chamada, o lexer lê caracteres do arquivo de entrada e tenta casar com uma das regras definidas.

### Como funciona internamente

Cada regra no lexer tem a forma:

```
padrão   { ação }
```

Por exemplo:

```
"int"    { lex_print("PALAVRA-CHAVE", "T_INT", yytext); return T_INT; }
[0-9]+   { lex_print("NUMERO_INT", "T_NUMERO", yytext); yylval.valorInteiro = atoi(yytext); return T_NUMERO; }
```

- O padrão é uma **expressão regular** ou string literal.
- A ação imprime o token encontrado e retorna a constante que o parser espera.
- `yytext` contém o lexema (texto original encontrado no arquivo).
- `yylval` é a **união de valores semânticos** — armazena o valor numérico ou o texto do identificador para uso posterior no parser.

### Prioridade de regras

O Flex aplica a regra mais longa que casa. Por isso `"int"` é reconhecido antes do padrão de identificadores, porque as palavras-chave são listadas primeiro no arquivo.

### Quando os tokens aparecem na saída

Como o parser consome tokens **um a um** enquanto constrói a árvore, todos os prints do lexer aparecem **antes** da impressão da AST (que só ocorre quando a regra `programa` termina completamente).

### Exemplo

A linha `int a = 10;` gera exatamente:

```
PALAVRA-CHAVE   T_INT    int
IDENTIFICADOR   T_ID     a
OPERADOR        T_ATRIB  =
NUMERO_INT      T_NUMERO 10
DELIMITADOR     T_PONTOVIRGULA  ;
```

---

## Fase 2 — Análise Sintática (`src/parser/parser.y`)

### O que acontece

O arquivo `parser.y` contém as **regras gramaticais do Bison**. O parser consome os tokens produzidos pelo lexer e verifica se a sequência corresponde à gramática da linguagem SimpleC.

### Como funciona internamente

O Bison usa um algoritmo **LALR(1)** — lê tokens da esquerda para a direita com um símbolo de lookahead. Quando reconhece uma sequência que corresponde ao lado direito de uma regra, executa a **ação semântica** associada (o código C++ entre `{ }`).

Cada regra cria um nó da AST. Exemplo simplificado:

```c
declaracao:
    tipo T_ID T_ATRIB expressao T_PONTOVIRGULA {
        tabela.inserirIdentificador($2, $1, escopoAtual);
        $$ = new NoDeclaracao($1, $2, $4);
    }
```

- `$1`, `$2`, `$4` referenciam os valores semânticos dos símbolos da regra.
- `$$` é o valor semântico que esta regra devolve para a regra acima.
- Ao mesmo tempo que constrói a AST, já chama a tabela de símbolos (`tabela.inserirIdentificador`).

### Gerenciamento de escopo

O parser mantém a variável global de contexto:

```cpp
std::string escopoAtual = "global";
```

Ao encontrar `inicio_funcao` (tipo + nome + `(`), muda `escopoAtual` para o nome da função. Ao fechar a função, restaura para `"global"`. Isso garante que variáveis declaradas dentro de funções sejam registradas no escopo correto.

### Hierarquia de regras e precedência

A gramática separa expressões em três níveis:

```
expressao  →  expressao + termo  |  termo       (menor precedência)
termo      →  termo * fator      |  fator        (maior precedência)
fator      →  NUMERO | ID | ( expressao )
```

Isso faz com que `a + b * 2` seja interpretado corretamente como `a + (b * 2)`, o que pode ser visto na AST:

```
Op (+)
  |-- Id (a)
  L-- Op (*)
        |-- Id (b)
        L-- Numero (2)
```

### Regra `programa` — ponto de convergência

Quando toda a entrada é reconhecida, a regra `programa` executa em sequência:

1. `raiz->imprimir()` → imprime a AST
2. `raiz->gerarCodigo()` → imprime o código intermediário original
3. Salva a expressão de cada comando (para mostrar o before/after)
4. `raiz->otimizar()` → aplica otimizações e imprime as regras usadas
5. `resetarContadorAST()` + `raiz->gerarCodigo()` → imprime o código otimizado
6. `tabela.listarTodos()` → imprime a tabela de símbolos

---

## Fase 3 — Análise Semântica (`src/semantica/tabela_simbolos.*`)

### O que acontece

A análise semântica é realizada **durante** a análise sintática, não em uma fase separada. Conforme o parser reconhece declarações e atribuições, ele consulta e atualiza a `TabelaSimbolos`.

### Como a tabela armazena os dados

Internamente a tabela usa:

```cpp
std::unordered_map<std::string, EntradaSimbolo> tabela;
```

A chave é uma string no formato `"escopo::nome"`, por exemplo:
- `"global::global_x"`
- `"main::a"`
- `"calcula_dobro::valor"`

Cada entrada armazena:

```cpp
struct EntradaSimbolo {
    std::string nome;      // "a"
    std::string tipo;      // "int"
    std::string escopo;    // "main"
    int endereco;          // 16  (offset de memória, +4 por variável)
    int valor;             // 10  (capturado de literais simples)
};
```

### Verificações realizadas

- **Redeclaração**: se `inserirIdentificador` é chamado com uma chave já existente, emite erro semântico.
- **Uso sem declaração**: em `atribuicao`, `tabela.existe()` verifica se a variável foi declarada antes de permitir a atribuição.

### Captura de valores iniciais

Para literais simples (`int a = 10;`), o parser usa `dynamic_cast` para identificar o tipo do nó da expressão e atualiza o valor na tabela:

```cpp
if (NoNumero* n = dynamic_cast<NoNumero*>($4))
    tabela.atualizarValor($2, escopoAtual, n->valor);
```

Isso permite que a tabela de símbolos exiba os valores corretos de variáveis inicializadas com literais.

---

## Fase 4 — Árvore Sintática Abstrata e Código Intermediário (`src/ast/ast.*`)

### O que é a AST

A AST é uma estrutura de árvore em memória que representa o programa de forma hierárquica, eliminando detalhes sintáticos irrelevantes (parênteses, ponto-e-vírgula, etc.).

### Hierarquia de classes

Todas as classes herdam de `No`:

```
No  (classe base abstrata)
├── NoNumero           → literais inteiros:   42
├── NoFloat            → literais float:      3.14
├── NoIdentificador    → variáveis:           x, resultado
├── NoOperacaoBinaria  → operações:           a + b, b * 2
├── NoDeclaracao       → declarações:         int x = 10
├── NoAtribuicao       → atribuições:         x = a + b
├── NoRetorno          → retornos:            return r
├── NoBloco            → sequência de cmds:   { cmd1; cmd2; }
└── NoFuncao           → funções:             int soma(...) { }
```

### Três métodos em cada nó

Cada nó implementa três responsabilidades:

**`imprimirNo(pre, isLast)`** — imprime a árvore com indentação visual usando `|--` e `L--`.

**`gerarCodigo()`** — percorre em **pós-ordem** e emite código de 3 endereços. Retorna o nome do resultado (variável ou temporário).

**`paraExpressao()`** — retorna a expressão como string infixa, usada para mostrar as transformações da otimização.

### Código de 3 endereços

`NoOperacaoBinaria::gerarCodigo()` usa um contador estático para gerar temporários únicos:

```cpp
std::string tmp = "t" + std::to_string(contador_temp++);
std::cout << tmp << " = " << le << " " << op << " " << ld << "\n";
return tmp;
```

Para `a + b * 2`:
1. `b * 2` → gera `t1 = b * 2`, retorna `"t1"`
2. `a + t1` → gera `t2 = a + t1`, retorna `"t2"`

---

## Fase 5 — Otimização Independente de Máquina (`src/ast/ast.cpp`)

### O que acontece

Após a geração do código intermediário original, a AST é percorrida novamente pelo método `otimizar()`. A transformação é feita **diretamente nos nós da árvore**, sem criar uma estrutura separada.

### Estratégia bottom-up (pós-ordem)

`NoOperacaoBinaria::otimizar()` primeiro otimiza os filhos e só então analisa o nó atual:

```cpp
No* novoEsq = esq->otimizar();
if (novoEsq != esq) { delete esq; esq = novoEsq; }

No* novoDir = dir->otimizar();
if (novoDir != dir) { delete dir; dir = novoDir; }

// Só agora verifica se o nó atual pode ser simplificado
```

Isso garante que reduções em cascata funcionem corretamente. Por exemplo, `(a + 0) * (c - c)`:
1. `(a + 0)` é otimizado para `a`
2. `(c - c)` é otimizado para `0`
3. O pai `(a * 0)` é otimizado para `0`

### Gerenciamento de memória

Quando `otimizar()` retorna um nó diferente de `this`, o chamador é responsável por deletar o original:

```cpp
No* r = esq;      // nó a ser retornado
esq = nullptr;    // evita que o destrutor de 'this' delete r
delete dir;       // descarta o operando que foi eliminado
return r;         // chamador vai deletar 'this'
```

### As três categorias

**1. Simplificação algébrica** — identidades aplicadas em `NoOperacaoBinaria`:

| Regra | Transformação |
|-------|--------------|
| `x + 0 = x` | `(a + 0)` → `a` |
| `0 + x = x` | `(0 + a)` → `a` |
| `x - 0 = x` | `(a - 0)` → `a` |
| `x - x = 0` | `(c - c)` → `0` |
| `x * 1 = x` | `(1 * expr)` → `expr` |
| `x * 0 = 0` | `(a * 0)` → `0` |
| `x / 1 = x` | `(h / 1)` → `h` |

**2. Dobramento de constantes** — quando ambos os operandos são `NoNumero`, a operação é avaliada agora:

```
(3 + 4)  →  7
(2 * 5)  →  10
```

**3. Eliminação de código morto** — em `NoBloco::otimizar()`, comandos após um `return` são removidos e deletados:

```cpp
if (dynamic_cast<NoRetorno*>(otim)) morto = true;
// próximas iterações: delete cmd sem processar
```

### Saída da fase de otimização

```
=== OTIMIZACAO INDEPENDENTE DE MAQUINA ===
(Simplificacao algebrica | Dobramento de constantes | Eliminacao de codigo morto)

  [SIMPL. ALGEBRICA] (a + 0)             ->  a          [x + 0 = x]
  [SIMPL. ALGEBRICA] (c - c)             ->  0          [x - x = 0]
  [SIMPL. ALGEBRICA] (a * 0)             ->  0          [x * 0 = 0]
  [SIMPL. ALGEBRICA] (1 * (f + g))       ->  (f + g)    [1 * x = x]
  [SIMPL. ALGEBRICA] (h / 1)             ->  h          [x / 1 = x]
  [SIMPL. ALGEBRICA] (0 + ((f+g) - h))   ->  (f+g) - h  [0 + x = x]

Transformacoes por instrucao:
  Antes:  int r = ((a + 0) * (c - c) + (1 * (f + g) - (h / 1))) + ((a + b) * (f + g))
  Depois: int r = (((f + g) - h) + ((a + b) * (f + g)))
```

O contador de temporários é resetado para `t1` antes de gerar o código otimizado, garantindo numeração limpa.

---

## Fase 6 — Tabela de Símbolos (saída)

A tabela é impressa pela regra `programa` do parser após a geração do código otimizado. Ela mostra o estado final de todos os símbolos coletados durante a análise:

```
Nome        Tipo    Escopo   Endereco   Valor
a           int     main     0          10
b           int     main     4          20
c           int     main     8          5
f           int     main     12         3
g           int     main     16         4
h           int     main     20         8
r           int     main     24         0
```

Cada variável recebe um **endereço de memória virtual** (incrementado de 4 bytes). Os valores são os capturados de inicializações literais; expressões compostas ficam como 0 por não terem sido avaliadas em tempo de compilação.

---

## Como as Partes se Conectam

```
main_ambiente.cpp
    │
    ├── define: TabelaSimbolos tabela  ← compartilhada com o parser
    │
    ├── abre teste03.sc → yyin
    │
    └── chama yyparse()
            │
            ├── yyparse chama yylex() repetidamente
            │       └── lexer.l: imprime token, retorna constante + yylval
            │
            ├── parser.y: reconhece regras, cria nós da AST
            │       └── chama tabela.inserirIdentificador() / existe()
            │
            └── regra `programa`:
                    ├── raiz->imprimir()          → AST
                    ├── raiz->gerarCodigo()        → código original
                    ├── raiz->otimizar()           → transforma AST, imprime regras
                    ├── resetarContadorAST()
                    ├── raiz->gerarCodigo()        → código otimizado
                    └── tabela.listarTodos()       → tabela de símbolos
```

---

## Fluxo de Dados entre Arquivos

| De | Para | O que passa |
|----|------|-------------|
| `lexer.l` | `parser.y` | tokens (constantes inteiras) + `yylval` (valor semântico) |
| `parser.y` | `ast.h/cpp` | instâncias de `No*` (nós da árvore) |
| `parser.y` | `tabela_simbolos` | inserções e consultas por nome/escopo |
| `ast.cpp` (`gerarCodigo`) | saída padrão | código intermediário de 3 endereços |
| `ast.cpp` (`otimizar`) | saída padrão | mensagens de regras aplicadas |
| `ast.cpp` (`paraExpressao`) | `parser.y` | strings de expressão para comparação before/after |
| `tabela_simbolos` | saída padrão | tabela formatada com nome, tipo, escopo, endereço e valor |

---

**Versão:** 1.2 — Maio de 2026
