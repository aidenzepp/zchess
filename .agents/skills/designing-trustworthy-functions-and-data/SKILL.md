---
name: designing-trustworthy-functions-and-data
description: Use when designing, implementing, refactoring, or reviewing functions, domain values, data structures, and APIs with hidden dependencies, discarded validation knowledge, invalid states, mixed abstraction levels, excessive mocking or interfaces, unclear effect boundaries, primitive-heavy signatures, or code that is difficult to test and understand locally.
---

# Designing Trustworthy Functions and Data

A trustworthy design makes functions and data tell the same truth. Functions expose what they need, can change, return, and may fail to do. Data preserves facts the system has already established so downstream functions do not re-check them or carry impossible error branches.

The governing balance is:

> Parse raw input into the smallest representation that preserves what was learned, then make every function ask for exactly the data, authority, and guarantees it requires.

Code here is Faber 1.17; the `faber` skill covers syntax and diagnostics.

## Know Whether the Function Is Honest

"Honest" and "dishonest" are the source video's pragmatic terms, not moral judgments and not Faber terminology.

An **honest function** exposes every operationally meaningful input, output, mutation, and external capability through its signature. A caller can see what determines the result and what the operation may affect without opening the body.

A **dishonest function** reaches beyond its signature for meaningful state or authority: the clock, the process-wide random generator, environment variables, the filesystem, the network, the console, a service locator, or a hidden cache. Its apparent contract is smaller than its real contract.

Honesty is related to purity, but they ask different questions:

| Question | Concern |
| --- | --- |
| Pure or impure? | Does evaluation have observable side effects, and is the result referentially transparent? |
| Honest or dishonest? | Does the signature reveal the operation's real inputs, outputs, effects, and authority? |

Therefore:

- `fn total(list<int> values) → int` can be pure and honest.
- `fn sort_by_priority(mut list<Task> tasks) → void` is impure but honest: `mut` declares the mutation, and every call repeats it.
- `fn write_receipt(mut Output output, Receipt receipt) → void ⇥ string` is impure but honest about where it writes.
- A function that calls Norma's `time.now()` is dishonest under this model because the clock input is hidden.
- `fn populate(mut ParticleWorld world, int count) → void` is dishonest if it calls `random.random_int`, which draws from one process-wide generator; accepting `mut Rng rng` makes it honest, but not pure.

Faber removes some dishonesty outright: an unmarked parameter is `ref`, read-only all the way down, and module-level mutable state does not exist (`SEM062 top_level_binding`). What remains is ambient authority reached through Norma effect modules (`time`, `random`, `process`, `console`, `fs`, `http`, `net`) and objects passed for one reason and used for another.

Honesty does not guarantee determinism, safety, correctness, or faithful tests. An explicitly passed clock, generator, callback, or transaction still performs real effects. Honesty makes those effects visible so ownership, testing, and reasoning can be deliberate.

Dishonesty is also contagious: a function that calls a hidden clock acquires that undeclared dependency itself. Move the dependency into the signature or keep the call inside a deliberately effectful boundary.

Do not try to eliminate all dishonesty. Programs must eventually read clocks, access files, use networks, and affect the world. Contain that necessary dishonesty in small boundary adapters, usually `main` or `main args`, then pass explicit values or capabilities inward—unless a transaction, lock, stream, retry, or durable workflow requires the effect and decision to stay together.

Draw the line at operationally meaningful dependencies. Do not pollute signatures with incidental physical influences such as allocation, scheduler timing that is irrelevant to the contract, or CPU temperature.

- Read [references/examples.md](references/examples.md) when implementing or refactoring.
- Read [references/non-examples.md](references/non-examples.md) when reviewing abstractions, dependency injection, strong types, parameter objects, or function extraction.
- Read [references/decision-guide.md](references/decision-guide.md) when the correct effect boundary, input shape, invariant owner, or extraction boundary is uncertain.
- Read [references/design-transformations.md](references/design-transformations.md) for paired Faber refactors from misleading or overpowered functions to trustworthy designs.
- Read [references/parse-dont-validate.md](references/parse-dont-validate.md) when validation returns `void`, checks recur, domain functions retain impossible error cases, or raw primitives cross trusted boundaries.
- Read [references/video-notes.md](references/video-notes.md) for the source video, timestamped notes, qualifications, and corrections behind this skill.

## Design the Contract Before the Body

Start with the call site that should exist. State the operation in one sentence, then inventory:

