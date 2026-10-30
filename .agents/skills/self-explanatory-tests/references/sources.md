# Testing Sources

These sources explain the reasoning behind the workspace policy. Use them when a test boundary, failure-message rule, dependency choice, or test modifier is disputed.

## Source Skills Adapted

- `contract-tests`: name the contract before RED, ground expected behavior, choose the smallest stable boundary, prefer observable outcomes, resist mocks, and run a refactor-survival check.
- `error-messages`: treat failures as developer-facing errors that identify what happened, why it matters, relevant context, and useful next evidence.
- `root-cause`: reproduce the symptom and protect the deepest violated promise rather than the first visible symptom.
- `red-green`: prove RED fails for the intended reason, classify failures honestly, and never report unexecuted tests as successful.
- `fool-proof-software`: remove hidden memory requirements from fixtures and make invalid setup fail at the earliest useful boundary.
- `software-taste`: make invalid states difficult to represent, parse once at honest boundaries, inject dependencies without ceremony, and test observable contracts.
- `designing-trustworthy-functions-and-data`: keep production contracts, dependencies, effects, invariants, and abstraction boundaries truthful while resisting mock-driven or type-driven over-design.

The repo-owned skill adapts these principles to Faber and this repository's warning policy; it does not import the source skills' broader workflows.

## Faber Mechanics

The installed `faber` CLI is the authority. Where these pages and `faber test` disagree, trust the CLI and record the disagreement; the modifier notes in `SKILL.md` were checked against 1.17.0.

- [Errors and testing](https://faberlang.dev/en-US/language/errors.html) — the `→` / `⇥` channels, `do` / `catch` recovery, inline `describe` / `test` / `assert`, colocated `*.proba` files, and the `faber test` selectors.
- [Errors as values, tests as declarations](https://faberlang.dev/en-US/language/behavior/errors-and-tests.html) — tests are type-checked declarations filtered out of production builds and run on the MIR runner.
- [Corpus: Testing](https://faberlang.dev/en-US/corpus/bucket/testing.html) — one page per test keyword and modifier, each with a checked example.
- [`assert`](https://faberlang.dev/en-US/corpus/assert.html) — the `panic` particle that carries an assertion's failure message.
- `faber test --help` — target selection by `--filter`, `--suite`, `--name`, `--tag`, and `--only-in`, plus `--list`, `--format json`, `--fail-fast`, and `--deny-warnings`.
- [Release notes](https://faberlang.dev/en-US/releases/) — breaking changes ship in minor releases; read the notes for the installed version.

## Contract Quality

- [Google Testing Blog: Test Behavior, Not Implementation](https://testing.googleblog.com/2013/08/testing-on-toilet-test-behavior-not.html) — tests should normally survive behavior-preserving implementation changes.
- [Google Testing Blog: Test Behaviors, Not Methods](https://testing.googleblog.com/2014/04/testing-on-toilet-test-behaviors-not.html) — behavior concepts, rather than one test per method, determine useful test boundaries.
- [Google Testing Blog: Don't Overuse Mocks](https://testing.googleblog.com/2013/05/testing-on-toilet-dont-overuse-mocks.html) — interaction-heavy mocks leak implementation details and provide weaker evidence than real or hermetic alternatives.
- [Google Testing Blog: Increase Test Fidelity by Avoiding Mocks](https://testing.googleblog.com/2024/02/increase-test-fidelity-by-avoiding-mocks.html) — choose the highest-fidelity dependency that remains appropriate for the test's size and determinism constraints.
- [Addy Osmani: Test-Driven Development Skill](https://github.com/addyosmani/agent-skills/blob/main/skills/test-driven-development/SKILL.md) — source for the prove-it regression pattern, DAMP tests, behavior-sized cases, and common testing rationalizations.

## Design and Dependency Boundaries

- [`trapping<W, lo‥hi>`](https://faberlang.dev/en-US/corpus/trapping%3CW%2C%20lo%E2%80%A5hi%3E.html) — a ranged integer cell: a store outside the range traps, so the type rules out the value instead of every caller checking it.
- [`match`](https://faberlang.dev/en-US/corpus/match.html) — exhaustive matching over a `union`, which lets the compiler own "every case is handled".
- [`interface`](https://faberlang.dev/en-US/corpus/interface.html) — the behavioral contract a consumer-owned seam and its test doubles implement.
- [Martin Fowler: Inversion of Control Containers and the Dependency Injection Pattern](https://martinfowler.com/articles/injection.html) — distinguishes dependency injection from service location and emphasizes separating configuration from use.
