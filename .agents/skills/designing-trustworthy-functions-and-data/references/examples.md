# Trustworthy Function Examples

Use these as reasoning patterns, not APIs to copy mechanically. Preserve the project's actual domain, ownership, error, and performance contracts.

For paired before-and-after Faber refactors, read [design-transformations.md](design-transformations.md).

## Classify the Real Contract

```faber
import from "norma:time" time

# Pure and honest: all inputs and the returned output are visible.
fn total(list<int> values) → int {
    mut int sum ← 0
    for from values let value {
        sum ← sum + value
    }
    return sum
}

# Impure and honest: `mut` marks the mutation here and at every call site.
fn sort_by_priority(mut list<Task> tasks) → void {
    tasks ← sort via asc from tasks let task {
        return task.priority
    }
}

# Impure and honest: the output destination is an explicit capability.
fn write_receipt(mut Output output, Receipt receipt) → void ⇥ string {
    output.write_line(receipt.summary())
}

# Dishonest: the signature hides the clock as a real input.
fn session_status(Session session) → SessionStatus {
    return status_at(session, time.now())
}
```

"Honest" does not mean "pure." It means the signature tells the truth about the operation. The boundary adapter may read the clock, then pass that value to an honest domain decision.

## Pass the Value the Decision Needs

A domain rule that compares a deadline needs a timestamp, not permanent access to a clock:

```faber
fn status_at(Session session, instant<ns> now) → SessionStatus {
    if now ≥ session.expires_at then return SessionStatus.expired
    return SessionStatus.active
}

interface Clock {
    fn now() → instant<ns>
}

fn refresh(Clock clock, Session session) → SessionStatus {
    return status_at(session, clock.now())
}
```

The environmental adapter owns obtaining time. The rule receives an ordinary controlled value. `Clock` is earned only when callers genuinely choose the clock; otherwise read `time.now()` in `main` and pass the instant.

## Pass a Real Nondeterministic Capability

Norma's `random` module draws from one process-wide generator, and `random.seed` reseeds it for everyone. A generator that consumes randomness across many operations should take a caller-owned generator instead:

```faber
import from "norma:random" random

# A small seeded generator. The caller owns its state.
class Rng {
    mut int state

    fn below(int bound) → int {
        self.state ← (self.state * 1103515245 + 12345) % 2147483648
        return self.state % bound
    }
}

fn populate(mut ParticleWorld world, int count, mut Rng rng) → void {
    for range 0 ‥ count let i {
        world.particles.append(Particle { dx = rng.below(9) - 4, dy = rng.below(9) - 4 })
    }
}

main {
    mut Rng rng ← Rng { state = random.random_int(0, 2147483647) }
    mut ParticleWorld world ← ParticleWorld { particles = empty }
    populate(mut world, 100, mut rng)
}
```

Production draws an unpredictable seed once, at the boundary; tests construct `Rng { state = 42 }`. No `RandomService` interface is necessary. `faber check` warns `WARN008 unused_mut_parameter` on `rng` although `rng.below` mutates it; removing `mut` is `SEM020`, so keep the marker.

## Carry a Proven Invariant

```faber
import from "norma:math" math

class Vec3 {
    let float x
    let float y
    let float z
}

union UnitVec3Error {
    NonFinite,
    ZeroLength
}

# Unit length. Build only through `unit_vec3`.
class UnitVec3 {
    @ private
    let Vec3 value

    fn vector() → Vec3 {
        return self.value
    }
}

fn unit_vec3(Vec3 v) → UnitVec3 ⇥ UnitVec3Error {
    if not is_finite(v) {
        throw variant NonFinite
    }
    # Scale first so large or subnormal components cannot overflow or
    # underflow while the length is calculated.
    let float scale ← largest_magnitude(v)
    if scale ≡ 0.0 {
        throw variant ZeroLength
    }
    let Vec3 scaled ← divide(v, scale)
    let Vec3 unit ← divide(scaled, math.sqrt(dot(scaled, scaled)))
    if not is_finite(unit) {
        throw variant NonFinite
    }
    return UnitVec3 { value = unit }
}

fn reflect(Vec3 incident, UnitVec3 normal) → Vec3 {
    let Vec3 n ← normal.vector()
    return subtract(incident, multiply(n, 2.0 * dot(incident, n)))
}
```

The other helpers are ordinary `Vec3` functions. `UnitVec3` owns a stable mathematical requirement so no downstream caller must remember it. Faber does not seal construction literals, so `unit_vec3` is the only construction site by convention.

