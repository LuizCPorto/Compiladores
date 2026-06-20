# Relatório de Execução — SimpleC Compiler

Este documento explica, passo a passo, como cada parte do compilador SimpleC funciona e como elas se conectam para produzir a saída observada ao executar `compilador.exe` com o arquivo `exemplos/teste03.sc`.

---

## Visão da Execução Completa

Ao rodar `compilador.exe`, o programa percorre **7 fases em sequência**:

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
[6] GERAÇÃO DE ASSEMBLY       → instruções x86-64 reais (AT&T/GAS)
   │                            gravadas em saida.asm
   ▼
[7] TABELA DE SÍMBOLOS        → lista todos os símbolos coletados
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
6. `inicializarGeradorAssembly()` + `raiz->gerarAssembly()` → gera assembly x86-64 e salva em `saida.asm`
7. `tabela.listarTodos()` → imprime a tabela de símbolos

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

## Fase 6 — Geração de Código Assembly (`src/ast/ast.cpp`)

### O que acontece

Após a otimização, a AST é percorrida uma última vez pelo método `gerarAssembly()`, que produz instruções x86-64 reais em sintaxe AT&T/GAS. O resultado é exibido no console **e** gravado em `saida.asm`.

### Estratégia de geração

O gerador segue um modelo **baseado em pilha**: o operando esquerdo de cada operação binária é empurrado com `pushq %rax` antes de avaliar o operando direito, depois recuperado com `popq %rcx`. Isso garante que expressões arbitrariamente aninhadas sejam avaliadas corretamente sem registradores intermediários dedicados.

**Função `inicializarGeradorAssembly(saida, tabela)`** configura o estado global do gerador:

| Estado estático | Significado |
|----------------|-------------|
| `asm_saida` | stream de saída (console ou arquivo) |
| `asm_tabela` | referência à tabela de símbolos |
| `asm_escopo` | nome da função atual ou `"global"` |
| `asm_offsets` | mapa `nome → offset` de cada variável na pilha |
| `asm_ret_label` | label de retorno da função atual (ex: `.Lmain_ret`) |

### Estrutura de um frame de função

Todas as variáveis locais vivem na pilha, abaixo do frame pointer `%rbp`:

```
┌─────────────────────┐  ← endereço maior
│  ...chamador...     │
│  endereço de retorno│  (empurrado por call)
│  %rbp salvo         │  ← pushq %rbp
├─────────────────────┤  ← %rbp  (após movq %rsp, %rbp)
│  var 1  (int a)     │  -4(%rbp)
│  var 2  (int b)     │  -8(%rbp)
│  var N  (int r)     │  -4*N(%rbp)
├─────────────────────┤  ← %rsp  (após subq $frame, %rsp)
│  zona de expressões │  (pushq/popq temporários)
└─────────────────────┘  ← endereço menor
```

O tamanho do frame é `N × 4` bytes arredondado para múltiplo de 16 (requisito ABI).

### Offsets de variáveis

As variáveis são ordenadas pelo campo `endereco` da tabela de símbolos (ordem de declaração) e mapeadas sequencialmente: primeira variável em `-4(%rbp)`, segunda em `-8(%rbp)`, e assim por diante.

### Parâmetros formais (System V AMD64 ABI)

Parâmetros são identificados automaticamente como variáveis do escopo que **não possuem** um `NoDeclaracao` correspondente no corpo da função. No prólogo, eles são despejados dos registradores (`%edi`, `%esi`, `%edx`, `%ecx`, `%r8d`, `%r9d`) para a pilha:

```asm
movl  %edi, -4(%rbp)   # parametro: valor
```

### Retorno de funções

Toda instrução `return expr` avalia a expressão em `%eax` (registrador de retorno) e salta para o label de epílogo da função, onde o frame é desmontado:

```asm
movl  -28(%rbp), %eax   # carrega valor de retorno
jmp   .Lmain_ret

.Lmain_ret:
    movq  %rbp, %rsp
    popq  %rbp
    ret
```

### Exemplo de saída — `teste03.sc`

