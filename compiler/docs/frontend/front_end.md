# ValuaScript Frontend Architecture (v1.0)

The ValuaScript frontend is strictly divided into two decoupled stages: the **Lexer** (Lexical Analysis) and the *
*Parser** (Syntax Analysis). This separation of concerns guarantees that string manipulation logic never pollutes the
structural grammar logic.

## Part 1: The Lexer (The Context-Blind Reader)

The Lexer's sole responsibility is to convert a stream of raw string characters into a linear array of manageable
`Token` objects.

### Core Philosophy: "Context Blindness"

The Lexer has absolutely no knowledge of ValuaScript's grammar. If it encounters the sequence `+ + +`, it does not throw
a syntax error. It simply outputs three `Plus` tokens. Validation is strictly reserved for the Parser.

### Key Architectural Decisions & The "Whys"

* **Manual Character Scanning over Regex:**
* *The Choice:* We built a manual `while (!is_at_end())` loop with a `switch` statement rather than using Regular
  Expressions to identify tokens.
* *The Why:* Performance and error reporting. Manual scanning allows us to perfectly track `line` and `column` numbers
  character-by-character. If a user makes a mistake on line 402, column 15, we can point exactly to it. Regex engines
  make this spatial tracking exceptionally difficult and slow down compilation.


* **Preserving Literal Quotes in Strings:**
* *The Choice:* When scanning `"hello"`, the Lexer outputs the token lexeme exactly as `"\"hello\""`, keeping the quotes
  intact.
* *The Why:* The Lexer should not destroy source information. Stripping quotes and handling escape characters (like
  `\n`) requires semantic understanding of the string's intent, which is delegated downstream to the Semantic Analyzer.


* **Maximal Munch Principle (Multi-Character Tokens):**
* *The Choice:* We implemented `match()` logic to look ahead. If the Lexer sees `=`, it checks the next character. If
  it's another `=`, it outputs `TokenType::Equals` (`==`) instead of two `Assign` (`=`) tokens.
* *The Why:* This prevents the Parser from having to reconstruct multi-character operators from fragments, keeping the
  Parser's logic strictly focused on grammar rather than token assembly.

---

## Part 2: The Parser (The Structural Builder)

The Parser takes the linear array of tokens and converts it into a hierarchical Abstract Syntax Tree (AST). We utilized
a **Recursive Descent** architecture, meaning the parser uses a series of mutually recursive function calls to traverse
the grammar.

### Core Philosophy: "Shape vs. Meaning"

The single most important rule in the ValuaScript parser is that it only validates the *Shape* (Syntax) of the code,
never the *Meaning* (Semantics).

### Key Architectural Decisions & The "Whys"

* **Allowing "Nonsense" Shapes (`1()` or `matrix[0][1]` on a scalar):**
* *The Choice:* The Parser will happily build an AST for `1()` (a function call where the target is a number) without
  throwing an error.
* *The Why:* Grammatically, `Expression()` is a valid shape. Checking if `1` is actually a callable function requires
  looking up memory addresses and types. By ignoring meaning, the Parser remains fast and lightweight. The Semantic
  Analyzer will later catch this and throw a "Type Mismatch" error.


* **AST Polymorphism via `std::unique_ptr<Expression>`:**
* *The Choice:* Every node in the AST holds its children as abstract `Expression` or `Statement` pointers, rather than
  concrete types (e.g., `VectorAccess` holds an `Expression*` as its target, not a `String`).
* *The Why:* **Orthogonality**. This guarantees infinite chaining. Because a function call target can be *any*
  expression, ValuaScript automatically supports advanced patterns like `factory_func()(arg)` or `matrix[0][1]` without
  needing special edge-case logic.


* **Strict Left-Associativity in Math:**
* *The Choice:* Operations of the same precedence are grouped left-to-right. `10 - 5 - 2` becomes `(10 - 5) - 2`.
* *The Why:* Mathematical accuracy. If we implemented right-associativity, `10 - (5 - 2)` would evaluate to `7`,
  silently corrupting the user's math.


* **The "C++ Trap" (Forbidding Chained Comparisons):**
* *The Choice:* The Parser intentionally throws a `Syntax Error` if a user writes `a < b < c`.
* *The Why:* In C/C++, `30 < 20 < 10` evaluates `30 < 20` to `false` (0), and then checks if `0 < 10`, returning `True`.
  This is a massive source of silent bugs. ValuaScript strictly forbids it, forcing the user to write `a < b and b < c`
  for absolute clarity.


* **Native Multi-Value Returns:**
* *The Choice:* `ReturnStatement` holds a `std::vector<std::unique_ptr<Expression>>` to support `return 10, 20`.
* *The Why:* ValuaScript supports multiple-assignment (`let a, b = get_coords()`). The return statement must mirror this
  structurally to allow functions to safely emit tuples without requiring a dedicated `Tuple` object wrapper in the
  frontend.

