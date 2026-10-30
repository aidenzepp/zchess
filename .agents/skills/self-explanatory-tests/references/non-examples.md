# Testing Non-Examples

These patterns create green tests without trustworthy evidence or failures without enough context to repair them. Each `faber` block compiles on 1.17.

## Opaque Failure Messages

```faber
let Account ∪ none selected ← select_first_enabled(accounts)
assert selected!.id ≡ "first"
assert outcome.status ≡ 0
assert limit ≡ 50 panic "failed"
```

The first failure ends the case, with no expression or line:

```text
FAIL src/main.fab::accounts/selects the first enabled account (0 ms)
  unwrap of nihil
```

Problems:

- `unwrap of nihil` does not say which value was missing or why it should exist.
- a bare `assert` reports `assertion failed`, and the status assertion discards the stderr that would explain it.
- `failed` does not name the assumption or the observed value.

## Repeating the Source Error

```faber
do {
    let Config parsed ← parse_config(fixture)
    assert parsed.name ≡ "zchess" panic "the fixture should name the package; got §"(parsed.name)
}
catch err {
    assert false panic err.message
}
```

The `catch` repeats the source error without explaining why parsing should have succeeded. Prefer:

```faber
catch err {
    assert false panic "the checked-in fixture should satisfy the current parser contract; got: §"(err.message)
}
```

## Testing the Implementation Path

```faber
test "calls the repository once" {
    mut CountingRepository repository ← CountingRepository {}
    list_tasks(mut repository)
    assert repository.list_calls ≡ 1
}
```

This freezes a call count, not the returned task contract. It can fail after harmless caching or pass while returned tasks are wrong.

## Tests as the Only Invariant

```faber
# Every caller must remember that file is 0 to 6.
fn step_east(int file) → int {
    assert file ≥ 0 and file ≺ 7 panic "file must be 0 to 6"
    return file + 1
}
```

If every caller and test must remember the range, the invariant has no owner, and 1.17 accepts this `assert` without the documented `WARN006`. Prefer a `trapping<u8, 0‥8>` file inside a `Square` built at the boundary, then test its construction and preservation.

## Service Bag

```faber
class Services {
    let Clock clock
    let TaskRepository tasks
    let HttpClient http
}

fn run(Services services) → void {
    print services.clock.now()
}
```

`run` needs only the time, but its signature grants every capability in the bag. Pass the narrow values or interfaces explicitly and wire them in `main`.

## Interface per Class

```faber
interface UserRepositoryLike {
    fn find(string id) → User ∪ none
}
```

An `interface` is not useful merely because a concrete `UserRepository` exists. Add the seam when the consumer needs a stable capability, legitimate substitution, or ownership boundary, not so a test double can replace every class.

## Convenient Fake, Different System

An in-memory store that silently allows duplicate keys while the real store rejects them is not faithful evidence. Run the same contract cases against both implementations or use the real store in an isolated directory.

## Broad Scenario Test

```faber
test "account workflow works" {
    # creates an account, changes permissions, imports data, retries a timeout,
    # exports a report, and checks six unrelated outcomes
}
```

One failure cannot identify which promise broke. Split independent behaviors while keeping each complete contract together. As written, it also passes: a `test` with no `assert` is green.

## Invented Behavior

```faber
test "retries exactly three times" {
    # No user requirement, public contract, or existing behavior establishes 3.
}
```

Do not turn an implementation guess into a permanent requirement. Find ground truth first.

## False Red

```text
$ faber test . --no-diagnostics
error[SEM002:unknown_type]: src/main.fab:12
  | let Account first ← Acount { id = "first", enabled = true }
error: package analysis failed
```

Exit 65 does not prove the behavior is missing or broken, and it hides every other case in the package. RED is valid only when the case fails on the intended contract: exit 1 with its own `panic` text.

## Hidden Machine State

```faber
do {
    let string config ← fs.read_file<string>("/Users/me/.config/app.toml")
    assert config.contains("name") panic "config should name the app"
}
catch err {
    assert false panic "config should exist"
}
```

This depends on one machine and may read real user data; `fs.home()` hides the same dependency. Use a unique directory under `fs.temp_dir()` with an explicit fixture.

## Modifier Bypass

```faber
test "parses the fixture" only {
    assert (parse_limit("50") ⊥ 0) ≡ 50
}

test "rejects a stale clock" skip "flaky" {
    assert false
}

test "rounds half to even" expect_failure {
    assert round_half_even(2.5) ≡ 3
}
```

With `only` present, this run reports `1 passed; 0 failed; 0 skipped`: the other cases vanish. Without it, `skip "flaky"` hides a symptom instead of naming a blocker, `expect_failure` fails either way on 1.17, and `⊥ 0` turns a parse failure into an ordinary wrong value.

Also reject `repeat N` or `flaky N` to absorb nondeterminism, ignoring warnings because the case passed, and skipping `faber test` because `faber check` passed. If a policy is genuinely wrong, change `AGENTS.md` with approval.

## Over-Abstracted Setup

```faber
let Context context ← standard_context()
```

If the tested outcome depends on identity, permissions, time, or stored rows hidden inside `standard_context`, the reader cannot see why the result should occur. Prefer DAMP setup that names contract-bearing facts directly.

## Common Rationalizations

| Rationalization | Reality |
| --- | --- |
| "The test passes." | A passing test can still protect the wrong behavior, only agree with its double, or assert nothing. |
| "A double is easier." | Easier setup is not better evidence; try a real local dependency, fake, fixture, or stub first. |
| "Dependency injection means interfaces everywhere." | Inject the smallest controlled value or capability; keep the concrete class when abstraction adds no honest seam. |
| "The test checks the invariant." | A test detects violations after the fact; a type or boundary should prevent or reject invalid states. |
| "It is just a private helper." | Private decomposition is replaceable unless that helper embodies the stable contract. |
| "The error is already printed." | Future readers still need the failed operation and assumption that made the error relevant. |
| "The test is obvious to me." | The failure must remain self-explanatory to an agent that did not author it. |
| "`repeat` will smooth out the flake." | `repeat` fails on any failing run; it exposes nondeterminism and never retries it away. |
| "I can rerun until it passes." | Intermittent success is evidence of nondeterminism, not correctness. |