```asm
# Gerado pelo Compilador SimpleC
# Sintaxe: AT&T/GAS  |  Target: x86-64 Linux
# Compilar: gcc -no-pie saida.asm -o programa

    .section    .text

    .globl  main
main:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $32, %rsp           # 7 variavel(is) * 4 bytes
    # int a = 10
    movl    $10, -4(%rbp)
    # int b = 20
    movl    $20, -8(%rbp)
    # ...demais declarações...
    # int r = (((f + g) - h) + ((a + b) * (f + g)))  ← expressão já otimizada
    movl    -16(%rbp), %eax     # f
    pushq   %rax
    movl    -20(%rbp), %eax     # g
    popq    %rcx
    addl    %ecx, %eax          # f + g
    pushq   %rax
    movl    -24(%rbp), %eax     # h
    popq    %rcx
    subl    %eax, %ecx          # (f+g) - h
    movl    %ecx, %eax
    pushq   %rax
    # ... (a+b)*(f+g) ...
    addl    %ecx, %eax          # resultado final em %eax
    movl    %eax, -28(%rbp)     # armazena em r
    # return r
    movl    -28(%rbp), %eax
    jmp     .Lmain_ret
.Lmain_ret:
    movq    %rbp, %rsp
    popq    %rbp
    ret
```

O `saida.asm` é compilável diretamente com GCC em Linux/WSL:
```bash
gcc -no-pie saida.asm -o programa && ./programa && echo $?
```

---

## Fase 7 — Tabela de Símbolos (saída)

A tabela é impressa pela regra `programa` do parser após a geração do assembly. Ela mostra o estado final de todos os símbolos coletados durante a análise:

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

O campo `Endereco` é um offset crescente (de 4 em 4 bytes) atribuído na ordem de declaração. O gerador de assembly usa esse campo para ordenar as variáveis no frame de pilha: a variável com menor endereço ocupa `-4(%rbp)`, a próxima `-8(%rbp)`, etc.

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
                    ├── raiz->imprimir()                    → AST
                    ├── raiz->gerarCodigo()                 → código intermediário original
                    ├── raiz->otimizar()                    → transforma AST, imprime regras
                    ├── resetarContadorAST()
                    ├── raiz->gerarCodigo()                 → código intermediário otimizado
                    ├── inicializarGeradorAssembly(buf, tab)
                    ├── raiz->gerarAssembly()               → assembly x86-64 em buffer
                    ├── imprime buffer no console
                    ├── grava saida.asm
                    └── tabela.listarTodos()                → tabela de símbolos
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
| `ast.cpp` (`gerarAssembly`) | `ostringstream` → console + `saida.asm` | instruções assembly x86-64 |
| `tabela_simbolos` | `ast.cpp` (`gerarAssembly`) | endereços e escopos para cálculo de offsets na pilha |
| `tabela_simbolos` | saída padrão | tabela formatada com nome, tipo, escopo, endereço e valor |

---

## Aplicações Relacionadas de Compiladores

As técnicas implementadas neste projeto — análise léxica e sintática, geração de IR, otimização e geração de código de máquina — estão na base de praticamente todo software moderno. As seções abaixo mostram onde esses mesmos conceitos aparecem em contextos reais.

---

### Compiladores JIT — Java, .NET e Python

Um compilador **JIT (Just-In-Time)** não traduz o programa antes de executá-lo: ele compila partes do código *durante* a execução, no momento em que elas são necessárias.

#### Java — HotSpot JVM

O código Java é primeiro compilado para **bytecode** (um IR portável, equivalente ao TAC do SimpleC) pela ferramenta `javac`. Na execução, a JVM monitora quais trechos são executados com frequência ("hot spots") e os compila para código nativo x86/ARM em tempo real, aplicando otimizações agressivas:

```
Código Java (.java)
       ↓ javac (front-end)
   Bytecode (.class)          ← IR portável (como o TAC do SimpleC)
       ↓ JIT HotSpot (middle + back-end, em tempo de execução)
  Código nativo x86/ARM
```

As otimizações do HotSpot incluem **inlining de métodos**, **eliminação de código morto** e **especulação de tipo** — versões sofisticadas do que `otimizar()` faz no SimpleC.

#### .NET — CLR e RyuJIT

