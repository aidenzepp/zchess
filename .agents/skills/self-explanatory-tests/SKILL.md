---
name: self-explanatory-tests
description: Use when writing, reviewing, organizing, or debugging tests, assessing test completeness, or shaping types and dependency boundaries so important behavior can be tested honestly in this workspace.
---

# Self-Explanatory Tests

Tests are executable contracts for future maintainers and agents. A failure should identify the broken behavior without requiring the reader to reconstruct what the original author intended.

- Read [references/examples.md](references/examples.md) when authoring or reorganizing tests.
- Read [references/non-examples.md](references/non-examples.md) when reviewing tests, diagnosing brittle coverage, or considering a test double or test modifier.
- Read [references/design-and-seams.md](references/design-and-seams.md) when tests expose a weak invariant, hidden dependency, awkward abstraction, or uncertain coverage boundary.
- Read [references/sources.md](references/sources.md) when the policy or test strategy is disputed.
- Use `faber` before writing or reviewing `.fab` or `.proba` source.
- Use `designing-trustworthy-functions-and-data` before changing production types, dependencies, effects, or function boundaries in response to test pressure.

## Name the Contract First

Do not write a test until you can state the behavior it protects and the evidence for that expectation. Ground truth can come from an explicit user decision, public API or CLI contract, schema, verified documentation, existing behavior, or a reproduced bug. Do not encode guessed behavior.

Before RED, identify:

- the behavior contract
- the smallest stable boundary that exposes it
- inputs and observable outputs, effects, or errors
- the highest-fidelity deterministic dependency available
- what the test intentionally does not protect
- the command that will prove RED and GREEN

For a small unit test, the test name and arrangement may make this packet self-evident. Record it explicitly for bug regressions, public-contract suites, ambiguous behavior, or test-double use.

## Map the Test Obligations

Do not equate completeness with one test per function, class, or module. Completeness means every meaningful promise has an owner, an enforcement mechanism, and credible evidence.

Inventory the obligations relevant to the change or system:

- domain rules, invariants, assumptions, and preconditions
- accepted, rejected, boundary, degenerate, and malformed inputs
- state transitions, lifecycle rules, side effects, and required non-effects
- success, failure, partial failure, retry, recovery, and cancellation
- persistence, serialization, parsing, and round trips
- ordering, duplication, idempotence, concurrency, and resource limits
- permissions, privacy, security, and unsafe boundaries
- time, randomness, environment, filesystem, network, and process behavior
- compatibility, migration, performance, and error contracts when they are part of the promise

Scale the inventory to the work. A small pure function may need only a few cases; a stateful or external workflow needs an explicit obligation map. For consequential work, record:

| Obligation | Owner | Enforcement | Test evidence | Remaining gap or status |
| --- | --- | --- | --- | --- |

An obligation may be covered, not applicable with a reason, or deliberately deferred with a named risk and explicit approval. Do not silently omit it. The compiler already refuses missing class fields, non-exhaustive `match`, and unhandled `⇥` calls, and a store outside a `trapping<W, lo‥hi>` range traps; do not test those impossible states. Test the boundary that turns untrusted input into those types.

## Let Tests Pressure the Design

Tests provide evidence that an invariant is enforced; they should not be the only thing enforcing it.

When a test must repeatedly establish or check the same assumption:

1. Name the invariant and the type or component that should own it.
2. Ask whether invalid states can be made unrepresentable with a ranged width, a sized list, a `union` variant, or a field layout that cannot hold the bad combination.
3. Otherwise, validate once at the earliest trustworthy boundary with `require` or `reject` on a `⇥` function, and carry a trusted value inward.
4. Make later functions accept the trusted type instead of raw input.
5. Test construction, rejection, invariant preservation, and every path that could bypass the boundary, including parsing and restoration.

A `class` literal is legal wherever the class is visible, even with `@ private` fields, so a fallible constructor is a convention, not a seal. Prefer field types that cannot hold the bad state.

Repeated validation, elaborate fixtures, or inaccessible behavior are design evidence. Look for a missing constructor, parser, state `union`, domain type, or dependency boundary before adding more test machinery. If redesign is outside the authorized scope, add the best regression test available and record the unenforced assumption as design debt; do not leave both the design and the behavior unprotected.

Test pressure does not authorize a mechanically purer architecture. Preserve transactions, ownership, ordering, cancellation, backpressure, and external-system semantics. Use `designing-trustworthy-functions-and-data` to select the production boundary, then use this skill to prove its contract.

## Choose the Test Boundary

- Put unit tests in a `describe` block after the declarations they exercise, in the same `.fab` module. An inline suite can reach module-private items; that is not permission to freeze them.
- Use a colocated `*.proba` file for public-contract suites. It sees only the `@ public` surface (`SEM006 import_module_private` otherwise), runs only under `faber test`, and is never importable from product code.
- Do not create a `.proba` file merely to move tests away from code.
- Test observable behavior through a boundary that survives a behavior-preserving refactor. If renaming a helper would break the test while behavior stays the same, the test is too coupled.
- For a bug, reproduce the symptom, trace it to the deepest violated promise, and protect that promise rather than the first visible workaround.

## Design Honest Test Seams

Make dependencies explicit when they introduce IO, nondeterminism, external state, dangerous side effects, or a meaningful ownership boundary. Dependency injection is a means of controlling those dependencies, not a requirement to abstract every collaborator.

Honest does not mean pure. A function may take a `mut` argument or call a supplied gateway and still be honest because its authority is visible in the signature and repeated at the call (`retry_payment(mut gateway, payment)`). Tests must control or observe those declared effects. On 1.17 a method called through a read-only `interface` parameter can still mutate the implementation, so observe required non-effects rather than trusting the marker.

