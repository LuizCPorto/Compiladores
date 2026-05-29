# Relatório de Execução — SimpleC Compiler

Este documento explica, passo a passo, como cada parte do compilador SimpleC funciona e como elas se conectam para produzir a saída observada ao executar `compilador.exe` com o arquivo `exemplos/teste03.sc`.

---

## Visão da Execução Completa

Ao rodar `compilador.exe`, o programa percorre **5 fases em sequência**:

```
teste03.sc
   │
   ▼
[1] ANÁLISE LÉXICA       → identifica e classifica tokens
   │
   ▼
[2] ANÁLISE SINTÁTICA    → valida gramática e constrói a AST
   │  (integrada com)
[3] ANÁLISE SEMÂNTICA    → preenche a tabela de símbolos
   │
   ▼
[4] GERAÇÃO DE CÓDIGO    → código intermediário de 3 endereços
   │
   ▼
[5] TABELA DE SÍMBOLOS   → lista todos os símbolos coletados
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

O Flex aplica a regra mais longa que casa. Por isso `<=` é reconhecido antes de `<`, e `"int"` é reconhecido antes do padrão de identificadores, porque as palavras-chave são listadas primeiro no arquivo.

### Quando os tokens aparecem na saída

Como o parser consome tokens **um a um** enquanto constrói a árvore, todos os prints do lexer aparecem **antes** da impressão da AST (que só ocorre quando a regra `programa` termina completamente).

### Exemplo

A linha `int global_x = 100;` gera exatamente:

```
PALAVRA-CHAVE   T_INT            int
IDENTIFICADOR   T_ID             global_x
OPERADOR        T_ATRIB          =
NUMERO_INT      T_NUMERO         100
DELIMITADOR     T_PONTOVIRGULA   ;
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

Quando toda a entrada é reconhecida, a regra `programa` executa:

1. Chama `raiz->imprimir()` → imprime a AST
2. Chama `raiz->gerarCodigo()` → imprime o código intermediário
3. Chama `tabela.listarTodos()` → imprime a tabela de símbolos

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

## Fase 4 — Árvore Sintática Abstrata (`src/ast/ast.*`)

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

### Dois métodos em cada nó

Cada nó implementa dois métodos virtuais:

**`imprimirNo(pre, isLast)`** — imprime a árvore com indentação visual usando `|--` e `L--` para mostrar ramificações.

**`gerarCodigo()`** — percorre a árvore em **pós-ordem** e emite código de 3 endereços. Retorna o nome do resultado (uma variável ou temporário).

### Código de 3 endereços

`NoOperacaoBinaria::gerarCodigo()` usa um contador estático para gerar temporários únicos:

```cpp
std::string tmp = "t" + std::to_string(contador_temp++);
std::cout << tmp << " = " << le << " " << op << " " << ld << "\n";
return tmp;
```

Para `a + b * 2`:
1. `b * 2` → gera `t2 = b * 2`, retorna `"t2"`
2. `a + t2` → gera `t3 = a + t2`, retorna `"t3"`
3. `resultado = t3` → `NoDeclaracao` emite `resultado = t3`

---

## Fase 5 — Tabela de Símbolos (saída)

A tabela é impressa pela regra `programa` do parser após a geração do código. Ela mostra o estado final de todos os símbolos coletados durante a análise:

```
Nome        Tipo    Escopo          Endereco   Valor
global_x    int     global          0          100
taxa        float   global          4          5
valor       int     calcula_dobro   8          0
dobro       int     calcula_dobro   12         0
a           int     main            16         10
b           int     main            20         20
resultado   int     main            24         0
```

Cada variável recebe um **endereço de memória virtual** (incrementado de 4 bytes) que representa seu deslocamento em relação ao início da memória de dados. Os valores são os capturados de inicializações literais; expressões compostas ficam como 0 por não terem sido avaliadas em tempo de compilação.

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
            └── regra `programa`: imprime AST + código intermediário + tabela
```

---

## Fluxo de Dados entre Arquivos

| De | Para | O que passa |
|----|------|-------------|
| `lexer.l` | `parser.y` | tokens (constantes inteiras) + `yylval` (valor semântico) |
| `parser.y` | `ast.h/cpp` | instâncias de `No*` (nós da árvore) |
| `parser.y` | `tabela_simbolos` | inserções e consultas por nome/escopo |
| `ast.cpp` | saída padrão | texto da AST e código intermediário de 3 endereços |
| `tabela_simbolos` | saída padrão | tabela formatada com nome, tipo, escopo, endereço e valor |

---

**Versão:** 1.1 — Maio de 2026
