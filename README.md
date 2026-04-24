# Compiler Assignment 3: SLR and LR(1) Parsers

This project implements SLR and LR(1) parsers for context-free grammars.

## Project Structure

- `src/cpp/`: C++ implementation source files.
- `src/java/`: Java implementation source files.
- `input/`: Test grammar and input files.
- `output/`: Generated tables, item sets, and traces.
- `docs/`: Assignment documentation and report.

## How to Build (C++)

Use the provided `Makefile`:

```bash
make
./parser
```

## How to Run (Java)

Compile and run the Java files from the `src/java` directory:

```bash
javac src/java/*.java
java src.java.Main
```

## Features

- Grammar augmentation
- SLR(1) Item sets and Parsing Table construction
- LR(1) Item sets and Parsing Table construction
- Parsing trace and Tree generation
- Grammar conflict detection
```