- required values and semantic guarantees
- `mut` inputs, `own` inputs, and observable outputs
- the `⇥` payload, `∪ none` absence, and partial failure
- preconditions, postconditions, and preserved invariants
- time, randomness, environment, filesystem, network, process, or console access
- ownership, ordering, transaction, cancellation, backpressure, and performance requirements

Do not make the signature describe the current implementation by accident. Make it describe the durable contract a caller must understand.

## Make Dependencies and Effects Visible

Keep deterministic domain decisions separate from environmental work when that separation preserves the real semantics. Entry points should translate operands and environment into application-owned operations quickly.

Prefer the smallest honest input:

1. Pass an ordinary value when the function only needs a snapshot such as `instant<ns> now`, an identifier, or configuration.
2. Pass a closure such as `(int) → int step` for one narrow operation that genuinely varies.
3. Pass a caller-owned stateful value as `mut` when the operation consumes a sequence, such as a seeded `Rng`.
4. Keep a concrete collaborator when substitution adds no useful control.
5. Define a consumer-owned `interface` when there is a stable capability, real alternative implementations, or external authority to isolate. Norma's effect modules are plain functions, not capability values, so this is often the honest seam for output or storage.

Wire environmental implementations at a visible composition boundary. A service locator, generic context, or dependency bag hides dependencies even when it is passed as an argument.

Place effects at the narrowest correct consistency boundary—not mechanically at the root of the call tree. Keep reads, decisions, and writes together when a transaction, lock, stream, retry, or durable workflow requires their ordering or shared lifetime.

## Ask for the Right Amount of Power

The parameter marker is the first power dial: no marker (`ref`) to read, `mut` to change the caller's value (`WARN008 unused_mut_parameter` flags one that is never assigned), `own` to consume or keep it (later use is `SEM050 use_after_own`). Then choose the shape:

- `list<T>` to walk or index a sequence; `list<T, size 1…∞>` or `list<b8, size 64…64>` when length is part of the contract
- `set<T>` or `map<K, V>` when membership, uniqueness, or association is the contract
- a cohesive domain class when its behavior or invariants matter
- individual values when passing the whole object would expose unrelated authority

Do not weaken a parameter merely because today's body compiles against it. Multiple passes, ordering, ownership, fixed length, and asymptotic cost are legitimate requirements.

Use a parameter object when its fields form one coherent concept or meaningful option set. It is not a cure for an operation that owns too many unrelated responsibilities.

## Parse at Boundaries and Preserve What You Learn

Treat parsing broadly: it transforms less-structured input into more-structured data or fails. The input may be text, a primitive, a list, a JSON `value`, a CLI operand, or a value imported from a target-language module.

Prefer:

```text
raw input -> fallible parser (→ T ⇥ E) -> invariant-bearing value -> infallible domain operations
```

over:

```text
raw input -> validate (→ void ⇥ E) -> keep using raw input -> re-check or propagate impossible errors
```

A `validate_*` or `is_valid` function that returns no refined value is a design smell when the caller continues using the original weak type. The check learned something, but the program discarded the proof. Return a type that preserves the fact, or parse directly into the aggregate that owns it.

Choose the proof owner deliberately:

- Use a value type when the guarantee is intrinsic to the value and should travel through returns, fields, collections, and APIs. Prefer a ranged or sized type (`trapping<u16, 1024…65535>`, `list<T, size 1…∞>`) when it can express the fact: every store re-checks it.
- Use a private aggregate when the rule is meaningful only as membership or state inside that aggregate and every construction and mutation path remains sealed.
- Use a transaction, guard, lease, or stateful operation when the fact depends on mutable external state and can become stale. Do not encode a temporary observation as a permanent value guarantee.

Keep fallibility at raw boundaries such as CLI operands, configuration, HTTP, files, JSON, and imported target-language values. Once parsing succeeds, core operations should not keep the same `⇥` payload unless they genuinely accept weaker input again.

Parsing does not justify a wrapper automatically. The parsed representation must remove checks, prevent misuse, clarify meaning, or make downstream operations total. Avoid generic `Validated<T>` markers that fail to name the established fact.

## Encode Only Invariants the Design Can Preserve

A dedicated type is earned when an invariant is consequential, stable, repeatedly needed, and enforceable through every construction and mutation path.

For an invariant-bearing type:

- put the invariant in a field's type when a ranged integer or sized list can express it
- otherwise mark the representation `@ private` and expose methods for legitimate access
- establish it in one `parse_*` or `@ conversion` function with a precise `union` payload on `⇥`, and route JSON, file, and imported input through it
- expose only operations that preserve the invariant, or return a weaker type
- test construction, rejection, preservation, and bypass paths

