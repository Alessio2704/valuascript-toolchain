# Compiler Architecture: The Orchestrator Pattern

The compiler is built on a **Pipeline Architecture**. This design allows for extreme modularity, where new features (optimizations, linting, etc.) can be added as self-contained "stages."

## The `CompilerStage` Interface

Every stage in the compiler inherits from a base interface that defines a strict contract:

* **Metadata Requirement**: Every stage must declare its name, the artifact it produces, and a list of dependencies it requires to run.
* **The `run` Method**: A pure virtual function that consumes a read-only history of artifacts produced by previous stages and returns a new artifact.

## The `Orchestrator`

The `Orchestrator` is the central engine that manages the compilation lifecycle:

* **Ownership**: It holds a `std::vector` of `std::unique_ptr<CompilerStage>` objects, managing their memory automatically.
* **Blackboard Pattern**: It maintains a `pipeline_artifacts_` vector (a "blackboard") that stores the output of every stage. This vector is passed to subsequent stages, allowing them to access any previous result in the pipeline.
* **Linear Execution**: It iterates through the stages, feeding the accumulated history into each one and capturing the results sequentially.

## `Pipeline Integrity: The DAG Validator`

To prevent logical errors (such as running an optimizer before a parser), the `Orchestrator` includes a **Validation Engine**.

### Validation Logic:

* **Dependency Tracking**: Before the compiler starts, it performs a "dry run" using a `std::set` to track which artifacts are conceptually available.
* **Topological Verification**: For every stage, the validator checks if all its required dependencies exist in the `available_artifacts` set.
* **Fail-Fast Mechanism**: If a dependency is missing or the stages are in the wrong order, the system throws a `std::logic_error` with a detailed description of the violation, preventing the compiler from running in an invalid state.

## `Data Flow and Type Safety`

Handling data between stages (e.g., passing an Abstract Syntax Tree to a Semantic Analyzer) is handled through **Type-Safe Type Erasure**.

### Artifact Wrapper

The `CompilerStageArtifact` struct uses `std::any` to store diverse data types alongside a `CompilerStageArtifactCode` enum to identify what the data is.

### Type-Safe Extraction Utility

A template free function, `extract_artifact_data`, was implemented to simplify data retrieval:

* **Automatic Casting**: It searches the artifact history for a specific code and automatically performs a `std::any_cast` to the expected type.
* **Robust Error Handling**: It throws descriptive runtime errors if an artifact is missing or if there is a type mismatch, ensuring that implementation bugs are caught immediately.