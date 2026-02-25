# ValuaScript Parser: Mechanics & Extension Guide

The Parser is a **Recursive Descent** engine. It consumes the linear array of `Token` objects produced by the Lexer and
outputs a highly structured, multidimensional Abstract Syntax Tree (AST).

## Part 1: How the Parser Works in Practice

A recursive descent parser translates formal grammar rules directly into C++ functions. Every rule in the language's
grammar becomes a distinct method in the `ParserStage` class.

### 1. The Pointer and The Primitives

Just like the Lexer, the Parser maintains a `current` index, but it iterates over a `std::vector<Token>` instead of a
string. It navigates this vector using a strict set of primitives:

* **`check(TokenType type)`:** Looks at the current token without consuming it. Returns `true` if it matches the type.
* **`match(std::initializer_list<TokenType> types)`:** Checks the current token against a list of types. If one matches,
  it consumes the token and returns `true`. This is the core branching mechanism.
* **`consume(TokenType type, ErrorCode code, string message)`:** The enforcer. It demands that the current token is a
  specific type. If it is, it consumes it. If it is not, it immediately throws a `ValuaScriptException` with the
  provided error code.

### 2. Precedence via Call Stack Hierarchy (Layering)

The Parser enforces mathematical precedence (PEMDAS/BODMAS) not by looking ahead, but by **burying high-precedence
operations deeper in the call stack**.

When the parser needs an expression, it calls the lowest-precedence method first:

1. `parse_expression()` calls -> `parse_assignment()`
2. `parse_assignment()` calls -> `parse_logical_or()`
3. `parse_logical_or()` calls -> ... down to ... -> `parse_addition()`
4. `parse_addition()` calls -> `parse_multiplication()`
5. `parse_multiplication()` calls -> `parse_unary()`
6. `parse_unary()` calls -> `parse_atom()` (Numbers, Identifiers, Grouping Parentheses)

Because `parse_addition()` must call `parse_multiplication()` to get its left and right operands, the multiplication
nodes are naturally forced to the bottom of the AST, ensuring they are evaluated first by the execution engine.

### 3. Left-Associativity via `while` Loops

To ensure `10 - 5 - 2` parses as `(10 - 5) - 2` (Left-Associative) rather than `10 - (5 - 2)` (Right-Associative), the
parser uses flat `while` loops instead of right-recursion.

```cpp
auto expr = parse_multiplication(); // Get the left side
while (match({TokenType::Plus, TokenType::Minus})) {
    Token op = previous();
    auto right = parse_multiplication(); // Get the right side
    // Wrap the existing tree as the left child of the new node
    expr = std::make_unique<BinaryExpression>(std::move(expr), std::move(right), op.type);
}
return expr;

```

### 4. Infinite Postfix Chaining

To handle complex contiguous chains like `matrix[0]()[1](arg)`, `parse_atom()` ends with an infinite `while(true)` loop.
Once it parses the base identifier (`matrix`), it loops continuously, checking for `[` or `(`. Each time it finds one,
it wraps the current expression as the `target` of a new `VectorAccess` or `FunctionCall` node, allowing the tree to
grow infinitely deep upwards.

---

## Part 2: Extending the Parser

When ValuaScript evolves, you must know exactly where to insert new logic. Here are extreme, complex extension
scenarios.

---

### Scenario A: Adding a Right-Associative Operator (e.g., Vector Cons `::`)

**The Challenge:** Unlike addition, a "cons" operator used to build lists (`1 :: 2 :: [3]`) must evaluate right-to-left
as `1 :: (2 :: [3])`. If it evaluated left-to-right, it would attempt to prepend the number `2` onto the number `1`,
causing a semantic crash.
*(Note: ValuaScript already implements right-associativity for the Power operator `^`. This scenario shows how to add
another one).*

**The Implementation:**

1. **Lexer (`token.h` & `lexer_stage.cpp`):**

* Add `ColonColon` to the `TokenType` enum.
* Update the `switch` case for `:` to check `match(':')` and emit `ColonColon`.


