## ValuaScript Parser Stage: Architecture and Design Documentation

This document outlines the design, architecture, and implementation details of the `ParserStage` component within the
ValuaScript compiler pipeline.

### 1. Architectural Overview

The ValuaScript parser is implemented as a **hand-written, top-down recursive descent parser**. It operates as an
independent stage within the compiler pipeline, inheriting from `CompilerStage`.

**Key Design Decisions:**

* **Encapsulation:** The core parsing logic is completely hidden inside an anonymous namespace within the implementation
  file (`parser_stage.cpp`). This prevents symbol leakage and keeps the header clean, exposing only the pipeline
  boundary (`ParserStage`).
* **Immutable Token Consumption:** The parser consumes a strictly read-only reference to a `std::vector<Token>`.
  Lookahead operations (`peek`, `check`) and consumption (`advance`, `match`, `consume`) are managed via a lightweight
  internal index (`current_`), ensuring $O(1)$ token access and avoiding unnecessary memory duplication.
* **Fail-Fast Error Handling:** The parser employs an exception-based error recovery model. Upon encountering illegal
  syntax, it immediately throws a heavily contextualized `ValuaScriptException` (anchored with exact line/column data
  from the offending token), halting AST generation.

### 2. AST Memory Model and Node Hierarchy

The Abstract Syntax Tree (AST) is heavily reliant on polymorphism and modern C++ smart pointers to enforce strict
ownership semantics and prevent memory leaks.

* **Tree Ownership:** The AST strictly uses `std::unique_ptr` for child nodes. This guarantees a directed acyclic
  graph (DAG) structure where the parent uniquely owns its children. It eliminates the overhead of reference counting (
  `std::shared_ptr`) during tree traversals, mapping perfectly to the lifecycle of an AST.
* **Polymorphic Base Classes:** * `AstNode`: The root interface.
* `Expression`: Represents nodes that evaluate to a value (e.g., `BinaryExpression`, `TensorLiteral`, `FunctionCall`).
* `Statement`: Represents operations that do not yield values (e.g., `Assignment`, `ReturnStatement`).


* **Zero-Copy Optimizations:** String values and identifier names are ingested via `std::move` from the token streams
  into the AST nodes (e.g., `StringLiteral`, `IdentifierAccess`).

### 3. Grammar Parsing and Precedence Rules

The recursive descent implementation inherently enforces operator precedence via the call stack. The parsing pipeline
stratifies expressions from lowest to highest precedence:

1. **Logical Operations:** `parse_or_expression` $\rightarrow$ `parse_and_expression`
2. **Relational/Comparison:** `parse_comparison_expression` (explicitly prevents operator chaining like `a < b < c` at
   the syntax level).
3. **Arithmetic:** `parse_addition_expression` $\rightarrow$ `parse_multiplication_expression`
4. **Exponentiation:** `parse_power_expression` (evaluated before multiplicative operators).
5. **Unary/Postfix:** `parse_unary_expression` $\rightarrow$ `parse_postfix_expression`
6. **Primary Atoms:** `parse_primary_expression` (literals, identifiers, grouping).

#### Advanced Language Constructs Supported

* **Structs & Types:** Supports structured data definitions (`StructDefinition`) and complex type annotations (
  `TypeAnnotation`), including generics (`Type<A, B>`) and tuple types (`(A, B)`).
* **First-Class Tensors:** Native parsing for tensor literals (`[a, b, c]`) and advanced slicing/indexing operations (
  `parse_tensor_access` handles `target[start:end]`).
* **Multiple Assignment & Return:** The grammar natively supports multiple return values (`ReturnStatement`) and
  destructuring/multiple assignments (`let a, b = ...`).

### 4. Compiler Pipeline Integration

The `ParserStage` adheres to the generic `CompilerStage` contract, ensuring loose coupling with the Lexer and subsequent
semantic analysis or code generation stages.

* **Inputs:** Validates and extracts `CompilerStageArtifactCode::TokenStream` and `CompilerStageArtifactCode::FilePath`
  from the artifact vector.
* **Outputs:** Packages the resulting `std::unique_ptr<Program>` into a dynamically typed or type-erased artifact tagged
  as `CompilerStageArtifactCode::Ast`.
* **Thread Safety:** The `ParserStage::run` method allocates a local `Parser` instance per invocation. Assuming the
  input artifacts are immutable, this design is thread-safe and allows parallel parsing of multiple compilation units in
  a multi-threaded compiler driver.