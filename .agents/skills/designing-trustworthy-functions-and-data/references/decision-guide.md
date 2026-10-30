# Function Design Decision Guide

Use this guide when several designs appear reasonable. The goal is not maximum purity, abstraction, or type strength. The goal is the smallest truthful contract that preserves the system's real semantics.

## Parse or Validate?

Ask what information the check learns and where that information must remain available.

| Situation | Default design |
| --- | --- |
| Raw input becomes a reusable domain value | Fallible parser `→ Domain ⇥ Error` returning an invariant-bearing type |
| A range or length bound is the whole invariant | Ranged integer or sized list type; parse into it at the boundary |
| A collection has a stable structural invariant | Fallible constructor function returning a class with `@ private` fields |
| The fact matters only during one local calculation | Local checked branch; do not invent a durable type |
| The fact depends on mutable external state | Transaction, guard, lease, version, or stateful operation that keeps proof tied to reality |
| The type cannot preserve the claimed property | Return a weaker type or keep an explicit runtime result |

Prefer a `parse_*` function, an `@ conversion fn` behind `↦`, or a JSON/file reader that returns the stronger type. A function such as `validate(value) → void ⇥ Error` is usually incomplete if callers continue passing `value` around in its original weak form.

Before adding a value type, ask:

- Does the guarantee travel through several functions, fields, returns, or collections?
- Will accepting the stronger type remove checks or `⇥` variants downstream?
- Is the property intrinsic and stable for the value's lifetime?
- Can a ranged or sized field type enforce it, or must the parser be the only construction site by convention?

If the guarantee exists only inside one private aggregate, parsing directly into that aggregate may be simpler. If an operation returns the element and downstream code benefits from knowing the guarantee, an element type is usually earned.

## Dependency Shape

| What the operation needs | Default shape | Escalate when |
| --- | --- | --- |
| One current fact | Ordinary value such as `instant<ns> now`, an id, or config | The component legitimately obtains it repeatedly or owns its policy. |
| One narrow behavior | Closure parameter such as `(T) → R` | It has a stable multi-operation capability or stateful lifecycle. |
| A stream of values | `generator` function, consumed with `for from` | The consumer must also control errors, flushing, or backpressure. |
| A sequence of nondeterministic draws | Caller-owned stateful value passed as `mut` (a seeded `Rng`) | Several real sources must be interchangeable. |
| Deterministic in-process collaborator | Concrete class or direct function | Multiple real implementations or ownership boundaries exist. |
| External authority or durable capability | Consumer-owned `interface` or a concrete adapter class | A Norma type already communicates the contract better. |

Prefer values over services and existing types over new interfaces. Norma's effect modules are plain functions, so the honest seam is usually a value read at the boundary. Do not make a dependency substitutable unless substitution improves control or evidence.

## Effect Placement

Ask these questions in order:

1. Can the domain decision be expressed from ordinary supplied values?
2. Which component owns acquiring those values?
3. Must a read and later write observe the same transaction, lock, version, stream, or actor state?
4. Can cancellation, retries, partial failure, or re-entry occur between the proposed layers?
5. Does moving the effect change ordering, atomicity, resource lifetime, or performance?

Separate deterministic calculation from action when the answers permit it. Otherwise keep the decision inside the narrow effectful boundary and test that boundary honestly.

## Invariant Type Test

Introduce a dedicated type only when most answers are yes:

- Would violating the invariant cause a meaningful bug or security/correctness failure?
- Is the invariant stable in the domain rather than incidental to one algorithm?
- Can all ordinary constructors establish it?
- Can every exposed method preserve it or return a weaker type?
- Can JSON, file, storage restoration, and imported input be parsed through it?
- Will downstream code become materially simpler or safer?
- Is the parser ergonomic enough that callers will not write the literal directly?
- Has the mathematical, lifecycle, or domain claim been independently verified?
- Does construction return the stronger type rather than merely reporting that the weaker input passed?
- Will downstream signatures use the stronger type so the proof is not immediately erased?

Use a checked function, a `require` / `reject` guard at an honest boundary, or an explicit validation result when a wrapper would proliferate states, block legitimate restoration, or certify a condition that can become stale.

