# 🚀 Guia de Execução - SimpleC Compiler

## 📋 Índice

1. [Pré-requisitos](#pré-requisitos)
2. [Instalação do Ambiente](#instalação-do-ambiente)
3. [Compilação do Projeto](#compilação-do-projeto)
4. [Execução](#execução)
5. [Testes](#testes)
6. [Resolução de Problemas](#resolução-de-problemas)

---

## ✅ Pré-requisitos

Antes de executar o projeto, você precisa ter instalado:

- ✔️ **GCC** (Compilador C/C++)
- ✔️ **Flex** (win_flex no Windows)
- ✔️ **Bison** (win_bison no Windows)
- ✔️ **PowerShell** ou **CMD** (Terminal do Windows)

---

## 🛠️ Instalação do Ambiente

### Opção 1: Usando Chocolatey (Recomendado)

O Chocolatey é um gerenciador de pacotes para Windows que facilita a instalação.

#### Passo 1: Instalar o Chocolatey
1. Abra o **PowerShell como Administrador**
2. Execute o comando:
   ```powershell
   Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
   ```
3. Aguarde a conclusão

#### Passo 2: Instalar MinGW (GCC)
```powershell
choco install mingw -y
```

#### Passo 3: Instalar Flex e Bison
```powershell
choco install winflexbison3 -y
```

#### Passo 4: Verificar Instalação
```powershell
gcc --version
win_flex --version
win_bison --version
```

Você deverá ver números de versão sem erros.

---

### Opção 2: Instalação Manual

Se preferir não usar Chocolatey:

1. **MinGW**: Baixe em https://www.mingw-w64.org/ e instale
2. **Flex e Bison**: Baixe em http://gnuwin32.sourceforge.net/ e instale
3. **Adicionar ao PATH**: Adicione as pastas de instalação ao PATH do Windows

---

## 🔨 Compilação do Projeto

### Método 1: Usando o Script Automático (Recomendado)

No terminal, na raiz do projeto (`C:\Users\carli\OneDrive\Desktop\Compiladores`), execute:

```powershell
.\compilar.bat
```

**O que o script faz:**
1. 🔹 Executa o Bison para gerar `parser.tab.c` e `parser.tab.h`
2. 🔹 Executa o Flex para gerar `lex.yy.c`
3. 🔹 Compila tudo com GCC gerando `SimpleC.exe` (ou `compilador.exe`)

**Saída esperada:**
```
========================================
Inciando a compilacao da SimpleC...
========================================
[1/3] Gerando o Parser (Bison)...
[2/3] Gerando o Lexer (Flex)...
[3/3] Compilando o executavel com GCC...
========================================
Sucesso! O compilador "SimpleC.exe" foi gerado.
========================================
```

### Método 2: Compilação Manual (Passo a Passo)

Se quiser ver cada etapa:

```powershell
# Etapa 1: Gerar Parser
win_bison -d src/parser/parser.y -o src/parser/parser.tab.c

# Etapa 2: Gerar Lexer
win_flex -o src/lexer/lex.yy.c src/lexer/lexer.l

# Etapa 3: Compilar
g++ src/parser/parser.tab.c src/lexer/lex.yy.c src/ast/ast.cpp -o compilador.exe
```

---

## ▶️ Execução

Após compilar com sucesso, você verá um arquivo `compilador.exe` na raiz do projeto.

### Opção 1: Modo Interativo

Execute o compilador sem argumentos:

```powershell
.\compilador.exe
```

O compilador ficará aguardando entrada. Digite seu código:

```
INT x = 10;
```

Pressione **Enter** e você verá:

```
SUCESSO: A declaracao da variavel foi compreendida pelo Parser!
```

**Para sair do modo interativo:**
- Pressione **Ctrl + D** (EOF - End of File)
- Ou **Ctrl + C** (Interromper)

---

### Opção 2: Lendo de um Arquivo de Teste

Crie um arquivo de teste ou use o exemplo fornecido:

```powershell
.\compilador.exe < exemplos\teste.sc
```

**Exemplo de arquivo (`exemplos/teste.sc`):**
```
INT x = 10;
INT y = 20;
INT z = 30;
```

---

## 🧪 Testes

### Teste 1: Declaração Simples

**Entrada:**
```
INT variavel = 42;
```

**Saída esperada:**
```
SUCESSO: A declaracao da variavel foi compreendida pelo Parser!
```

---

### Teste 2: Múltiplas Declarações

**Arquivo `exemplos/teste.sc`:**
```
INT a = 1;
INT b = 2;
```

**Execução:**
```powershell
.\compilador.exe < exemplos\teste.sc
```

**Saída esperada:**
```
SUCESSO: A declaracao da variavel foi compreendida pelo Parser!
SUCESSO: A declaracao da variavel foi compreendida pelo Parser!
```

---

### Teste 3: Erro de Sintaxe

**Entrada:**
```
INT x =;
```

**Saída esperada:**
```
Error, entrada não reconhecida: syntax error
```

---

### Teste 4: Erro Léxico

**Entrada:**
```
INT x = @;
```

**Saída esperada:**
```
Erro lexico
```

---

## 🐛 Resolução de Problemas

### ❌ Erro: "comando não encontrado: win_bison"

**Causa:** Flex/Bison não está instalado ou não está no PATH

**Solução:**
```powershell
choco install winflexbison3 -y
```

Se ainda não funcionar, adicione manualmente ao PATH:
- Vá para **Variáveis de Ambiente** do Windows
- Adicione o caminho de instalação (geralmente `C:\Program Files\GnuWin32\bin`)

---

### ❌ Erro: "comando não encontrado: gcc"

**Causa:** GCC não está instalado ou não está no PATH

**Solução:**
```powershell
choco install mingw -y
```

---

### ❌ Erro: "Arquivo não encontrado"

**Causa:** Você não está na pasta correta

**Solução:**
```powershell
cd C:\Users\carli\OneDrive\Desktop\Compiladores
ls  # Verifique se vê compilar.bat
.\compilar.bat
```

---

### ❌ Erro: "Access denied" ao executar compilar.bat

**Causa:** Política de execução de scripts restrita

**Solução:**
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

Depois tente novamente:
```powershell
.\compilar.bat
```

---

### ❌ Compilador.exe não foi criado

**Causa:** Erro na compilação

**Solução:**
1. Verifique se há mensagens de erro na tela
2. Verifique se todos os arquivos `.l`, `.y`, `.cpp`, `.h` existem
3. Execute a compilação manual para ver erros específicos:
   ```powershell
   win_bison -d src/parser/parser.y -o src/parser/parser.tab.c
   ```

---

### ⚠️ Aviso: "Syntax error" mesmo em código correto

**Causa:** Possível problema na definição da gramática

**Solução:**
1. Verifique se o código segue exatamente o padrão: `INT variavel = numero;`
2. Certifique-se de que há espaços entre tokens
3. Recompile o projeto:
   ```powershell
   .\compilar.bat
   ```

---

## 📝 Dicas Úteis

### Atalho: Recompilar Sempre

Se estiver fazendo alterações no `lexer.l` ou `parser.y`, execute frequentemente:

```powershell
.\compilar.bat
```

### Testar Rapidamente

Crie um arquivo de teste na pasta `exemplos` e execute:

```powershell
.\compilador.exe < exemplos\seu_teste.sc
```

### Ver Tokens Gerados (Debug)

Para debug, você pode modificar o `lexer.l` para mostrar tokens:

```lex
"INT"       { printf("TOKEN: T_INT\n"); return T_INT; }
```

### Limpar Arquivos Gerados

Se quiser limpar os arquivos gerados pelo Flex e Bison:

```powershell
Remove-Item src/lexer/lex.yy.c
Remove-Item src/parser/parser.tab.c
Remove-Item src/parser/parser.tab.h
```

---

## 📊 Fluxo Completo de Uso

```
1. Abrir PowerShell na pasta do projeto
   └─ cd C:\Users\carli\OneDrive\Desktop\Compiladores

2. Compilar o projeto
   └─ .\compilar.bat

3. Executar no modo interativo
   └─ .\compilador.exe
   └─ Digitar: INT x = 10;
   └─ Pressionar: Enter

4. Ou executar um arquivo de teste
   └─ .\compilador.exe < exemplos\teste.sc

5. Ver resultado na tela
   └─ SUCESSO: A declaracao da variavel foi compreendida pelo Parser!
```

---

## 📞 Suporte

Se encontrar problemas:

1. ✅ Verifique se todas as ferramentas estão instaladas: `gcc --version`, `win_flex --version`, `win_bison --version`
2. ✅ Confirme que está na pasta correta
3. ✅ Limpe e recompile: Delete `lex.yy.c`, `parser.tab.c`, `parser.tab.h` e execute `.\compilar.bat` novamente
4. ✅ Consulte o arquivo `README.md` para informações adicionais

---

**Última atualização:** Maio de 2026  
**Versão:** 1.0  
**Compatibilidade:** Windows 10/11 com PowerShell
