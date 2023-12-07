# Custom C Memory Allocation Functions

## Overview

This repository contains my implementation of various memory allocation functions in C, built upon a framework provided by Professor R. Jesse Chaney at Portland State University. It focuses on custom implementations of memory allocation techniques, mirroring functionalities of standard library functions like `malloc` and `free`.

## Project Contents

- **`vikalloc.c`**: The core file where I implemented the memory allocation functions.
- **`vikalloc.h`**: Header file defining structures and prototypes used in `vikalloc.c`.
- **`main.c`**: Test driver program for the allocation functions.
- **`Makefile`**: Contains commands for compiling and building the project, authored collaboratively with Professor Chaney.

## Implemented Functions

- `vikalloc()`: Custom `malloc` function.
- `vikfree()`: Custom `free` function.
- `vikcalloc()`: Custom `calloc` function.
- `vikrealloc()`: Custom `realloc` function.
- `vikstrdup()`: Custom `strdup` function.
- Utility and control functions for the allocator.

## Skills Demonstrated

- **C Programming**: Applied advanced C programming concepts for system-level programming.
- **Memory Management**: Demonstrated understanding of dynamic memory allocation and deallocation.
- **Debugging**: Ensured efficient memory operations and debugged complex scenarios.

## Building the Project

- Clone the repository to your local machine.
- Navigate to the project directory.
- Run the `make` command to build the program:
- To execute the program, run:
- Additional Makefile commands:
- `make clean`: Clean up compiled files.
- `make opt`: Compile with optimization flags.
- `make tar`: Create a tarball of the project.

## Acknowledgements

I'd like to extend my gratitude to Professor R. Jesse Chaney for providing the initial framework and guidance. This collaboration was pivotal in deepening my understanding of memory management in C.

---

Through this project, I have showcased my proficiency in C programming, particularly in low-level memory management and allocator design, key skills for system-level software development.


