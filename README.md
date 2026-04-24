# Bottom-Up Parser: SLR(1) and LR(1)

## Team Members
| Name | Roll Number |
|------|-------------|
| Member 1 | 23i-0796 |
| Member 2 | 23i-0783 |

## Programming Language
**C++17**

---

## Compilation

### Prerequisites
- `g++` with C++17 support (`g++ --version` ≥ 7)
- GNU Make

### Build
```bash
make
```
This produces the `parser` executable and creates the `output/` directory.

### Clean
```bash
make clean
```

---

## Execution

### General Syntax
```
./parser <mode> <grammar_file> [input_file]
```

### Modes
| Mode | Description |
|------|-------------|
| `--slr` | Run SLR(1) parser only |
| `--lr1` | Run LR(1) parser only |
| `--both` | Run both parsers and compare |
| `--compare` | State/conflict comparison only (no parsing) |

### Examples
```bash
# SLR(1) on expression grammar with valid inputs
./parser --slr input/grammar2.txt input/input_valid.txt

# LR(1) on expression grammar
./parser --lr1 input/grammar2.txt input/input_valid.txt

# Both parsers + comparison on expression grammar
./parser --both input/grammar2.txt input/input_valid.txt

# Demonstrate LR(1) superiority over SLR(1) (grammar3 has SLR conflicts)
./parser --both input/grammar3.txt input/input_valid.txt

# Compare state counts only
./parser --compare input/grammar2.txt

# Test with invalid inputs
./parser --slr input/grammar2.txt input/input_invalid.txt

# Dangling-else conflict grammar
./parser --both input/grammar_with_conflict.txt input/input_valid.txt
```

### Make Shortcuts
```bash
make run-slr       # SLR on grammar2 + valid inputs
make run-lr1       # LR(1) on grammar2 + valid inputs
make run-both      # Both on grammar2 + valid inputs
make run-compare   # Compare on grammar3 (SLR conflict demo)
make run-conflict  # Both parsers on conflict grammar
make run-invalid   # Test invalid inputs
make valgrind      # Memory leak check
```

---

## Input File Format

### Grammar File (`input/grammar*.txt`)
- One production rule per line
- Format: `NonTerminal -> alt1 | alt2 | ...`
- Arrow symbol: `->`
- Alternatives separated by `|`
- **Non-terminals**: Multi-character names starting with uppercase (e.g., `Expr`, `Term`, `Factor`)
- **Terminals**: lowercase letters, operators, keywords (e.g., `id`, `+`, `*`, `(`, `)`)
- **Epsilon**: use `epsilon` or `@`
- Lines starting with `#` are comments
- First production's LHS is the start symbol

#### Example
```
# Expression Grammar
Expr -> Expr + Term | Term
Term -> Term * Factor | Factor
Factor -> ( Expr ) | id
```

### Input String File (`input/input*.txt`)
- One input string per line (space-separated tokens)
- Lines starting with `#` are comments

#### Example
```
id + id * id
( id + id )
id
```

---

## Output Files (auto-generated in `output/`)

| File | Description |
|------|-------------|
| `augmented_grammar.txt` | Augmented grammar with production IDs |
| `slr_items.txt` | All LR(0) states with items |
| `slr_parsing_table.txt` | SLR(1) ACTION/GOTO table + conflicts |
| `slr_trace.txt` | Step-by-step SLR parsing trace |
| `lr1_items.txt` | All LR(1) states with items + lookaheads |
| `lr1_parsing_table.txt` | LR(1) ACTION/GOTO table + conflicts |
| `lr1_trace.txt` | Step-by-step LR(1) parsing trace |
| `comparison.txt` | Side-by-side SLR vs LR(1) metrics |
| `parse_trees.txt` | Parse trees for accepted strings |

---

## Known Limitations
- Single-character non-terminals (E, T, F) are **not** allowed — use multi-character names (Expr, Term, Factor)
- Ambiguous grammars (e.g., dangling-else) will show conflicts in both parsers; shift is preferred by default
- LR(1) table construction can be slow for very large grammars (exponential state space)
- Grammar file must use spaces to separate all tokens in productions
