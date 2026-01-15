# Polynomial Language Parser

A compiler front-end implementation for a custom polynomial language. This project includes lexical analysis, parsing, semantic error detection, and program execution for a domain-specific language designed to define and evaluate polynomial expressions.

## Overview

This parser processes programs written in a polynomial language that supports:

- Polynomial declarations with single or multiple variables
- Polynomial evaluation with nested calls
- Variable input/output operations
- Assignment statements
- Static semantic analysis
- Dynamic program execution

## Language Syntax

### Program Structure

A program consists of four sections in order:

```
TASKS <task_numbers>
POLY
<polynomial_declarations>
EXECUTE
<statements>
INPUTS <input_values>
```

### Polynomial Declarations

```
// Single variable (implicitly named 'x')
f = 2x^2 + 3x + 1;

// Multiple variables
g(a, b) = a^2 + 2a b + b^2;

// Parenthesized expressions
h = (x + 1)(x - 1);

// Constants
constant = 42;
```

### Statement Types

```
INPUT var;           // Read input into variable
OUTPUT var;          // Print variable value
var = poly(args);    // Assign polynomial evaluation result
```

### Polynomial Evaluation Arguments

Arguments can be:
- Variables: `f(x)`
- Numbers: `f(5)`
- Nested evaluations: `g(f(x))`

## Tasks

The parser supports five analysis/execution tasks, specified in the TASKS section:

| Task | Description |
|------|-------------|
| 1 | Semantic error detection (always performed internally) |
| 2 | Program execution and output |
| 3 | Uninitialized variable warnings |
| 4 | Useless assignment detection |
| 5 | Polynomial degree computation |

## Semantic Errors

The parser detects four types of semantic errors (in priority order):

| Code | Description | Example |
|------|-------------|---------|
| 1 | Duplicate polynomial declaration | `f = x; f = 2x;` |
| 2 | Undefined variable in polynomial body | `f(x) = x + y;` (y undefined) |
| 3 | Undeclared polynomial in evaluation | `y = h(x);` (h not declared) |
| 4 | Wrong number of arguments | `f(x,y) = ...; z = f(a);` |

Output format: `Semantic Error Code N: <line_numbers>`

## Warnings

| Code | Description |
|------|-------------|
| 1 | Use of uninitialized variable |
| 2 | Useless assignment (value never used) |

Output format: `Warning Code N: <line_numbers>`

## Building

### Prerequisites

- C++17 compatible compiler (g++ recommended)
- Make (optional)

### Compilation

Using Make:
```bash
make
```

Manual compilation:
```bash
g++ -std=c++17 -Wall -o poly_parser inputbuf.cc lexer.cc parser.cc
```

## Usage

```bash
./poly_parser < input_file.txt
```

Or with Make:
```bash
make run INPUT=tests/test_basic_task2.txt
```

## Example

### Input
```
TASKS 2 5
POLY
square = x^2;
add(a, b) = a + b;
EXECUTE
INPUT x;
INPUT y;
z = square(x);
w = add(z, y);
OUTPUT w;
INPUTS 3 4
```

### Output
```
13
square: 2
add: 1
```

Task 2 computes: `square(3) = 9`, then `add(9, 4) = 13`  
Task 5 reports polynomial degrees.

## Project Structure

```
.
├── inputbuf.h          # Input buffer class declaration
├── inputbuf.cc         # Input buffer implementation
├── lexer.h             # Lexical analyzer declarations and token types
├── lexer.cc            # Lexical analyzer implementation
├── parser.h            # Parser class with AST and statement structures
├── parser.cc           # Parser implementation with execution logic
├── Makefile            # Build configuration
├── run_tests.sh        # Automated test runner
├── README.md           # This file
└── tests/              # Test case directory
    ├── test_basic_task2.txt
    ├── test_sem_err1_duplicate.txt
    ├── test_sem_err2_invalid_name.txt
    ├── test_sem_err3_undeclared.txt
    ├── test_sem_err4_wrong_args.txt
    ├── test_task2_evaluation.txt
    ├── test_task3_uninit_warn.txt
    ├── test_task4_useless_assign.txt
    ├── test_task5_degrees.txt
    └── ...
```

## Testing

Run all tests:
```bash
make test
```

Or directly:
```bash
./run_tests.sh
```

### Test Categories

| Category | Test Files |
|----------|------------|
| Basic execution | `test_basic_task2.txt`, `test_task2_evaluation.txt` |
| Semantic errors | `test_sem_err1_*.txt` through `test_sem_err4_*.txt` |
| Warnings | `test_task3_*.txt`, `test_task4_*.txt` |
| Degree computation | `test_task5_degrees.txt`, `test_high_degree.txt` |
| Complex features | `test_nested_eval.txt`, `test_complex_poly.txt` |

## Architecture

### Components

1. **InputBuffer** (`inputbuf.h/cc`): Character-level input handling with unget support
2. **LexicalAnalyzer** (`lexer.h/cc`): Tokenization of input stream
3. **Parser** (`parser.h/cc`): 
   - Recursive descent parsing
   - AST construction for polynomials
   - Semantic error detection
   - Program execution

### Token Types

```
POLY, INPUT, TASKS, EXECUTE, OUTPUT, INPUTS,
EQUAL, LPAREN, RPAREN, ID, COMMA, POWER, NUM,
PLUS, MINUS, SEMICOLON, ERROR, END_OF_FILE
```

### AST Node Types

- `TERM_LIST`: Sum/difference of terms
- `TERM`: Coefficient with monomial list
- `MONO_LIST`: Product of monomials
- `MONO`: Primary with exponent
- `PRIMARY`: Variable or parenthesized expression

## Grammar (Simplified)

```
program      → tasks_section poly_section execute_section inputs_section
tasks_section → TASKS num_list
poly_section → POLY poly_decl_list
poly_decl    → ID [ '(' id_list ')' ] '=' term_list ';'
term_list    → term { ('+' | '-') term }
term         → [NUM] monomial_list | NUM
monomial     → primary [ '^' NUM ]
primary      → ID | '(' term_list ')'
statement    → INPUT ID ';' | OUTPUT ID ';' | ID '=' poly_eval ';'
poly_eval    → ID '(' argument_list ')'
argument     → NUM | ID | poly_eval
```

## Implementation Notes

- Polynomial evaluation uses AST traversal with argument substitution
- Degree computation considers exponents in monomial products
- Useless assignment detection uses backward liveness analysis
- Memory is simulated with a fixed-size array (2000 locations)

## License

This is an educational project. Original template by Rida Bazzi (2017-2019).

## Contributing

When adding new tests:
1. Create test file in `tests/` directory
2. Add expected output to `run_tests.sh`
3. Run `make test` to verify
