# Clyte: A Simple C Compiler
Clyte is a minimal C compiler designed for educational and research purposes. It implements a basic tokenizer, parser, and code generator for a subset of the C programming language.

# Features
- Lexical analysis and parsing of C source code
- Abstract syntax tree (AST) generation
- Code generation for x86-64 architecture
- Support for integer operations, control flow, and function calls

# Getting Started
To build and run Clyte, ensure you have a working C compiler and standard build tools installed.

```sh
git clone https://github.com/Gadzata/Clyte.git
cd clyte
make
```
# Usage
Once compiled, you can use Clyte to compile a C source file:

```sh
./clyte input.c -o output
```
