# Testing Examples

Use these examples as shapes, not as domain requirements. Preserve the repository's real types and contracts. Fragments omit the enclosing `describe` / `test` and the declarations they exercise.

## Inline Unit Contract

Production behavior and its unit contract live in the same `.fab` module:

```faber
class Account {
    let string id
    let bool enabled
}

# Return the first enabled account, or null when none is enabled.
fn select_first_enabled(list<Account> accounts) → Account ∪ none {
    for from accounts let account {
        if account.enabled then return account
    }
    return null
}

describe "select_first_enabled" {
    test "selects the first enabled account" {
        let Account disabled ← Account { id = "disabled", enabled = false }
        let Account first ← Account { id = "first", enabled = true }
        let Account second ← Account { id = "second", enabled = true }
        let Account ∪ none selected ← select_first_enabled([disabled, first, second])
        assert selected ≢ null panic "the fixture contains an enabled account, but none was selected"
        assert selected!.id ≡ "first" panic "expected the first enabled account; got §"(selected!.id)
    }
}
```

The test names the behavior, uses visible setup, asserts the observable result, and checks presence before `!`, which would otherwise fail as `unwrap of nihil`.

## Diagnostic Assertion

When the default assertion would hide the useful output, include it. Here `main` passes its arguments to `run_cli` and exits with the returned status:

```faber
let CliOutcome outcome ← run_cli(["--limit", "500"])
assert outcome.status ≢ 0 panic "an invalid limit should fail, but the command exited 0; stderr: §"(outcome.stderr)
assert outcome.stderr.contains("limit must be between 1 and 100") panic "stderr should explain the accepted range; stderr: §"(outcome.stderr)
```

Without `panic`, either line would report only `assertion failed`. When the process boundary itself is the contract, `process.capture` from `norma:process` runs a built executable and returns its status and output.

## Error Contract and Test Diagnostic

Keep the product error contract separate from the message that helps a developer repair a failed test:

```faber
do {
    let u8 accepted ← parse_limit("500")
    assert false panic "a limit above 100 should be rejected; accepted §"(accepted)
}
catch err {
    assert err.code ≡ "limit_out_of_range" panic "the stable error code changed; got §"(err.code)
    assert err.message ≡ "limit must be between 1 and 100; received 500" panic "the error should name the range and the offending value; got §"(err.message)
}
```

The compared strings belong to the product contract; the `panic` messages say which part changed. `catch` handles only the `⇥` channel, so the `assert false` still fails the test if the value is accepted.

## Bug Regression

Protect the violated behavior rather than the internal helper that caused it:

```faber
test "rejects an end before the start" {
    do {
        let TimeWindow accepted ← time_window(20, 10)
        assert false panic "time_window(20, 10) should be rejected; got length §"(accepted.length())
    }
    catch err {
        match err {
            case EndBeforeStart let start, end {
                assert start ≡ 20 and end ≡ 10 panic "the error should carry the rejected bounds; got start §, end §"(start, end)
            }
        }
    }
}
```

Before fixing the bug, confirm this case fails with its own `panic` text, not with `package analysis failed`. The exhaustive `match` makes a new `TimeWindowError` variant a compile error here until the test decides whether it is acceptable.

## Invariant-Carrying Type

When many callers require the same precondition, put it in the field types and test the boundary that builds them:

```faber
class Square {
    let trapping<u8, 0‥8> file
    let trapping<u8, 0‥8> rank
}

# Build a square from untrusted board coordinates.
fn square(int file, int rank) → Square ⇥ string {
    require file ≥ 0 and file ≺ 8 throw "file § is off the board"(file)
    require rank ≥ 0 and rank ≺ 8 throw "rank § is off the board"(rank)
    return Square { file = file, rank = rank }
}

describe "square" {
    test "reports an off-board file" {
        do {
            let Square accepted ← square(8, 0)
            assert false panic "square(8, 0) should be rejected; got file §"(accepted.file)
        }
        catch err {
            assert err ≡ "file 8 is off the board" panic "got §"(err)
        }
    }
}
```

