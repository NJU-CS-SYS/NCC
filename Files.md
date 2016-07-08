# Files Description

This is the file describing the functions of all the source files tables.

## Source codes

| File name       | Usage                                                   |
|-----------------|---------------------------------------------------------|
| main.c          | main() function.                                        |
| asm.c/h         | Assembly codes generation.                              |
| ast.c/h         | Abstract syntax tree creation.                          |
| basic-block.c/h | Basic code blocks management.                           |
| cmm-strtab.c/h  | A string table used to store string. (Not useful)       |
| cmm-symtab.c/h  | Symbal tables management for NCC.                       |
| cmm-type.c/h    | Definition of basic types and type modification tools.  |
| ir.c/h          | Intercode structure and related tools definition.       |
| lexical.l       | For flex to generate lexical parser.                    |
| lib.c/h         | Definition of inner types and macros.                   |
| node.c/h        | Tree node definition and modification tools.            |
| operand.c/h     | Operand structure definition and modification.          |
| predefine.c/h   | predefined assembly codes.                              |
| register.c/h    | Register definition, modification and allocation.       |
| semantic.c/h    | Semantic analyzation source code.                       |
| syntax.y        | Syntax definition file for bison.                       |
| translate.c/h   | Translate AST into intercode.                           |
| Makefile        | Makefile for this project. Use with gdb/clean/test/execute.|
| test.sh         | Testing shell scipt used by Makefile.                   |

## Documents

| File name       | Usage                                                   |
|-----------------|---------------------------------------------------------|
| README.md       | General description of this project.                    |
| Files.md        | Description of source files in this project. (This file)|
| NCC 文法.md      | Description of syntax using in NCC. (Chinese document)  |

## Test files (./test/)

| File name       | Usage                                                   |
|-----------------|---------------------------------------------------------|
| basic-calc.cmm  | Test basic arithmetic calculation, including overflow.  |
| char-0.cmm      | Test basic char type and char type array.               |
| comment.cmm     | Test comments.                                          |
| easy-for.cmm    | Test for-loop.                                          |
| struct.cmm      | Test struct, struct array and struct as function parameter.|
| global_dec.cmm  | Test global variables.                                  |
