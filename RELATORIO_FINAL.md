# Relatorio final - Mini-Compilador SimpleC

## Introducao e objetivo

O SimpleC e um mini-compilador didatico para demonstrar as etapas centrais de compilacao: analise lexica, analise sintatica, AST, tabela de simbolos, analise semantica, codigo intermediario, otimizacoes, codigo final e ambiente de execucao.

## Definicao da linguagem

A linguagem aceita declaracoes, atribuicoes, expressoes aritmeticas, blocos, funcoes simples e retorno.

Exemplo:

```c
int a;
int b;
int x;
b = 3;
x = a + b * 2;
```

## Tokens, lexer e automatos

O lexer reconhece palavras-chave (`int`, `float`, `void`, `return`, `main`), identificadores, numeros inteiros, numeros float, operadores aritmeticos, atribuicao, parenteses, chaves, virgula e ponto e virgula. A implementacao atual esta em `src/lexer/lex.yy.c` como scanner manual para funcionar mesmo sem `win_flex` instalado.

## Gramatica e parser

O parser aceita uma lista de comandos. A precedencia e tratada por niveis: `expressao` para `+` e `-`, `termo` para `*` e `/`, e `fator` para literais, identificadores e parenteses. A implementacao atual esta em `src/parser/parser.tab.c` como parser manual para funcionar mesmo sem `win_bison` instalado.

## AST

A AST esta em `src/ast/ast.h` e `src/ast/ast.cpp`. Ela representa numeros, floats, identificadores, operacoes binarias, declaracoes, atribuicoes, retornos, blocos e funcoes.

## Tabela de simbolos e analise semantica

A tabela esta em `src/semantica/tabela_simbolos.h` e `src/semantica/tabela_simbolos.cpp`, com declaracoes no `.h` e implementacao no `.cpp`. Ela guarda nome, tipo, escopo, categoria, endereco e valor opcional.

Validacoes implementadas:

- Declaracao duplicada no mesmo escopo.
- Uso de variavel nao declarada.
- Compatibilidade simples de tipos.
- Parametros, variaveis locais, globais e funcoes.
- Retorno de funcao compativel com o tipo declarado.

Exemplo de erro:

```c
x = 10;
```

Saida esperada:

```text
Erro semantico: variavel 'x' nao declarada.
```

## Codigo intermediario

O modulo `src/intermediario` guarda uma lista de instrucoes de tres enderecos. A AST gera instrucoes reais, sem valores fixos.

Entrada:

```c
x = a + b * 2;
```

Saida:

```text
t1 = b * 2
t2 = a + t1
x = t2
```

## Ambiente de execucao

O modulo `src/ambiente` demonstra a pilha de execucao com activation records. Ele evita guardar referencia ao topo da pilha antes de chamar outra funcao.

Saida esperada:

```text
[call] main()
[stack] cria AR de main
[call] soma(a=2,b=3)
[stack] cria AR de soma
[compute] r = a + b = 5
[return] soma = 5
[stack] remove AR de soma
[assign] x = 5
[end] main()
```

## Codigo final

O modulo `src/codigo_final` converte codigo intermediario para pseudo-assembly usando `LOAD`, `ADD`, `SUB`, `MUL`, `DIV`, `STORE` e `RET`. A saida tambem e salva em `saida.asm`.

Exemplo:

```text
LOAD R1, a
ADD R1, b
STORE t1, R1
```

## Otimizacoes

O modulo `src/otimizacao` implementa regras simples:

- Simplificacao algebrica.
- Propagacao de constantes.
- Eliminacao de temporarios mortos.
- Reuso de subexpressoes comuns simples.

O compilador mostra o codigo antes e depois da otimizacao.

## Testes realizados

Comando:

```powershell
.\compilar.bat
Get-Content exemplos\teste.sc | .\SimpleC.exe
.\AmbienteExecucao.exe
```

Casos testados:

- Programa valido com declaracoes e expressao com precedencia.
- Variavel nao declarada.
- Declaracao duplicada.
- Atribuicao de `float` para `int`.
- Demonstracao de `main` chamando `soma`.

## Conclusao

O projeto agora cobre os itens 6 a 14 do plano: tabela de simbolos, analise semantica, intermediario, ambiente de execucao, codigo final, otimizacoes, relatorio e checklist de entrega.
