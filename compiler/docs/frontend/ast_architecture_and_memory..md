# ValuaScript Abstract Syntax Tree (AST): Architecture & Memory Mechanics

This document details the architectural philosophy, memory management strategy, and structural mechanics of the
ValuaScript Abstract Syntax Tree (AST). It explains not just how the tree is built, but *why* it is built this way,
addressing the fundamental challenges of representing recursive grammatical structures in memory.

---

## 1. The Core Challenge: Dynamic Sizing and Object Slicing

An AST is a hierarchical representation of source code. A parent node (like a `BinaryExpression`) must hold references
to its child nodes (the `left` and `right` operands).

The fundamental problem in C++ is that the compiler must know the exact memory size of an object at compile time.

If we attempted to store child nodes by value:

```cpp
class BinaryExpression : public Expression {
public:
    Expression left;  // ERROR: Compiler does not know the size of 'Expression'
    Expression right; 
};

```

Because `Expression` is an abstract base class, it has no fixed size. A `NumberLiteral` might require 24 bytes, while a
`FunctionCall` might require 64 bytes.

Furthermore, if C++ allowed this, assigning a `FunctionCall` to an `Expression` variable would trigger **Object Slicing
**. The C++ compiler would literally slice off all the specialized data (like the function's argument array), leaving
only the empty base `Expression` shell.

**The Solution:** We must use pointers. A pointer on a 64-bit architecture is always exactly 8 bytes, regardless of
whether it points to a tiny boolean or a massive, deeply nested function body.

---

## 2. The ValuaScript Solution: Smart Pointers & Polymorphism

Historically, compilers used raw pointers (`Expression* left`), which required manual `new` and `delete` calls. If the
parser crashed midway due to a syntax error, any orphaned nodes caused massive memory leaks.

ValuaScript solves this using **Modern C++ Smart Pointers**, achieving absolute memory safety with zero runtime
overhead.

### 2.1 `std::unique_ptr` (Exclusive Ownership)

Almost every node in the ValuaScript AST is wrapped in a `std::unique_ptr`.

An AST is a strict, acyclic hierarchy. A node has exactly *one* parent. The `left` side of `1 + 2` belongs exclusively
to the `+` node. `std::unique_ptr` perfectly enforces this architectural reality in memory.

* **Zero Overhead:** It is exactly the same size as a raw pointer.
* **Automatic Cleanup (RAII):** When a parent node is destroyed, its `unique_ptr` fields automatically delete their
  children. Those children delete their children. The entire tree cleans itself up instantly, even during an exception.

### 2.2 `std::move()` (The Baton Pass)

Because `std::unique_ptr` enforces exclusive ownership, it cannot be copied. This dictates the mechanics of our
Recursive Descent parser.

When the parser builds the tree bottom-up, it transfers ownership up the call stack using `std::move()`:

```cpp
std::unique_ptr<Expression> left_node = parse_atom(); 
Token op = previous();
std::unique_ptr<Expression> right_node = parse_atom();

// Transfer ownership to the parent. The local pointers become null.
return std::make_unique<BinaryExpression>(
    std::move(left_node), 
    std::move(right_node), 
    op.type
);

```

### 2.3 `std::shared_ptr` (Pipeline Lifecycle)

While internal nodes have 1:1 ownership, the **root node** (the `Program`) represents the entire compiled file. It must
be passed through the compiler's pipeline (Parser -> Semantic Analyzer -> Code Generator).

By wrapping the root `Program` in a `std::shared_ptr`, multiple compiler stages can reference the tree simultaneously.
Once the final stage finishes and the reference count drops to zero, the root deletes itself, triggering the
`unique_ptr` cascade that safely destroys the entire AST.

---

## 3. Why Trees Are Inherently Difficult

Constructing the tree is only half the battle. The true difficulty in compiler design lies in navigating it. There are
three core challenges:

### 3.1 The Traversal Problem (The N-Dimensional Maze)

You cannot use a simple `for` loop on a tree. You must write recursive algorithms to walk the branches, and the order of
visitation changes the entire logic of the compiler:

* **Pre-order (Parent first, then children):** Used for scope resolution. You must register a function's parameters in
  the Symbol Table *before* analyzing the function's body.
* **Post-order (Children first, then parent):** Used for execution/evaluation. You cannot evaluate a `*` node until you
  have fully traversed to the bottom leaves and resolved their mathematical values.

### 3.2 Type Blindness (The Cost of Polymorphism)

Because our tree uses polymorphic base classes (`std::unique_ptr<Expression>`), the compiler is "blind" to the actual
node types during traversal. When the analyzer looks at `node->left`, it does not know if it is looking at a
`NumberLiteral` or a `FunctionCall`.

This requires safe downcasting (`dynamic_cast`) or structural patterns like the **Visitor Pattern** to force nodes to
reveal their true identities, adding complexity to the Semantic Analyzer.

### 3.3 The Call Stack Limit (Stack Overflow)

Because we navigate trees using recursive functions, we are bound by the CPU's hardware call stack limit. If a user
writes an edge-case file that concatenates 100,000 strings (`"a" + "b" + "c"...`), the tree leans 100,000 nodes deep.
Walking this tree requires 100,000 nested function calls, which will crash the operating system with a Stack Overflow.

---

## 4. Alternative AST Architectures

If we did not use OOP Polymorphism (base classes and virtual functions), there are three professional alternatives used
in compiler engineering:

### Alternative 1: Algebraic Data Types (`std::variant`)

Similar to Swift `enums`, this approach eliminates base classes entirely. You define a strict, type-safe box that can
hold exactly one of the known node structs.

* **Navigation:** Uses compile-time pattern matching (`std::visit`) instead of `dynamic_cast`.
* **Pros:** Extremely type-safe; forces the developer to handle every node type.
* **Cons:** Defining recursive variants in C++ requires clumsy wrapper structs.

### Alternative 2: The Tagged Union (The C-Compiler Way)

The historic approach used by early C compilers. One massive `Node` struct contains an `enum` indicating its type (the "
Tag") and a memory `union` overlapping the specific data fields.

* **Navigation:** A massive `switch(node->tag)` statement.
* **Pros:** Very fast memory allocation; no virtual table overhead.
* **Cons:** Highly unsafe. Reading the wrong union field results in silent memory corruption.

### Alternative 3: Data-Oriented Design (The ECS Way)

Used by cutting-edge compilers (like Zig) and game engines. Pointers are eliminated completely. Nodes are stored as flat
`structs` in giant contiguous arrays (`std::vector`). An "AST Node" is simply an integer index pointing to a slot in an
array.

* **Navigation:** Array lookups (e.g., `ast.binary_nodes[4]`).
* **Pros:** The absolute maximum performance possible due to perfect CPU L1 cache utilization. Eliminates Stack Overflow
  risks because trees can be traversed with flat `for` loops.
* **Cons:** Extremely difficult to maintain. Inserting or removing a node requires shifting arrays and recalculating
  thousands of integer indices.

### Conclusion: The ValuaScript Choice

ValuaScript utilizes **Polymorphism + Smart Pointers** because it offers the perfect balance of mathematical intuition,
infinite extensibility, and default memory safety, allowing rapid and safe evolution of the language's grammar.