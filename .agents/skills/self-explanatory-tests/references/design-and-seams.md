# Design Pressure and Test Seams

Use this reference when a test is difficult to write, an invariant is repeatedly checked, or a dependency seems to require a large abstraction.

This reference translates design pressure into testing consequences. Use `designing-trustworthy-functions-and-data` to decide whether and how production types, dependencies, effects, or function boundaries should change. Test difficulty is evidence, not automatic authorization for a refactor.

## Build the Obligation Map

Start from promises, not files. One component may own several obligations, and one obligation may cross several components.

| Obligation | Owner | Enforcement | Test evidence | Remaining gap or status |
| --- | --- | --- | --- | --- |
| A square is on the board | `Square` | `trapping<u8, 0‥8>` fields and a fallible `square` constructor | Constructor acceptance and rejection cases, plus every path that builds a `Square` | Covered |
| Sessions expire at the deadline | Session policy | Pure comparison using an explicit current time | Boundary cases immediately before, at, and after the deadline | Covered |
| A notification is not duplicated after retry | Delivery workflow | Idempotency key and durable result | Retry test against an isolated store and recording sink | Covered |
| Concurrent writers preserve ordering | Store | Not yet established | None | Deferred only with named risk and approval |

The map is not paperwork for every helper. Use it when the work has multiple promises, state, external effects, safety implications, or meaningful failure modes. Its purpose is to reveal obligations that otherwise disappear between unit tests.

## Move Invariants into the Design

Suppose several functions accept an `int` file but require it to be on the board. Repeating `require file ≥ 0 and file ≺ 8` in callers, or `assert` in tests, leaves the invariant distributed and easy to forget.

Prefer a `Square` whose fields are `trapping<u8, 0‥8>`, built by a fallible `square(int file, int rank) → Square ⇥ string`. Downstream functions accept `Square`, making the requirement visible. A store outside the range traps, so no `Square` holds an off-board coordinate, even from a literal that skips the constructor.

The type does not eliminate testing. It changes the test obligations:

- on-board coordinates are accepted and kept
- each off-board axis is rejected through `⇥`, not by a trap
- operations and restoration paths that produce a `Square` report bad input instead of storing it
- downstream functions need no off-board cases

The type enforces the invariant. The tests prove that the boundary turns bad input into reportable errors.

When an invariant relates two fields, such as an end that must not precede its start, a constructor cannot seal it against literals. Prefer a layout that cannot express the bad combination, such as a start and a `u32` length.

Do not assume a stronger type is automatically safer. Confirm that every operation can preserve the claim. A knight's move from a `Square` can leave the board, so its honest return type is `Square ∪ none` or `Square ⇥ E`. Ranged arithmetic is exact and checked only at a store, so code that builds an off-board `Square` traps instead of reporting.

## Inject the Value Before the Service

If a decision only needs the current time, pass the current time:

```faber
let SessionStatus status ← status_at(session, 1000)
assert status ≡ SessionStatus.Expired panic "a session should be expired exactly at its deadline; got §"(status)
```

This is dependency injection without an interface. It keeps the domain decision pure and gives the test exact control.

Introduce a clock capability only when the component itself legitimately owns the act of obtaining time across multiple operations. Do not add a `Clock` interface merely because one function reads one timestamp.

Use the same progression for randomness and identifiers: inject the resulting value first, a narrow generator function second, and an `interface` only when a coherent capability needs an owner. Do not reach for `random.seed`; it reseeds the shared generator for the whole run.

## Make Capability Seams Consumer-Owned

When a workflow must perform an external effect, define the smallest `interface` the workflow needs in domain terms. For example, a reminder workflow may need `fn deliver(Reminder reminder) → void ⇥ DeliveryError`, not an HTTP client, mail client, provider package, or application context.

A good seam:

- names a stable domain capability
- makes failure explicit on the `⇥` channel
- has no methods used only by tests
- can be implemented by the real adapter and a faithful fake
- does not expose transport choreography to the core
- is wired to the real implementation at a visible composition boundary, usually `main`

Do not invent a broad repository, manager, provider, or service abstraction before the consumer contract is known.

An explicit capability may still be nondeterministic, stateful, or unfaithful to production. Keep transaction, ordering, cancellation, and backpressure behavior at the boundary that owns it. A fake store does not prove the real store's uniqueness, isolation, rollback, or durability.

## Choose the Double by Evidence Quality

- Use the real local implementation when it is safe and deterministic, such as real files under `fs.temp_dir()`.
- Use a hermetic service or maintained fake when real wiring matters but remote effects do not.
- Use a fixture or stub when only a returned result matters.
- Use a recording double when the external interaction is itself the promise, for example, a dangerous charge must not be issued twice.

Every double is a hand-written `class … implements` the interface. If a fake owns behavior such as uniqueness, ordering, or retry semantics, run the same contract cases against the fake and the real implementation, for example a function that takes the interface and returns a failure description each test asserts is `null`. Otherwise the fake may make tests green by behaving more conveniently than production.

## Diagnose Test Pain

When testing is difficult, classify the pressure before adding an abstraction:

| Pressure | Likely response |
| --- | --- |
| Invalid values flow through many functions | Introduce a ranged field, trusted domain type, or boundary parser. |
| Tests construct states that should be impossible | Model the states as a `union` and let exhaustive `match` own them. |
| Time, randomness, or IDs make results unstable | Inject the value or a narrow generator function. |
| External effects are inseparable from decisions | Separate the pure decision from the effectful adapter. |
| A double needs many ordered expectations | Test a higher-level outcome or reduce choreography exposed by the seam. |
| The fixture requires unrelated system state | Reconsider component ownership or split the boundary by responsibility. |
| A fake repeatedly disagrees with production | Add shared contract cases or replace it with a higher-fidelity dependency. |

Test pain is evidence, not automatic authorization for a refactor. Keep redesign within the requested scope; otherwise preserve the behavior with a test and report the design debt.

Before separating a decision from an effect, verify that doing so does not create a time-of-check/time-of-use gap or break atomicity. The correct response may be a deterministic inner calculation called from inside the effectful operation rather than moving the entire decision outside it.