## Input Strength

Choose from semantic requirements, not abstract generality:

| Requirement | Likely Faber shape |
| --- | --- |
| Read-only access | No marker (`ref` is the default) |
| Change the caller's value | `mut` parameter, repeated at the call: `f(mut x)` |
| Consume or retain the value | `own` parameter, repeated at the call: `f(own x)` |
| A sequence to walk or index | `list<T>` |
| A sequence with a length contract | `list<T, size 1…∞>`, `list<T, size 64…64>` |
| A number with a domain range | `trapping<W, lo…hi>`; `saturating` or `wrapping` only when clamping or wrap-around is the policy |
| Membership or association | `set<T>`, `map<K, V>` keyed by a primitive or ranged type (class keys fail at run time in 1.17) |
| Domain behavior and invariants | A cohesive class |
| A small subset of unrelated state | Individual values or a narrow interface |
| Many coherent options | An options class with explicit fields and `enum` choices |

Do not ask for `mut` or `own` out of habit; `faber check` flags `WARN008 unused_mut_parameter` and `WARN009 unused_own_parameter` (pass an owned value on with `items.append(own item)`). `WARN008` also fires, falsely, when the only change is a mutating method call. Do not drop a length, range, or ownership requirement merely to accept more callers.

## Extraction Test

A new function is usually justified when at least one is true:

- Its name adds domain or algorithmic vocabulary.
- It centralizes a policy or invariant.
- It separates a meaningful boundary or effect.
- It hides lower-level mechanics from a higher-level operation.
- It forms a stable independently testable contract.
- It replaces repeated logic whose copies could diverge.

Keep the code local when the proposed helper only:

- aliases a field access or obvious expression
- forwards arguments unchanged
- exists to reduce line count
- exposes private choreography for a fake
- requires a reader to chase files to understand one short operation

Comments that divide a function into phases are a review signal, not automatic proof of bad design. A transaction, parser, protocol, or performance-sensitive loop may be clearer as one auditable scope.

## Representation and Invariant Ownership

Before extracting helpers around an existing representation:

1. State the operation the domain needs without naming the current container or algorithm.
2. List every policy the implementation must remember: validation, canonicalization, uniqueness, ordering, searching, indexing, or restoration.
3. Remove policies that exist only because of the current representation.
4. Assign each remaining invariant to the smallest natural owner.
5. Prefer a domain or ranged type for per-value validity and a built-in collection for relationships between values.
6. Compose those owners when neither can enforce the complete contract alone.
7. Recheck every construction and restoration path; the representation must not be bypassed by JSON loading, bulk import, or a construction literal in another module.

Do not stop at the first local improvement. A built-in sort can remove handwritten ordering while leaving normalization as a convention. A map can remove sorting while leaving string keys invalid. A validated key can eliminate invalid names while leaving lookup encoded as a scan. Continue until each meaningful invariant has an explicit owner and the obsolete policy machinery can be deleted.

Do not add a class merely to hide a built-in collection. An outer domain collection is earned when it owns additional operations, cross-entry invariants, persistence validation, or authority. Otherwise, composing an invariant-bearing key directly with `map` or `set` may be the clearest complete design, as `set<ServicePort>` does.

## Output and Materialization

- Return a complete value when it naturally represents a bounded result.
- Return a `generator` when the result can be large, incremental, or consumed partially; it is lazy, so the caller can `break` early.
- Accept a closure when inversion of consumption is intentional and its error and ordering behavior is defined. A closure cannot throw on the caller's `⇥` channel.
- Mutate an explicitly supplied `mut` value when mutation is part of the honest contract and avoids misleading copies or allocations.

Check asymptotic payload size as well as container count. Storing all permutations of an `n`-item input requires `n!` results and usually `Theta(n * n!)` payload storage.

## Final Complexity Budget

For every new abstraction, complete this sentence:

> This exists because it ______, and a direct implementation would ______.

If the answer is only "makes faking possible," "is cleaner," "might be reused," or "makes the function shorter," the abstraction has not yet justified itself.
