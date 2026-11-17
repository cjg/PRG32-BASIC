# PRG32-BASIC

## Overview
**PRG32-BASIC** is a structured, line-number-free BASIC dialect for the **PRG32** RISC-V platform.

* **Execution Model:** Direct AST (Abstract Syntax Tree) interpreter written in C99/C11.
* **Memory Model:** Zero dynamic heap allocation (no `malloc` pr `free`). All AST nodes, variables, and dynamic strings reside in statically allocated buffers.
* **Frame Synchronization:** Pure sequential top-to-bottom script execution with automatic phase switching between PRG32's `update()` and `draw()` callbacks.

## Example

### Hello World

```bas
PRINT "HELLO WORLD"
```