Functions that take a `Square` need no off-board cases: a store outside the range traps (``9 does not fit in `trapping<u8, 0‥8>` (field `file`)``), even from a literal that skips `square`. Test that the constructor keeps on-board coordinates and rejects each off-board axis, and that every other path to a `Square` (parsing, move arithmetic, restoring a saved game) checks before it stores.

## Injected Value

Prefer passing the needed value over inventing an interface:

```faber
let Session session ← Session { deadline = 1000 }
let SessionStatus status ← status_at(session, 1000)
assert status ≡ SessionStatus.Expired panic "a session should be expired exactly at its deadline; got §"(status)
```

The test controls time directly. A clock `interface` is unnecessary unless obtaining time is itself a coherent dependency owned by the component.

## Narrow Capability Seam

A workflow that sends reminders may depend on a small `ReminderSink` `interface` with one `deliver` method. `main` wires it to the real adapter; tests use a maintained in-memory `class … implements ReminderSink` and assert the workflow result and delivered reminder. The seam describes what the workflow needs, not how one provider currently performs it.

## Justified Interaction Test

An interaction assertion is appropriate when the interaction is the promise. The recording double is an ordinary class:

```faber
class RecordingGateway implements Gateway {
    mut list<string> charged = empty

    fn charge(Payment payment) → void ⇥ string {
        self.charged.append(payment.id)
    }
}

describe "retry_payment" {
    test "does not charge a payment that already completed" {
        mut RecordingGateway gateway ← RecordingGateway {}
        let Payment payment ← Payment { id = "pay-1", status = Completed }
        do {
            let PaymentStatus status ← retry_payment(mut gateway, payment)
            assert status ≡ Completed panic "got §"(status)
        }
        catch err {
            assert false panic "retrying a completed payment failed: §"(err)
        }
        assert gateway.charged.is_empty() panic "a completed payment must not be charged again; charged §"(gateway.charged)
    }
}
```

The non-interaction protects a dangerous external effect, not incidental call order.

## Public-Contract Suite

A colocated `.proba` file sees only the `@ public` surface, as an outside caller would:

```faber
# Verify the public configuration-loading contract.
import from "./config" config

describe "config.parse" {
    test "reports the missing required field" {
        do {
            let config.Config parsed ← config.parse("port = 8080")
            assert false panic "a config without a name should be rejected; got port §"(parsed.port)
        }
        catch err {
            assert err.field ≡ "name" and err.reason ≡ "required" panic "expected name/required; got §/§"(err.field, err.reason)
        }
    }
}
```

Use a `.proba` file such as `src/config.proba` for public contracts, not merely to place tests in another file. Only `faber test` analyses it.

## Explicit Fixture

The construction literal is the fixture builder. A field without a default is required, so each fixture names its contract-bearing state:

```faber
let Account fixture ← Account { id = "account-1", enabled = true }
```

Omitting one does not compile:

```faber
let Account fixture ← Account { id = "account-1" }
```

Expected: `SEM004 missing_required_struct_field`. A default such as `mut bool enabled = true` is hidden fixture state; add one only when no test depends on its value. A fixture function in a `.proba` file stays out of product code.

## Dependency Fidelity

For a storage contract, prefer real files in an isolated directory over a call-count double:

```faber
let string root ← fs.join([fs.temp_dir(), "task-store-§"(random.uuid())])
let TaskStore store ← TaskStore { root = root }
mut list<string> ids ← empty

do {
    fs.create_dir(root)
    store.save("task-1", "Ship")
    ids ← store.list_ids()
    fs.remove_tree(root)
}
catch err {
    assert false panic "the store under § should accept a save and a listing"(root)
}

assert ids ≡ ["task-1"] ∷ list<string> panic "the store should list exactly the saved task; got §"(ids)
```

This proves the stored and returned state, not which internal call produced it. Cleanup precedes the assertion because `teardown` cannot see `root` and a failed assertion ends the case.