O ecossistema .NET (C#, F#, VB) segue a mesma arquitetura: o compilador Roslyn gera **CIL (Common Intermediate Language)**, e o **RyuJIT** converte CIL para código nativo no momento da execução. O CIL é um IR tipado e verificável, mais rico que o TAC de 3 endereços, mas serve o mesmo propósito de separar o front-end (específico da linguagem) do back-end (específico da máquina).

#### Python — CPython e PyPy

O CPython (implementação oficial) compila o código Python para **bytecode `.pyc`** e o executa em uma máquina virtual. Não há JIT por padrão — o que o torna mais lento para código intensivo em CPU.

O **PyPy** resolve isso adicionando um JIT sobre o bytecode Python: detecta laços quentes e os compila para código nativo, alcançando velocidades próximas ao C em muitos benchmarks. A diferença central em relação ao SimpleC é que o PyPy opera sobre um IR dinâmico (com tipos desconhecidos em tempo de compilação), exigindo técnicas como **guards** e **deoptimização**.

---

### Compiladores para GPU

GPUs possuem centenas ou milhares de núcleos paralelos e uma arquitetura radicalmente diferente de CPUs x86. Compilar para GPU exige um back-end especializado que substitui o `gerarAssembly()` do SimpleC por um gerador de código para arquiteturas como **NVIDIA PTX** ou **AMD GCN**.

#### CUDA (NVIDIA)

O compilador **NVCC** aceita código C++ anotado com extensões CUDA (`__global__`, `__device__`) e separa o programa em duas partes:

```
Código CUDA (.cu)
       ↓ NVCC (front-end compartilhado)
       ├── código CPU  →  back-end x86  →  binário do host
       └── código GPU  →  back-end PTX  →  PTX (IR da GPU)
                                               ↓ driver NVIDIA (JIT em PTX)
                                          código nativo da GPU
```

O **PTX** é um IR de baixo nível (semelhante ao TAC do SimpleC, mas com instruções paralelas), compilado pelo driver da GPU para o hardware real no momento de instalação. Isso permite que o mesmo código PTX rode em GPUs de gerações diferentes.

#### OpenCL e SPIR-V

O **OpenCL** usa o **SPIR-V** como IR portável para GPUs de qualquer fabricante (NVIDIA, AMD, Intel). O SPIR-V é um bytecode binário estruturado, compilado em tempo de execução pelo driver para a GPU disponível — a mesma ideia do bytecode Java, mas voltada para paralelismo massivo.

As otimizações específicas de GPU substituem as simplificações algébricas do SimpleC por técnicas como **coalescência de acessos à memória**, **unrolling de laços** e **vetorização SIMD**.

---

### Compiladores para Sistemas Embarcados e IoT

Sistemas embarcados (microcontroladores, sensores, dispositivos IoT) impõem restrições severas que tornam o back-end do compilador crítico:

| Recurso | Computador desktop | Sistema embarcado típico |
|---|---|---|
| RAM | 8–64 GB | 2 KB – 512 KB |
| Flash/ROM | centenas de GB | 16 KB – 4 MB |
| Consumo | 65–300 W | 1 mW – 1 W |
| SO | Windows/Linux | bare-metal ou RTOS |

#### GCC e LLVM para ARM/AVR/RISC-V

O **GCC** e o **LLVM/Clang** são os compiladores mais usados para embarcados. Eles aplicam no back-end otimizações que o SimpleC ainda não possui, mas que seguem os mesmos princípios:

- **Eliminação de código morto** — remove funções nunca chamadas (equivalente ao `otimizar()` do SimpleC, mas aplicado globalmente entre arquivos via LTO — *Link-Time Optimization*)
- **Inlining** — substitui chamadas de função pelo corpo da função, eliminando o custo do frame de pilha (`pushq %rbp` / `popq %rbp`)
- **Alocação de registradores** — ao contrário do SimpleC, que usa a pilha para todos os temporários, compiladores reais alocam variáveis em registradores (`%eax`, `%ecx`...) para evitar acessos lentos à memória
- **Peephole optimization** — substitui sequências de instruções por equivalentes mais curtas ou rápidas

#### TinyML e compiladores de redes neurais

Uma frente emergente no IoT é executar modelos de aprendizado de máquina em microcontroladores. Ferramentas como **TensorFlow Lite Micro** e **Apache TVM** compilam modelos de redes neurais para código C ou assembly otimizado para dispositivos sem SO — aplicando as mesmas etapas do SimpleC (IR → otimização → código de máquina), mas sobre grafos de operações matriciais em vez de expressões aritméticas simples.

---

### Conexão com o SimpleC

| Conceito do SimpleC | Equivalente no mundo real |
|---|---|
| `lexer.l` + `parser.y` | Front-end do GCC (`cc1`), Clang (`clang -cc1`), Roslyn |
| `gerarCodigo()` → TAC | Bytecode Java/CIL/.NET, LLVM IR, GCC GIMPLE |
| `otimizar()` | Passes de otimização do LLVM (`-O2`, `-O3`) |
| `gerarAssembly()` → `saida.asm` | Back-end x86 do GCC, PTX do NVCC, Thumb2 do ARM GCC |
| `saida.asm` | `.ptx` (GPU), `.elf` (embarcado), `.class` (JVM) |

O SimpleC percorre o mesmo caminho que todos esses compiladores. A diferença é escala e quantidade de otimizações — os princípios de front/middle/back-end, IR e geração de código são exatamente os mesmos.

---

**Versão:** 1.3 — Junho de 2026