Prefer the smallest honest seam:

1. Pass a value when the decision only needs data such as the current time or a generated identifier.
2. Pass a function argument, such as `() → string`, for one narrow operation.
3. Use a small, consumer-owned `interface` for a coherent capability that genuinely varies.
4. Keep a concrete `class` when substitution would add no useful control or clarity.

Faber has no module-level mutable state (`SEM062`), so hidden dependencies arrive through process state: the filesystem, environment, wall clock, and `random.seed`, which reseeds the shared generator. Wire real implementations at a visible composition boundary, usually `main`. Do not hide dependencies in a context class or service bag, create one `interface` per `class`, expose private choreography, or add factories solely so a test can count calls.

Passing a dependency makes it explicit; it does not make it deterministic or faithful. Do not move reads or writes outside the transaction, stream, or workflow boundary that gives them meaning.

Then prefer the highest-fidelity implementation of the seam that remains safe and deterministic:

1. Real local implementation.
2. Hermetic local service or emulator.
3. Maintained fake.
4. Recorded fixture.
5. Stub that returns values without asserting interactions.
6. Recording double, only when the interaction itself is the stable contract.

Faber has no mocking framework; every double is an ordinary `class … implements` the interface. Dangerous side effects may justify replacing a real dependency, but not arbitrary interaction assertions. A double that restates the current call graph can pass while production is broken. If one is necessary, record why higher-fidelity options are unsuitable and why the asserted interaction is a durable contract. When a fake implements important semantics, run the same contract cases against the fake and the real implementation.

## Keep the Suite Honest

`AGENTS.md` requires every diagnostic and warning to be resolved, including ones that appear only under `faber test`. Test modifiers are not escape hatches. On 1.17:

- `skip "reason"` and `todo "reason"` report the case as skipped. Name the blocker, not "flaky".
- `only` silently drops every other case without reporting them as skipped. Never commit it.
- `expect_failure` expects a `⇥` escape that a test body cannot produce, so the case fails whether its assertion fails or passes. Do not use it.
- `flaky N` fails at run time; retries are not executed.
- `repeat N` reruns the case and fails when any run fails. It exposes nondeterminism; it does not retry it away.

Keep `assert` in tests. Production code uses `require` or `reject`, or a type that cannot hold the bad value. The docs call production `assert` `WARN006`, but 1.17 emits nothing, so review must catch it.

## Make Failures Self-Explanatory

Treat a test failure as a developer-facing error message. As directly as the assertion surface allows, it should reveal:

- what behavior or operation failed
- why the result was expected
- what was actually observed or which error occurred
- which input, fixture, identifier, or state matters for diagnosis

Keep the diagnostic audience distinct from the product audience. When testing a user-facing or machine-readable error contract, assert that boundary output separately; use the `panic` message to help a developer diagnose why the contract failed.

A failure prints only the case path and the `panic` text. So:

- Name each test for the behavior and condition it proves, with one coherent reason for failure.
- A bare `assert` reports `assertion failed`, with no expression, operands, or line. Give an assertion a `panic` message unless the test name settles its meaning, and interpolate what was observed: `assert got ≡ 4 panic "…; got §"(got)`.
- `!` on a missing value reports `unwrap of nihil`. Assert presence with a message first.
- A `⇥` call in a test needs `do` / `catch` (`SEM011`). In `catch`, name the precondition that should have held and include the error.
- For an expected rejection, put `assert false panic …` after the call inside `do`. `catch` handles only the `⇥` channel, so the assertion still fails the test.
- Do not use `⊥` in a test; the fallback hides the error that would explain the failure.
- Include inputs or observed output needed for diagnosis, and for stateful workflows whether the expected state was preserved.
- A `test` with no `assert` passes. Assert the contract the case names.
- Never expose secrets or dump large payloads merely to add context.

## Prove Red, Green, and Refactor

For new behavior or a bug fix:

1. Run the focused case before implementation, for example `faber test . --filter "suite/case"`, and confirm it fails for the intended missing or broken contract.
2. Make the smallest implementation change that satisfies the contract.
3. Run the focused case and confirm it passes.
4. Refactor without changing behavior, then rerun the focused and relevant broader tests.

`faber test` analyses the whole package before running any case. A real RED exits 1 with the case's own `panic` text. Exit 65 with `package analysis failed` is malformed setup, not RED, and it blocks every case in the package.

When a test fails, classify it before editing: the test is wrong, the implementation is wrong, the design blocks honest testing, or the environment is broken. Do not turn a wrong test green by changing production behavior, and do not hide an implementation bug by weakening the assertion.

## Keep Tests Reliable

- Use fixed inputs, times, identifiers, and ordering when the behavior permits it.
- Use a unique directory under `fs.temp_dir()` instead of real user data or machine state. There is no temporary-directory fixture, and `setup` / `teardown` cannot see a test's bindings, so clean up inside the test before asserting.
- Build fixtures in the test or in plain functions; hooks share no bindings with cases.
- Cases run serially (`--test-threads` is reserved). Do not rely on execution order or on files, environment, or seeds another case left behind.
- Prefer readable, behavior-sized setup over abstract helpers that hide the scenario. Repetition is acceptable when it keeps the contract visible.
- Make the correct fixture path the easiest path: encode a hidden setup rule in a field without a default, or fail at the earliest boundary with specific context.

## Verify

Run the narrowest relevant case while iterating. Before completion, run the workspace checks:

```bash
faber format .
faber check .
faber test .
```

`faber check` does not analyse `.proba` files; only `faber test` does. Read the actual failure output before reporting success. A passing run with warnings is not complete.
