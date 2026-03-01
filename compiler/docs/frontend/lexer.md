# Lexer Stage

## 1. Architectural Overview

The `LexerStage` is the first transformation phase in the ValuaScript compilation pipeline. It acts as an adapter
between the raw source input (as a stream of characters) and the subsequent syntax analysis phases (Parser), which
require structured input.

The lexer is implemented as an encapsulated unit within the `valuascript::compiler` namespace. It adheres to the
`CompilerStage` interface, transforming `CompilerStageArtifactCode::SourceCode` into
`CompilerStageArtifactCode::TokenStream`.

---

## 2. Token Specification

The lexer reduces the source code into a linear sequence of `Token` structures. Each `Token` preserves its lexical type,
associated lexeme, and source position (line and column) for diagnostic accuracy.

### 2.1 Token Categories

The grammar defines the following token taxonomy:

| Category        | Token Types                                                                                                                         |
|-----------------|-------------------------------------------------------------------------------------------------------------------------------------|
| **Punctuation** | `LeftParen`, `RightParen`, `LeftBracket`, `RightBracket`, `LeftBrace`, `RightBrace`, `Comma`, `Colon`, `At`, `Percent`              |
| **Operators**   | `Plus`, `Minus`, `Star`, `Slash`, `Caret`, `Assign`, `Equals`, `NotEquals`, `Greater`, `GreaterEqual`, `Less`, `LessEqual`, `Arrow` |
| **Literals**    | `Identifier`, `String`, `DocString`, `Number`                                                                                       |
| **Keywords**    | `Import`, `Let`, `If`, `Then`, `Else`, `True`, `False`, `And`, `Or`, `Not`, `Func`, `Struct`, `Return`                              |
| **Metadata**    | `EndOfFile`                                                                                                                         |

---

## 3. Lexical Analysis Logic

### 3.1 Scanning Strategy

The `Lexer` class utilizes a single-pass, predictive scanning approach. It maintains pointers to the `start_` and
`current_` positions within the source string.

* **Dispatch Mechanism:** The `scan_token()` method acts as the primary dispatch loop, consuming the lookahead character
  and branching into specialized handlers for operators, literals, and identifiers.
* **Encapsulation:** The implementation is strictly hidden within an anonymous namespace, preventing symbol leakage.
* **Lookahead:** The lexer supports $1$-character lookahead (`peek()`) and $2$-character lookahead (`peek_next()`) to
  differentiate multi-character operators (e.g., `==`, `!=`, `->`, `docstrings`).

### 3.2 Specific Handling Logic

#### String Literals

The lexer handles both standard string literals (`"..."`) and multi-line docstrings (`"""..."""`).

* **State Detection:** A boolean `is_docstring` flag is set by checking for a triple-quote sequence.
* **Termination:** Docstrings are terminated by the first occurrence of `"""`. Standard strings are terminated by a
  single `"`.
* **Diagnostics:** Unclosed strings at `EOF` trigger a `ValuaScriptException` with `ErrorCategory::Lexical`.

#### Numeric Literals

Numeric scanning supports integer and floating-point formats, with optional `_` as a digit separator.

* **Validation:** The lexer enforces strict separator rules: an underscore `_` must be followed by a digit. A trailing
  or leading underscore, or an underscore immediately followed by a non-digit, is treated as an invalid lexical
  sequence.
* **Complexity:** The consumption is performed in $O(N)$ time relative to the number of digits in the token.

#### Identifiers and Keywords

* **Mechanism:** Identifiers are greedy, consuming all alphanumeric characters and underscores.
* **Resolution:** Once a candidate identifier is fully scanned, a `static` lookup map (`kKeywords`) is queried. This
  allows for $O(1)$ keyword identification, minimizing overhead for reserved word checks.

---

## 4. Error Handling and Diagnostics

The lexer is designed to fail early and informatively. It does not attempt to recover from lexical errors (e.g., invalid
characters or unclosed strings) but instead propagates a `ValuaScriptException`.

* **Error Structure:** Exceptions carry:
* `ErrorCategory::Lexical`
* `ErrorCode`: Specific code for the violation (e.g., `InvalidCharacter`, `UnclosedString`).
* `SourceLocation`: Including `line`, `column`, and `file_path`.

---

## 5. Performance Considerations

* **Time Complexity:** The tokenizer operates in $O(N)$ time, where $N$ is the total length of the source string. Each
  character is visited a constant number of times.
* **Memory Complexity:** The output is a `std::vector<Token>`. Memory usage is $O(T)$, where $T$ is the number of tokens
  generated.
* **Allocation:** The implementation relies on standard containers. While `std::string::substr` is used to capture
  lexemes, this is efficient for typical source code distributions.

---

## 6. Extension Guidelines

To extend the lexer (e.g., adding a new operator):

1. **Modify `TokenType`:** Add the entry to the `enum class` in `token.h`.
2. **Update `to_string`:** Add the string conversion entry for debugging.
3. **Update `get_keyword_type`:** If the new token is a keyword, register it in the `kKeywords` map.
4. **Implement Logic:** Add the character transition in `Lexer::scan_token` in `lexer_stage.cpp`. If the token is
   multi-character, use `match()` or `peek_next()` logic within the switch case.