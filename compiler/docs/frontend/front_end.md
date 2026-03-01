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
