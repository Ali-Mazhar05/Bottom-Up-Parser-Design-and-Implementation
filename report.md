# Technical Report: Bottom-Up Parser Design and Implementation

**Course**: Compiler Construction  
**Team Members**: 23i-0796, 23i-0783  
**Project**: SLR(1) and LR(1) Parser Implementation  

---

## 1. Introduction
Bottom-up parsing is a strategy for analyzing the grammatical structure of a string based on a context-free grammar. Unlike top-down parsing (which starts with the start symbol and attempts to reach the input), bottom-up parsing starts with the input tokens and performs "reductions" to build the parse tree upwards towards the start symbol.

LR parsers (Left-to-right, Rightmost derivation) are the most powerful type of bottom-up parsers. This project implements two variants:
- **SLR(1)**: Simple LR, which uses LR(0) items and FOLLOW sets for reduction decisions.
- **LR(1)**: Canonical LR, which includes lookahead information directly in the items to resolve ambiguities that SLR cannot.

---

## 2. Approach

### Data Structures
- **Grammar**: Represented as a list of `Production` structs. Terminals and non-terminals are tracked in `set<string>` for O(log n) lookup.
- **LR Items**:
    - `LR0Item`: Contains `prodId` and `dotPos`.
    - `LR1Item`: Inherits from `LR0Item` and adds a `lookahead` string.
- **Item Sets (States)**: Implemented as `vector<LR0Item>` or `vector<LR1Item>`. During construction, `set<Item>` is used to ensure uniqueness.
- **Parsing Table**: A `ParsingTable` class containing:
    - `map<int, map<string, set<Action>>> action`: The ACTION table.
    - `map<int, map<string, int>> gotoT`: The GOTO table.
- **Parse Tree**: A n-ary tree where each node (`TreeNode`) contains a label and a list of `shared_ptr<TreeNode>` children.

### Algorithm Implementation Details
- **CLOSURE**: 
    - For LR(0): Recursively adds productions for the non-terminal following the dot.
    - For LR(1): Additionally propagates lookaheads by computing `FIRST(beta a)` where `beta` is the symbol following the non-terminal and `a` is the current lookahead.
- **GOTO**: Computes the set of items that can be reached from a state by transitioning over a specific grammar symbol.
- **Table Construction**: Iterates through all states. Shift actions are created from transitions over terminals. Reduce actions are created from items where the dot is at the end.

### Lookahead Handling in LR(1)
LR(1) lookaheads are handled by extending the item definition. When expanding a non-terminal $B$ in $A \to \alpha \cdot B \beta, a$, the new items $B \to \cdot \gamma, b$ are generated where $b \in FIRST(\beta a)$. This precisely tracks which terminal can follow a reduction in a specific context.

### Design Decisions and Trade-offs
- **Using `using namespace std`**: Adopted project-wide to improve code readability and maintainability, as requested.
- **Memory Management**: Used `shared_ptr` for Parse Tree nodes to ensure automatic memory reclamation without manual `delete` calls.
- **Conflict Resolution**: Implemented a "Shift-over-Reduce" preference by default to handle common ambiguities like the dangling-else.

---

## 3. Challenges
- **Lookahead Propagation**: Correcting the logic for $FIRST(\beta a)$ when $\beta$ can derive $\epsilon$ was tricky. The solution involved a robust `FIRST` set computation that accounts for nullable symbols.
- **Efficiency**: LR(1) state space can grow exponentially(and my laptop crashed a few times cuz i tested on LLAMA model with large grammars but we ball). Used `std::map` with sorted item sets as keys to uniquely identify states quickly.
- **String Formatting**: Aligning the parsing table in the console required careful use of `std::setw` and calculating dynamic column widths. 
- **Parse Tree Visualization**: The `draw_tree` function was initially complex to get right. It took a few attempts(3 hours of my life that I can't get back) to debug the recursive calls and ensure the lines were drawn correctly.

---

## 4. Test Cases

### Grammar 1: Simple Expression
**Rules**: `Expr -> Expr + Term | Term`, `Term -> Factor`, `Factor -> id`  
| Input String | Result | Note |
|--------------|--------|------|
| `id` | Accept | Simple base case |
| `id + id` | Accept | Basic reduction |
| `id + id + id`| Accept | Left-associative check |
| `+ id` | Reject | Unexpected operator |
| `id +` | Reject | Missing operand |

### Grammar 2: Standard Arithmetic
**Rules**: Includes `*`, `/`, and `( )`.  
| Input String | Result | Note |
|--------------|--------|------|
| `id * id + id`| Accept | Precedence check |
| `( id + id ) * id` | Accept | Parentheses check |
| `id + ( id )` | Accept | Nested structure |
| `id * * id` | Reject | Consecutive operators |
| `( id + id` | Reject | Unbalanced parens |

### Grammar 3: SLR Conflict (LR(1) Resolution)
**Rules**: `S -> L = R | R`, `L -> * R | id`, `R -> L`  
| Input String | Result | Note |
|--------------|--------|------|
| `id = id` | Accept | Valid assignment |
| `* id = id` | Accept | Pointer assignment |
| `id` | Accept | Simple R-value |
| `id =` | Reject | Missing RHS |

---

## 5. Comparison Analysis

| Metric | SLR(1) | LR(1) |
|--------|--------|-------|
| **States (Grammar 2)** | 12 | 22 |
| **States (Grammar 3)** | 10 | 14 |
| **Parsing Power** | Weak (Fails on `S->L=R`) | Strong (Clean) |
| **Construction Time** | < 1ms | ~2-5ms |
| **Memory Usage** | Very Low | Moderate |

**Insights**: LR(1) is significantly more powerful, resolving conflicts by "splitting" states based on lookahead context. However, this leads to a larger state count (nearly 2x in some cases).

---

## 6. Sample Outputs

### Parsing Table (Grammar 3 - SLR)
```text
State |      id      *      =      $ |  Start   Lval   Rval
-----------------------------------------------------------
I2    |             r3     s8     r3 | 
```
*Note: State I2 on '=' shows a shift/reduce conflict in SLR because '=' is in FOLLOW(Rval).*

### Conflict Resolution (Grammar 3 - LR(1))
In LR(1), state I2 is split into two states. One specifically expects `=` (for the assignment context), and the other expects `$` (for the R-value context), thus resolving the conflict.

### Parse Tree Example (`id + id`)
```text
└── Expr
    ├── Expr
    │   └── Term
    │       └── Factor
    │           └── id
    ├── +
    └── Term
        └── Factor
            └── id
```

---

## 7. Conclusion
This project demonstrated that while SLR(1) is efficient and easy to implement, it is often insufficient for practical programming languages that have subtle context-dependent ambiguities. LR(1) provides a robust solution at the cost of increased table size. Implementing the diagnostic feedback taught us that a parser's value lies not just in accepting valid code, but in providing clear, actionable error messages for developers.