2. **Parser (`parser_stage.cpp`):**

* Create a new method `parse_cons()`. Insert it into the hierarchy just above your lowest precedence binary operators (
  e.g., above addition/multiplication but below assignments).
* **The Trick:** Instead of a `while` loop, use **Recursion** to force right-associativity.

```cpp
std::unique_ptr<Expression> parse_cons() {
    auto expr = parse_addition(); // Get left side (the item to prepend)
    
    // Check for the operator using an 'if' instead of a 'while'
    if (match({TokenType::ColonColon})) {
        Token op = previous();
        
        // Recursively call parse_cons() for the right side!
        // This forces the parser to dive all the way down the right side of the 
        // chain before it starts building the tree back upwards.
        auto right = parse_cons(); 
        
        expr = std::make_unique<BinaryExpression>(std::move(expr), std::move(right), op.type);
    }
    
    return expr;
}

```

By recursively calling the same method for the right-hand operand, the parser naturally defers the creation of the
current node until the entire right-side chain has been fully parsed and constructed.

### Scenario B: Adding a Complex Block Statement (e.g., `match` / `switch`)

**The Challenge:** You want to add a Rust-style `match` statement:
`match x { 1 -> do_this(), 2 -> do_that() }`
**The Implementation:**

1. **AST (`ast.h`):** Create `MatchBranch` (holds a condition expression and a statement body). Create
   `MatchStatement` (holds the target expression and a `std::vector<MatchBranch>`).
2. **Parser (`parser_stage.cpp`):**

* Add `parse_match_statement()`.
* `consume(TokenType::Match)`
* `auto target = parse_expression()`
* `consume(TokenType::LeftBrace)`
* Start a `while (!check(TokenType::RightBrace))` loop.
* Inside the loop: `auto cond = parse_expression()`, `consume(TokenType::Arrow)`, `auto body = parse_statement()`, and
  push the branch to the vector. Optionally `match(TokenType::Comma)`.
* `consume(TokenType::RightBrace)`.


3. **Routing:** Add `if (check(TokenType::Match)) return parse_match_statement();` to your main `parse_statement()`
   routing logic.

### Scenario C: Adding an Infix Operator with Short-Circuit Logic (e.g., Null-Coalescing `??`)

**The Challenge:** You want `a ?? b`. If `a` is null, evaluate `b`.
**The Implementation:**

1. **AST (`ast.h`):** Do *not* use `BinaryExpression`. Create a dedicated `NullCoalescingExpression`. Why? Because
   semantic execution needs to know to conditionally evaluate the right side, unlike addition where both sides are
   always evaluated.
2. **Parser (`parser_stage.cpp`):**

* Determine precedence. It usually sits just above Assignment but below Logical OR.
* Create `parse_null_coalesce()`.
* Use the standard `while (match({TokenType::DoubleQuestion}))` loop.
* Build and return the `std::make_unique<NullCoalescingExpression>(left, right)`.

### Scenario D: Adding Anonymous Functions (Lambdas)

**The Challenge:** You want to allow passing functions as expressions: `map(data, func(x) -> x * 2)`.
**The Implementation:**

1. **AST (`ast.h`):** Create `LambdaExpression : public Expression`. It requires exactly the same fields as
   `FunctionDefinition` (parameters, return types, body).
2. **Parser (`parser_stage.cpp`):** * A lambda is an expression, not a root-level definition. Therefore, it must be
   added to `parse_atom()`.

* In `parse_atom()`, add:

```cpp
if (match({TokenType::Func})) {
    // We already consumed 'func'. 
    // Do NOT expect an identifier name here.
    // Jump straight to parsing parameters:
    consume(TokenType::LeftParen, ...);
    // ... parse params, arrow, return types, and body brace ...
    return std::make_unique<LambdaExpression>(params, returns, body);
}

```

Because `parse_atom()` returns an `Expression`, this lambda can now be assigned to variables, passed into function
calls, or even executed immediately via the postfix loop `func(x) -> x * 2 (10)`.