Faber 1.17 does not seal construction: `@ private` blocks outside reads, writes, and method calls, but any importing module can still write the construction literal. When the invariant cannot live in a field's type, keep the parser the only construction site by convention, say so in the doc comment, and look for stray literals in review.

Once constructed, functions should accept that type and omit redundant checks and impossible error variants.

Never strengthen a parameter or return type because a theorem, lifecycle fact, or domain assumption merely sounds plausible. Verify that the implementation can prove and preserve the claim. Prefer runtime validation or a weaker result when the type would encode a lie.

## Keep One Conceptual Level Per Function

A function may orchestrate several sibling operations or implement one detailed operation. It should not repeatedly alternate between domain intent and low-level bits, indices, protocol mechanics, storage representation, or handwritten algorithms.

Extract a cohesive block when the new function:

- names a real operation or policy
- encapsulates an invariant or representation rule
- isolates a meaningful effect
- removes distracting lower-level machinery
- creates a useful contract or testing seam
- has meaningful reuse

Keep short local control flow together when extraction would only add navigation. A helper whose name restates its body has not created an abstraction.

Prefer Faber's built-in operations (`sort via`, `set`, `map`, `filter`) and Norma over reimplementing algorithms inside domain code. Repeated normalization, ordering, lookup, or parsing rules may reveal a missing data structure rather than a need for more scattered helpers.

## Let the Representation Enforce the Contract

Do not assume the current representation deserves to survive the refactor. Before improving its helpers or algorithms, separate:

- behavior the domain actually requires
- constraints imposed only by the current representation
- per-value invariants such as validity, canonical form, range, or units
- relationship invariants such as uniqueness, association, ordering, or queue discipline

Choose the smallest natural owner for each real invariant. A domain type or ranged type can make each value valid by construction. A built-in collection can provide relationships and operations such as unique-key lookup. Compose those owners when the contract contains both kinds of invariant; solving one does not solve the other.

For example, replacing a sorted list with a `map` can remove sorting and search policy, but a `map<string, AssetType>` still does not prove that its keys are valid or canonical. Conversely, a validated key type does not by itself provide lookup. A class whose `@ private` map is reachable only through methods that accept `AssetName` lets both do the work they naturally own.

Key maps and sets by primitives or ranged types. In 1.17 a class key passes `faber check` but fails at run time (`value is not a valid map/set key`); the Rust target does not compile it and the Go target silently misses lookups.

Prefer this structural composition over comments, call-site conventions, duplicated normalization, defensive checks, wrapper ladders, or custom algorithms. Add an outer domain collection class only when it owns additional behavior or policy beyond what its field types already guarantee.

Judge simplicity by the total policy burden left in the system, not by the number of types. One small invariant-bearing type plus one built-in collection may eliminate more complexity than a primitive collection surrounded by helpers.

## Make Every Abstraction Earn Its Cost

Before keeping a new function, wrapper, `interface`, generic, parameter object, closure, fake, or plan type, name what it buys:

- a truthful contract
- prevention of an important invalid state
- controlled nondeterminism or external authority
- preservation of ownership or consistency
- reusable policy
- materially better local reasoning or testing

If it buys none of these, remove it. Test deterministic code through values and results before adding fakes. Test files, networks, processes, and storage with the highest-fidelity safe implementation practical; a fake class behind an interface does not reproduce external semantics.

## Review the Result

Before completion:

1. Read the call site without opening the implementation. Confirm that intent and required guarantees are apparent.
2. Confirm every meaningful dependency and effect is visible: no buried Norma effect call, no service locator.
3. Confirm the function receives no unrelated authority and no unused `mut` or `own`.
4. Confirm every strong type states a true invariant that every construction path, including stray literals, preserves.
5. Search for validation returning `void`, `bool` validity checks, and repeated `require` / `reject` guards. Confirm each learned fact is preserved in a type or intentionally confined to an aggregate or live consistency boundary.
6. Confirm `⇥` channels appear where weak input becomes strong data, not throughout functions that already receive proven values.
7. Name the owner of every remaining invariant. Reject invariants owned only by comments, duplicated helpers, or caller discipline.
8. Confirm the representation does not preserve accidental requirements such as sorting or indexing when the domain only needs association or lookup.
9. Confirm transaction, ownership, ordering, cancellation, and performance behavior survived decomposition.
10. Remove mechanical helpers, speculative interfaces, wrapper ladders, and interaction-only test seams.
11. Write `describe` / `test` cases for the contract, invariant boundaries, deterministic core, and effectful adapters at appropriate fidelity.
12. Run `faber format .`, `faber check .`, and `faber test .`.