---

## Part 3: The Testing Philosophy

Our testing suite is designed for maximum developer velocity and absolute structural certainty.

### Key Architectural Decisions & The "Whys"

* **The "Trusted Lexer" Pipeline for Parser Tests:**
* *The Choice:* Instead of manually creating mocked arrays of tokens (e.g., `Token(Let), Token(Id), Token(Assign)`), our
  parser tests pass raw strings (e.g., `"let a = 1"`) through the Lexer first.
* *The Why:* Manually mocking token arrays is unmaintainable. If we add a field to the `Token` class, hundreds of tests
  would break. By treating our fully-tested Lexer as a "Trusted Component," parser tests remain highly readable and
  future-proof.


* **Deep AST Geometric Validation:**
* *The Choice:* We wrote specific tests that physically `dynamic_cast` down the AST tree to verify exact node placement.
* *The Why:* Checking that the parser "didn't crash" is insufficient. We must prove the mathematical geometry of the
  tree. Casting nodes ensures that `1 + 2 * 3` specifically puts the `*` at the bottom of the tree, mathematically
  guaranteeing correct operator precedence.

---

## Part 4: Extensibility & Language Evolution

ValuaScript’s frontend is designed to be effortlessly extensible. When adding a new language feature, you will always
follow a strict, four-step pipeline. Because the Lexer and Parser are decoupled, you never have to rewrite core
logic—you only add to it.

### Scenario A: Adding a New Statement (e.g., a `while` loop)

Statements are structural blocks that do not evaluate to a value (unlike expressions).

1. **Lexer (`token.h` & `lexer_stage.cpp`):** * Add `While` to the `TokenType` enum.

* Add `"while"` to the Lexer's reserved keyword map.


2. **AST (`ast.h`):** * Create a new class: `class WhileStatement : public Statement`.

* Give it two fields: `std::unique_ptr<Expression> condition` and `std::vector<std::unique_ptr<Statement>> body`.


3. **Parser (`parser_stage.cpp`):** * Write a new private method: `std::unique_ptr<Statement> parse_while_statement()`.

* Inside it, consume the `While` token, parse the condition expression, consume the `{`, loop to parse the body
  statements, and consume the `}`.
* Update your main statement routing loop (where it currently checks for `Let` and `Return`) to also route to
  `parse_while_statement()` if it sees `TokenType::While`.


4. **Tests:** * Add a structural test proving the `WhileStatement` AST node holds the correct condition and body.

### Scenario B: Adding a New Operator (e.g., a Pipe Operator `|>`)

Operators modify how expressions are chained or evaluated mathematically.

1. **Lexer (`token.h` & `lexer_stage.cpp`):**

* Add `Pipe` to the `TokenType` enum.
* In the Lexer's `switch` statement, add a case for `|` that uses `match('>')` to emit the `Pipe` token.


2. **AST (`ast.h`):**

* *No change needed.* The existing `BinaryExpression` class perfectly handles all two-sided operations.


3. **Parser (`parser_stage.cpp`):**

* Determine the **Precedence** of the new operator. Should it evaluate before or after addition?
* If it has very low precedence (evaluates last), create a new method `parse_pipe_expression()` that calls your current
  lowest-precedence method, loops while `match({TokenType::Pipe})`, and builds a `BinaryExpression`.
* Update `parse_expression()` to point to your new `parse_pipe_expression()` at the top of the chain.


4. **Tests:**

* Add a test in `ast_math_precedence_test.cpp` to prove `1 + 2 |> func()` groups exactly as intended.

### Scenario C: Adding New Data Types (e.g., `tensor` or `struct`)

Types in ValuaScript are handled beautifully due to our abstraction choices.

* **Standard Types & Generics (`tensor<scalar>`):** * *Effort: Zero.* The frontend already parses any identifier
  combined with `< >` as a `TypeAnnotation`. The Lexer and Parser do not care if `tensor` exists. The Semantic Analyzer
  will be the one to check the `TypeRegistry` and validate it.
* **New Literals (e.g., Dictionary `{ "key": 1 }`):**
* Add `LeftBrace`/`RightBrace` to tokens (already exist).
* Create a `DictLiteral` AST class holding a vector of key-value expression pairs.
* Add a branch in `parse_atom()` to detect a `{` and route to a `parse_dict_literal()` method.

### The Golden Rule of Extension

**Never mix meaning with shape during an extension.** If you add a `struct` definition, the parser's only job is to
ensure the user wrote `struct Name { ... }`. Do not write parser logic to check if that struct name was already
taken—that is strictly the domain of the Symbol Table in the Semantic Analyzer. By adhering to this rule, ValuaScript
can scale to hundreds of features without the frontend ever becoming a bottleneck.