An operation must return the type it can prove:

```faber
fn cross(UnitVec3 left, UnitVec3 right) → Vec3 {
    return cross_vectors(left.vector(), right.vector())
}

fn normalized_cross(UnitVec3 left, UnitVec3 right) → UnitVec3 ⇥ UnitVec3Error {
    return unit_vec3(cross(left, right))
}
```

The ordinary cross product is not necessarily unit length. Parallel unit vectors produce zero.

## Parse a Primitive into a Domain Value

When a range expresses the invariant, let the type carry it. Every store into a ranged cell is checked, so no construction path can bypass it:

```faber
type ServicePort = trapping<u16, 1024…65535>

union PortError {
    Privileged {
        u16 port
    }
}

fn parse_port(u16 raw) → ServicePort ⇥ PortError {
    if raw ≺ 1024 {
        throw variant Privileged { port = raw }
    }
    return raw
}

class PortPool {
    @ private
    mut set<ServicePort> free

    fn release(ServicePort port) → bool {
        let bool fresh ← not self.free.has(port)
        self.free.add(port)
        return fresh
    }

    fn contains(ServicePort port) → bool {
        return self.free.has(port)
    }
}
```

The raw `u16` is parsed once at a configuration, CLI, file, or network boundary, with a typed error. Pool operations no longer repeat the check or carry `PortError`, and the guarantee travels with every port. An unchecked store into the ranged type traps instead of reporting, so untrusted input still belongs in the parser; the type makes a mistake elsewhere fail loudly.

If the rule were meaningful only inside `PortPool` and elements never crossed its boundary, a private aggregate that parses raw ports itself could own the invariant without a separate element type.

## Preserve the Transaction Boundary

```faber
fn transfer(mut Transaction transaction, Transfer command) → Receipt ⇥ TransferError {
    let LockedAccounts accounts ← transaction.lock_accounts(command.from, command.to)
    let TransferPlan plan ← plan_transfer(accounts, command)
    transaction.apply(plan)
    transaction.append_ledger_entry(command)
    return transaction.commit()
}
```

`plan_transfer` can be deterministic, but acquiring locks, deciding from current state, writing, and committing remain inside one consistency boundary; each bare call propagates on `⇥ TransferError`. Hoisting all effects to `main` would make the design less correct.

## Request the Power Actually Used

```faber
fn total(list<int> values) → int {
    mut int sum ← 0
    for from values let value {
        sum ← sum + value
    }
    return sum
}

fn block_parity(list<b8, size 64…64> block) → b8 {
    mut b8 parity ← 0
    for from block let byte {
        parity ← parity ⊻ byte
    }
    return parity
}
```

The first operation only reads, so it asks for no marker. The second honestly asks for exactly one 64-byte block; the sized list is part of the algorithm's contract.

## Use a Cohesive Parameter Object

```faber
enum FirstTick {
    immediately,
    after_interval
}

enum Repetition {
    once,
    forever
}

class TimerOptions {
    let TimerId id
    let trapping<u32, 1…86400> interval_seconds
    let FirstTick first_tick
    let Repetition repetition
}

fn schedule_refresh(mut Scheduler scheduler) → void ⇥ string {
    scheduler.set_timer(TimerOptions {
        id = parse_timer_id("refresh"),
        interval_seconds = 5,
        first_tick = FirstTick.after_interval,
        repetition = Repetition.forever
    })
}
```

The fields describe one timer policy. The field name carries the unit, the ranged type carries the bounds, and enums replace positional booleans.

## Keep Domain Intent at One Level

```faber
class AssetCatalog {
    @ private
    mut map<string, AssetType> types_by_name

    fn is_asset_of_type(AssetName name, AssetType expected) → bool {
        let AssetType ∪ none found ← self.types_by_name.get(name.key())
        if found ≡ null then return false
        return found ≡ expected
    }
}
```

`AssetName` owns validation and canonical casing; `key()` returns the canonical text. The private `map` owns association and lookup, and only methods taking `AssetName` reach it, so no raw string becomes a key. The method does not lowercase text, keep a sorted list, or implement binary search. The key is `string` because class keys fail at run time in 1.17.

## Stream Results That Should Not Be Materialized

```faber
main {
    for from permutations("abcd") let permutation {
        print permutation
    }
}
```

`permutations` is declared `fn permutations(string input) generator → string`. A `generator` is consumed lazily, so the caller controls output and can stop early without allocating every factorial-sized result at once.
