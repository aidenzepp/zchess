# Design Transformations

These paired Faber examples show how to turn misleading, overpowered, or mixed-level function designs into truthful contracts.

The snippets are independent shapes, not one package. Each one passes `faber check` 1.17 once its neighboring types exist; adapt domain names and error payloads, but preserve the contract distinction each pair demonstrates.

## 1. Honest Does Not Mean Pure

A function can mutate and still be honest when the mutation is declared:

```faber
fn clear_all(mut list<int> values) → void {
    values ← empty
}

fn remove_disabled(mut list<Account> accounts) → int {
    let int previous ← accounts.length()
    accounts ← accounts.filter(Account account ∴ account.enabled)
    return previous - accounts.length()
}
```

Both functions are impure because they mutate. Both are honest because `mut` exposes exactly what they may change, and the caller must write `remove_disabled(mut accounts)`. `remove_disabled` also returns the number removed, demonstrating that mutation and a return value can be separate, visible output channels.

By contrast, this function's purpose is to read a hidden environmental input:

```faber
import from "norma:time" time

fn current_time() → instant<ns> {
    return time.now()
}
```

Under this skill's vocabulary, `current_time` is necessarily dishonest: the clock that determines its result is absent from the signature. Keep this operation at a boundary and pass its result inward:

```faber
import from "norma:time" time

fn session_status(Session session, instant<ns> now) → SessionStatus {
    if now ≥ session.expires_at then return SessionStatus.expired
    return SessionStatus.active
}

fn handle_request(Session session) → Response {
    let instant<ns> now ← time.now()
    return response_for(session_status(session, now))
}
```

The request boundary remains deliberately dishonest and effectful. The domain decision becomes honest and deterministic. If obtaining time is itself a caller-owned policy, pass a narrow `Clock` interface to this adapter; do not pass an application-wide context merely to reach a clock.

## 2. An Accessor with a Hidden Dependency

### Non-example

The name promises required assets, but the implementation silently drops the ones already on disk:

```faber
import from "norma:fs" fs

fn required_assets(Manifest manifest) → list<string> {
    return manifest.required.filter(string path ∴ not fs.exists(path))
}
```

A caller cannot tell that filesystem state changes the result.

### Better

Make the distinction and dependency explicit:

```faber
fn required_assets(Manifest manifest) → list<string> {
    return manifest.required
}

fn missing_required_assets(Manifest manifest, set<string> loaded) → list<string> {
    return manifest.required.filter(string path ∴ not loaded.has(path))
}
```

The caller chooses whether it needs the complete requirement or the currently missing subset, and the boundary decides how `loaded` is observed.

## 3. Permutations Mixed with Output

### Non-example

The algorithm chooses where results go:

```faber
fn print_permutations(string prefix, list<string> rest) → void {
    if rest.is_empty() {
        print prefix
    }
    for range 0 ‥ rest.length() let i {
        print_permutations(prefix + rest[i], without(rest, i))
    }
}
```

The permutation logic cannot be reused for a file, a search, or a test without capturing process output.

### Bounded materialization when the complete result is the contract

```faber
union PermutationError {
    TooMany {
        int limit
    }
}

fn all_permutations(list<string> items, int limit) → list<string> ⇥ PermutationError {
    mut list<string> results ← empty
    for from arrangements("", items) let arrangement {
        if results.length() ≡ limit {
            throw variant TooMany { limit = limit }
        }
        results.append(arrangement)
    }
    return results
}
```

The algorithm is honest and the materialization limit is part of the contract. An unbounded list may separate calculation from output, but storing every result usually requires `Theta(n * n!)` payload space and should not be presented as generally safe.

### Better when materialization is too expensive

```faber
fn arrangements(string prefix, list<string> rest) generator → string {
    if rest.is_empty() {
        yield prefix
    }
    else {
        for range 0 ‥ rest.length() let i {
            for from arrangements(prefix + rest[i], without(rest, i)) let arrangement {
                yield arrangement
            }
        }
    }
}

main {
    for from arrangements("", ["a", "b", "c"]) let arrangement {
        print arrangement
    }
}
```

A `generator` is consumed lazily: the caller can compose, `break` early, and choose the destination without allocating all results. A closure parameter is the other consumer-injection shape, but it cannot report on the caller's `⇥` channel.

## 4. `populate` and the Process-Wide Generator

### Non-example

```faber
import from "norma:random" random

class ParticleWorld {
    mut list<Particle> particles

    fn populate(int count) → void {
        for range 0 ‥ count let i {
            self.particles.append(Particle { dx = random.random_int(-4, 4), dy = random.random_int(-4, 4) })
        }
    }
}
```

