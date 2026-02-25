# ValuaScript Lexer: Mechanics & Extension Guide

The Lexer (or Scanner) is the first line of defense in the ValuaScript compiler. Its job is to group raw characters into meaningful words (`Tokens`) and discard formatting detritus like whitespace and comments. It operates as a strict, forward-moving sliding window over the source string.

## Part 1: How the Lexer Works in Practice

### 1. The Sliding Window (`start` and `current`)

At its core, the Lexer tracks its position in the source code using indices (conceptually, a `start` and a `current` pointer).

* `start` points to the first character of the lexeme being scanned.
* `current` points to the character currently being evaluated.
* When a token is finalized, the substring from `start` to `current` is emitted as the `lexeme`, and `start` catches up to `current` to begin the next token.

### 2. The Core Primitives

The Lexer navigates the string using three essential methods:

* **`advance()`**: Consumes the next character and moves the `current` pointer forward. It *always* changes state.
* **`peek()`**: Looks at the current character without consuming it. It is a safe lookahead.
* **`match(expected)`**: A conditional `advance()`. It peeks at the current character; if it matches the `expected` character, it consumes it and returns `true`. Otherwise, it does nothing and returns `false`.

### 3. The Maximal Munch Principle

When the Lexer encounters a `<` character, it doesn't immediately emit a `Less` token. It uses the Maximal Munch principle: *always consume the longest possible valid token*.
It calls `match('=')`. If true, it emits `LessEqual` (`<=`). If false, it falls back to emitting `Less` (`<`).

### 4. Handling Keywords vs. Identifiers

The Lexer does not have a hardcoded `switch` case for every keyword (like `let`, `func`, `return`). Instead, it treats every alphabetical sequence as a generic `Identifier`. Once the full identifier is consumed (e.g., `r-e-t-u-r-n`), it checks a pre-populated Hash Map (or Dictionary) of reserved keywords. If `return` is in the map, it emits a `TokenType::Return`. If not, it emits a `TokenType::Identifier`. This makes adding new keywords completely frictionless.

---

## Part 2: Extending the Lexer (Future-Proofing)

Because the Lexer is a simple state machine, extending it requires only mechanical additions. Here is exactly how to evolve the Lexer when ValuaScript needs new features.

### Scenario A: Adding a New Keyword (e.g., `while` or `struct`)

**Goal:** Make ValuaScript understand a new reserved word.
**Effort:** Trivial.

1. **Update `TokenType` Enum:** Add `While` or `Struct` to your `TokenType` definition.
2. **Update the Keyword Map:** Locate the dictionary/map where reserved words are registered in the Lexer initialization.
```cpp
// Inside the Lexer's constructor or initialization block
keywords["let"] = TokenType::Let;
keywords["func"] = TokenType::Func;
keywords["return"] = TokenType::Return;

// --- YOUR ADDITION ---
keywords["while"] = TokenType::While;
keywords["struct"] = TokenType::Struct;

```



*That is it.* The existing identifier scanning loop will automatically catch these words, look them up in the map, and emit the correct token.

### Scenario B: Adding a New Multi-Character Operator (e.g., `+=` or `!=`)

**Goal:** Add a compound operator that reuses existing characters.
**Effort:** Low.

1. **Update `TokenType` Enum:** Add `PlusEqual` or `NotEqual`.
2. **Update the `switch` statement:** Find the case for the starting character (e.g., `+` or `!`) and use the `match()` primitive to branch the logic.
```cpp
case '+':
    // --- YOUR ADDITION ---
    if (match('=')) {
        add_token(TokenType::PlusEqual);
    } else {
        add_token(TokenType::Plus);
    }
    break;

case '!':
    // --- YOUR ADDITION ---
    if (match('=')) {
        add_token(TokenType::NotEqual);
    } else {
        // Depending on your language, a naked '!' might be a TokenType::Not
        add_token(TokenType::Not); 
    }
    break;

```



### Scenario C: Adding a New Literal Type (e.g., Hexadecimal Numbers `0xFF`)

**Goal:** Allow users to write numbers in base-16.
**Effort:** Medium.

1. **Locate the Number Parsing Logic:** Find the block that triggers when `is_digit(peek())` is true.
2. **Add a Lookahead Branch:** Before entering the standard base-10 parsing loop, check if the current character is `0` and the next character is `x` or `X`.
```cpp
// Inside the number scanning method
if (peek() == '0' && (peek_next() == 'x' || peek_next() == 'X')) {
    advance(); // consume '0'
    advance(); // consume 'x'

    // Scan hexadecimal characters
    while (is_hex_digit(peek())) {
        advance();
    }
    add_token(TokenType::NumberLiteral); // Or a specific HexLiteral token
    return;
}

// ... fallback to standard decimal parsing ...

```