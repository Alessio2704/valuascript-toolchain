## ValuaScript Abstract Syntax Tree (AST): Architecture and Memory Semantics

This document details the internal representation, node hierarchy, and memory ownership model of the ValuaScript
Abstract Syntax Tree (AST). The AST serves as the primary intermediate representation (IR) between the parsing and
semantic analysis phases.

---

### 1. Architectural Philosophy and Memory Model

The ValuaScript AST is engineered strictly as a **Directed Acyclic Graph (DAG)** with a strict tree topology (every node
has exactly one owner).

* **Strict Single Ownership:** The architecture enforces memory safety and lifecycle determinism through the exclusive
  use of `std::unique_ptr<T>` for all child nodes. There are no dangling references or circular dependencies. When the
  root `Program` node is destroyed, the entire tree cascades into deterministic destruction without requiring a garbage
  collector.
* **Zero-Copy Ingestion:** The parser transfers ownership of lexemes directly into the AST using `std::move()`. Fields
  like `std::string name` or `std::string value` in literals and identifiers are move-constructed, eliminating redundant
  heap allocations for strings during tree generation.
* **Expression vs. Statement Bifurcation:** The AST strictly categorizes nodes into `Expression` (computations that
  yield a value) and `Statement` (operations that produce side effects).

---

### 2. Core Node Hierarchy

The base of the tree relies on a purely virtual polymorphic interface.

| Base Class   | Role                                                                                                   | Memory Footprint Characteristics                       |
|--------------|--------------------------------------------------------------------------------------------------------|--------------------------------------------------------|
| `AstNode`    | Root virtual interface. Guarantees proper polymorphic destruction via `virtual ~AstNode() = default;`. | 8 bytes (vtable pointer).                              |
| `Expression` | Inherits from `AstNode`. Represents nodes that can be evaluated to a typed value.                      | Inherits vptr. Heavily recursive structures.           |
| `Statement`  | Inherits from `AstNode`. Represents side-effecting operations, control flow jumps, or bindings.        | Inherits vptr. Typically holds vectors of expressions. |

---

### 3. Expression Node Semantics

Expressions represent the core computational fabric of ValuaScript. The language treats several constructs as
first-class expressions that might be statements in older paradigms.

#### 3.1 Primitives and Identifiers

* **Literals (`NumberLiteral`, `StringLiteral`, `BooleanLiteral`):** Terminal leaf nodes. They encapsulate primitive
  state directly.
* **`IdentifierAccess`:** Represents a read operation against the current lexical environment.

#### 3.2 Compound and Control Expressions

* **`BinaryExpression` / `UnaryExpression`:** Standard arithmetic and logical operations. Op-codes are stored as raw
  `TokenType` enumerations to maintain high cache locality and fast switch-dispatch during evaluation/compilation.
* **`ConditionalExpression`:** ValuaScript treats `if/then/else` as an expression (similar to Rust or Kotlin), allowing
  constructs like variable assignment directly from a conditional branch. It owns three discrete `Expression` sub-trees.

#### 3.3 Tensors and Dictionaries (First-Class Data Structures)

* **`TensorLiteral` / `TupleLiteral`:** Represented as contiguous `std::vector<std::unique_ptr<Expression>>`.
* **`DictLiteral`:** Maintains insertion order and relationships via
  `std::vector<std::pair<std::string, std::unique_ptr<Expression>>>`. Note that keys are restricted to statically known
  strings at the AST level, optimizing property access lookups.
* **`TensorAccess`:** Handles both direct indexing (`tensor[0]`) and slice generation. The parser creatively overloads
  `BinaryExpression` with a `TokenType::Colon` to represent the `[start:end]` bounds within the `index` property.

#### 3.4 Function Calls and Named Arguments

* **`FunctionCall`:** Supports complex invocation semantics. Arguments are stored as
  `std::pair<std::string, std::unique_ptr<Expression>>`, natively supporting named/keyword arguments right out of the
  parser stage.

---

### 4. Statement and Declaration Nodes

Declarations structure the execution environment and define custom types.

#### 4.1 Type System Representation

* **`TypeAnnotation` & `TupleTypeAnnotation`:** The AST models a robust static type system. `TypeAnnotation` supports
  generics via the `generic_args` vector (e.g., `Result<T, E>`).

#### 4.2 Assignments and Returns

* **`Assignment`:** Natively supports destructuring and multiple returns. The `targets` field is a
  `std::vector<std::string>`, allowing `let a, b = compute()`.
* **`ReturnStatement`:** Mirrors assignment by accepting a `std::vector<std::unique_ptr<Expression>>` to support
  multi-value returns out of functions.

#### 4.3 High-Level Declarations

* **`StructDefinition`:** Defines contiguous memory layouts. Fields bind string identifiers directly to `TypeAnnotation`
  sub-trees.
* **`FunctionDefinition`:** A heavy node encapsulating the function signature (`name`, `parameters`, `return_types`),
  the local block `body`, and an optional `docstring` for native tooling/LSP support.

---

### 5. The Root Node: `Program`

The `Program` node is the ultimate owner of the compilation unit. It organizes the global scope into logically separated
vectors:

1. `import_statements`: Dependency resolution.
2. `directives`: Compiler/Runtime metadata (e.g., `@inline`, `@optimize`).
3. `struct_definitions`: Type registry population.
4. `function_definitions`: Global executable code.
5. `execution_steps`: Top-level script evaluations (e.g., global `let` bindings).

This segmented structure allows subsequent compiler passes (like hoisting or type-checking) to iterate over declarations
in a highly predictable, cache-friendly manner without needing to filter a generic list of "top-level nodes."