`populate` appears to depend only on the world and count, but it also consumes the process-wide generator behind `norma:random`. Other random calls change its result at a distance, and callers cannot choose reproducibility except by reseeding everyone with `random.seed`.

### Honest Faber version

```faber
import from "norma:random" random

fn populate(mut ParticleWorld world, int count, mut Rng rng) → void {
    for range 0 ‥ count let i {
        world.particles.append(Particle { dx = rng.below(9) - 4, dy = rng.below(9) - 4 })
    }
}

main {
    mut Rng rng ← Rng { state = random.random_int(0, 2147483647) }
    mut ParticleWorld world ← ParticleWorld { particles = empty }
    populate(mut world, 42, mut rng)
}

describe "populate" {
    test "a fixed seed reproduces the world" {
        mut Rng first ← Rng { state = 42 }
        mut Rng second ← Rng { state = 42 }
        mut ParticleWorld a ← ParticleWorld { particles = empty }
        mut ParticleWorld b ← ParticleWorld { particles = empty }
        populate(mut a, 5, mut first)
        populate(mut b, 5, mut second)
        assert a.particles ≡ b.particles
    }
}
```

`Rng` is the small seeded class from [examples.md](examples.md#pass-a-real-nondeterministic-capability). Production draws an unpredictable seed once at the boundary; tests, simulations, and bug reproductions choose a known seed. This is dependency injection without a framework or an `interface`. `populate` is still impure—it mutates the world and the generator—but both mutations are honest. Durable replay data must also record the generator algorithm and relevant code version; a seed alone is not a permanent replay format.

## 5. Primitive and Boolean Timer Arguments

### Non-example

```faber
fn schedule() → void {
    set_timer("refresh", 5, true, false, true)
}
```

The call site does not identify the time unit or what any boolean means.

### Better

```faber
fn schedule(mut Scheduler scheduler) → void ⇥ string {
    scheduler.set_timer(TimerOptions {
        id = parse_timer_id("refresh"),
        interval_seconds = 5,
        first_tick = FirstTick.after_interval,
        repetition = Repetition.once,
        retrigger = RetriggerPolicy.restart
    })
}
```

The options class describes one timer configuration. Enums, a unit-bearing field name, and a ranged interval type replace positional primitives and boolean blindness. [examples.md](examples.md#use-a-cohesive-parameter-object) declares the class; this version adds a `RetriggerPolicy` enum field.

### Overcorrection

```faber
fn set_timer(Application application) → TimerHandle ⇥ TimerError {
    # Reads six fields from a much larger object.
    return TimerHandle { id = application.timer_count }
}
```

Passing the entire application grants unrelated knowledge and authority. Use a cohesive options class, not a renamed context bag.

## 6. Asking for More Power Than the Body Uses

### Non-example

```faber
fn total(mut list<int> values) → int {
    mut int sum ← 0
    for from values let value {
        sum ← sum + value
    }
    return sum
}
```

The body only reads, but the signature claims write-back to the caller's list, and every call must say `total(mut xs)`. `faber check` warns `WARN008 unused_mut_parameter`.

### Better

Drop the marker: `fn total(list<int> values) → int`. Ask for `mut`, `own`, or a sized type such as `list<b8, size 64…64>` only when the operation genuinely changes, consumes, or depends on the length of its input.

## 7. "Must Call A Before B"

### Non-example

```faber
fn start(mut System system) → void ⇥ string {
    initialize(mut system)
    # Must call initialize first.
    run(mut system)
}
```

The comment carries a lifecycle invariant the API permits callers to violate.

### Better: one class per state

```faber
class UninitializedSystem {
    let Configuration configuration
}

class InitializedSystem {
    mut Resources resources
}

fn initialize(UninitializedSystem system) → InitializedSystem ⇥ InitializeError {
    let Resources resources ← load_resources(system.configuration)
    return InitializedSystem { resources = resources }
}

fn run(mut InitializedSystem system) → void ⇥ RunError {
    system.resources.run()
}
```

`run` accepts only the state that `initialize` produces. As with any class, another module could still write `InitializedSystem { … }`, so keep `initialize` the only construction site.

When the earlier value must not be used again, take it with `own` and pass it on with `own`. Faber then refuses reuse at compile time. `faber check` rejects this: `SEM050 use_after_own`.

```faber
class Letter {
    let string body
}

class Outbox {
    mut list<Letter> queued
}

fn send(mut Outbox outbox, own Letter letter) → void {
    outbox.queued.append(own letter)
}

main {
    mut Outbox outbox ← Outbox { queued = empty }
    let Letter letter ← Letter { body = "hello" }
    send(mut outbox, own letter)
    send(mut outbox, own letter)
}
```

This is stronger than an empty receipt token, which can be constructed anywhere and outlive the condition it claims.

## 8. "The Lock Must Be Held"

The source pairs this with a lock guard. Faber 1.17 has no user-level lock type and no module-level mutable state (`SEM062 top_level_binding`), so "ambient shared state behind a lock" cannot be written. The lesson carries over unchanged: a proof must be tied to the live state it certifies, as the state classes and `own` hand-off in section 7 are.

## 9. Normalized Surface Normals

### Non-example

```faber
fn reflect(Vec3 incident, Vec3 normal) → Vec3 ⇥ ReflectError {
    if not is_unit_length(normal) {
        throw variant NormalNotNormalized
    }
    return subtract(incident, multiply(normal, 2.0 * dot(incident, normal)))
}
```

Every reflection call accepts invalid normals and must remember the same validation and failure policy, and every caller must handle `⇥ ReflectError`.

### Better when unit length is a stable domain invariant

```faber
fn reflect(Vec3 incident, UnitVec3 normal) → Vec3 {
    let Vec3 n ← normal.vector()
    return subtract(incident, multiply(n, 2.0 * dot(incident, n)))
}
```

`UnitVec3` uses a `@ private` field and fallible, overflow-safe construction. See [examples.md](examples.md#carry-a-proven-invariant) for `unit_vec3`.

Do not overstate what the type proves: `fn cross(UnitVec3 left, UnitVec3 right) → Vec3` must not return `UnitVec3`. The magnitude is `sin(theta)`, and parallel unit vectors produce zero. A separately named normalized cross product must be fallible.

## 10. One Level of Abstraction in Asset Lookup

### Non-example

```faber
fn is_asset_of_type(list<Asset> sorted_assets, string asset_name, AssetType expected) → bool {
    # Phase 1: representation policy.
    let string normalized ← asset_name.lowercase()

    # Phase 2: search mechanics.
    mut int low ← 0
    mut int high ← sorted_assets.length()
    while low ≺ high {
        let int middle ← low + (high - low) / 2
        let Asset asset ← sorted_assets[middle]
        if asset.name ≡ normalized then return asset.kind ≡ expected
        if asset.name ≺ normalized {
            low ← middle + 1
        }
        else {
            high ← middle
        }
    }
    return false
}
```

The function alternates between domain intent, name-normalization policy, index arithmetic, and search implementation. Handwritten binary search also concentrates subtle correctness risk inside a domain operation, and every insertion must remember to keep the list sorted and lowercased.

### Better: let the representation own both invariants

```faber
union AssetNameError {
    Empty
}

# Lowercase, non-empty asset name. Build only through `parse_asset_name`.
class AssetName {
    @ private
    let string text

    fn key() → string {
        return self.text
    }
}

fn parse_asset_name(string raw) → AssetName ⇥ AssetNameError {
    reject raw.length() ≡ 0 throw variant Empty
    return AssetName { text = raw.lowercase() }
}

class AssetCatalog {
    @ private
    mut map<string, AssetType> types_by_name

    fn insert(AssetName name, AssetType asset_type) → void {
        self.types_by_name.put(name.key(), asset_type)
    }

    fn asset_type(AssetName name) → AssetType ∪ none {
        return self.types_by_name.get(name.key())
    }

    fn is_asset_of_type(AssetName name, AssetType expected) → bool {
        let AssetType ∪ none found ← self.asset_type(name)
        if found ≡ null then return false
        return found ≡ expected
    }
}
```

`AssetName` owns validation and lowercase canonicalization. `map` owns unique-key association, replacement, and lookup. `AssetCatalog` earns its place by sealing the keys: its private map is keyed by canonical text, and every method accepts only `AssetName`, so raw strings from files, JSON, or user input must pass through `parse_asset_name` before entering it. (A bare `map<AssetName, AssetType>` would be the smaller design, but class keys fail at run time in Faber 1.17.)

Ordering disappears because it was required by the old binary-search representation, not by the domain. The domain operation remains at one conceptual level, and neither callers nor helpers must remember normalization, sorting, or search policy.

`string.lowercase()` is the case policy here. Unicode case folding and normalization require a separately specified policy.

## 11. Thin Entry Points

`main args` is the framework boundary Faber gives you: the runtime owns argument parsing and when the body runs. Keep it a small adapter into application-owned functions:

```faber
import from "norma:time" time

@ cli { name = "sessions" }
@ operand { rest = true, type = string, binding = ids }
main args argv {
    let instant<ns> now ← time.now()
    for from argv.ids let raw {
        do {
            let SessionId id ← parse_session_id(raw)
            print render_status(status_for(id, now))
        }
        catch err {
            print "skipped §: §"(raw, err)
        }
    }
}
```

The entry is not pure, and the runtime still controls when it runs. Its job is boundary translation—operands to parsed ids, the clock to an `instant`—not domain policy.
