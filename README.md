# 🚀 Trabalho de Compiladores

Bem-vindos ao repositório do nosso mini-compilador, desenvolvido como trabalho prático para a disciplina de Compiladores (UFT). 

Este projeto utiliza a clássica abordagem de separação entre **Análise Léxica** (usando Flex) e **Análise Sintática** (usando Bison) em C/C++.

**Equipe:** Luiz, Matheus, Vrass e Talita.

---

## 📁 Estrutura do Projeto

Para manter a organização e separar as responsabilidades, o nosso projeto segue a seguinte arquitetura de pastas:

```text
/
├── src/
│   ├── lexer/
│   │   └── lexer.l      # Regras do Analisador Léxico (Tokens e Regex)
│   └── parser/
│       └── parser.y     # Regras do Analisador Sintático (Gramática EBNF)
├── exemplos/
│   └── codigo.txt       # Arquivos com código-fonte para testes
├── compilar.bat         # Script de automação que compila o projeto
└── README.md            # Este arquivo

```
## ⚙️ Pré-requisitos (Configurando o Ambiente)

Para que o código compile na sua máquina, você precisa do compilador C/C++ (GCC) e das ferramentas Flex e Bison. A forma mais fácil de instalar tudo no Windows é usando o **Chocolatey**.

**Passo a passo da instalação:**
1. Abra o **PowerShell como Administrador**.
2. Instale o compilador C/C++ (MinGW) rodando:
   ```powershell
   choco install mingw -y
  
3. Instale o win_flex rodando: 
    ```powershell
    choco install winflexbison3 -y
    ```

## 🔨 Como Compilar o Projeto
Nós automatizamos o processo de gerar os arquivos .c e .h e juntar tudo. Sempre que você alterar algo no lexer.l ou no parser.y, basta rodar o nosso script.

No terminal do VS Code (na raiz do projeto), digite:
   ```powershell
    .\compilar.bat
   ```

## 🚀 Como Executar e Testar
Com o compilador.exe gerado, temos duas formas de testar o funcionamento:

Opção 1: Modo Interativo (Terminal)
Rodando o executável diretamente, ele fica aguardando você digitar o código.
   ```powershell
    .\compilador.exe
   ```
Exemplo de uso: Digite INT x = 10; e aperte Enter.

Opção 2: Lendo de um arquivo de testes
A melhor forma de testar blocos maiores de código é escrever tudo no arquivo exemplos/codigo.txt e mandar o compilador ler ele de uma vez.

   ```powershell
    .\compilador.exe < exemplos\codigo.txt
   ```
