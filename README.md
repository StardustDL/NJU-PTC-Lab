# NJU PTC Lab

![CI](https://github.com/StardustDL-Labs/NJU-PTC-Lab/workflows/CI/badge.svg)

A compiler of `C--`, a `C`-like programming language introduced for the compiler lesson in Nanjing University.

- This is the source codes of my programming assignment of Principles and Techniques of Compiler courses (2020 Spring).
- Each push (or pull request) will be tested, goto [Actions](https://github.com/StardustDL-Labs/NJU-PTC-Lab/actions) for details.
- There are still some bugs, but won't be fixed for now.

## Sources

The main workflow is 

```
CMM source -> Lexical -> Syntax -> Semantics -> IR -> Assembly
```

| File                               | Description   |
|------------------------------------|---------------|
| `main.c`                           | Entry point   |
| `lexical.h, lexical.l, _lexical.c` | Lexical       |
| `syntax.h, syntax.y, _syntax.c`    | Syntax        |
| `semantics.h, semantics.c`         | Semantics     |
| `ir.h, ir.c`                       | IR code       |
| `asm.h, asm.c`                     | Assembly code |

There are some helper functions, macros and structs in other files.

| File                     | Description                                             |
|--------------------------|---------------------------------------------------------|
| `common.h`               | Shared header                                           |
| `debug.h`                | Debugging                                               |
| `hash.h, hash.c`         | Hasher                                                  |
| `object.h, object.c`     | Object creating and destroying, wrapper for malloc/free |
| `list.h, list.c`         | Linked-list                                             |
| `type.h, type.c`         | Types in CMM language                                   |
| `symbol.h, symbol.c`     | Symbol and symbol table                                 |
| `ast.h, ast.c`           | Syntax tree and IR code                                 |
| `optimize.h, optimize.c` | Optimizer for IR code                                   |

## Build

Environment:

- OS: Ubuntu 16.04 or 18.04
- Compiler: gcc 7.5
- Flex: 2.6
- Bison: 3.0
- Spim

Instructions:

1. Install dependencies:

```sh
apt-get install bison flex build-essential spim
```

2. Build project:

```sh
cd src && make
```

## Run

### Lexical

Check lexical analysis.

```sh
./src/ncc a.cmm --lexcial
```

### Syntax

Parse source codes into syntax tree.

```sh
./src/ncc a.cmm --syntax
```

### Semantics

Check semantics analysis.

```sh
./src/ncc a.cmm --semantics
```

### IR

Translate source codes to intermediate codes.

```sh
# output to stdout
./src/ncc a.cmm --ir

# output to file
./src/ncc a.cmm a.ir --ir
```

### Assembly

Generate MIPS-32 assembly codes.

```sh
# output to stdout
./src/ncc a.cmm

# output to file
./src/ncc a.cmm a.s
```

## Test

```sh
cd test && ./testall.sh
```

## Thanks & Related projects

- [compilers-tests](https://github.com/massimodong/compilers-tests)
- [ncc-tools](https://github.com/wierton/ncc-tools)
- [MIPS-Instruction-Tools](https://github.com/StardustDL/MIPS-Instruction-Tools)
