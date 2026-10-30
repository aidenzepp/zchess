# Trustworthy Function Non-Examples

These patterns often look modular or strongly typed while making behavior less truthful, less testable, or harder to understand. Each block compiles; the problem is the design.

For paired before-and-after Faber refactors, read [design-transformations.md](design-transformations.md).

## Dishonest Hidden Ambient State

```faber
import from "norma:random" random

fn populate(mut ParticleWorld world, int count) → void {
    for range 0 ‥ count let i {
        world.particles.append(Particle { dx = random.random_int(-4, 4), dy = random.random_int(-4, 4) })
    }
}
```

The result depends on the process-wide generator absent from the signature. Any other `random` call, or a `random.seed` anywhere in the program, changes behavior at a distance. Passing a caller-owned `mut Rng` would make the effect honest, not pure.

The user-level version cannot be written: a module-level binding is `SEM062 top_level_binding`. Ambient state enters through Norma effect modules instead.

## Service Locator Disguised as Injection

```faber
fn populate(AppContext context, mut ParticleWorld world) → void {
    let int dx ← context.services.randomness.next_int(-4, 4)
    world.particles.append(Particle { dx = dx, dy = 0 })
}
```

Passing the global-shaped object does not reveal the function's actual requirements and grants access to unrelated authority.

## Interface per Deterministic Helper

```faber
interface AdditionService {
    fn add(int left, int right) → int
}

interface StringTrimmer {
    fn trim(string value) → string
}
```

These interfaces manufacture fakes rather than express real substitution or external authority. Call deterministic operations directly and test their results.

## A False Type-Level Guarantee

```faber
fn cross(UnitVec3 left, UnitVec3 right) → UnitVec3 {
    # Incorrect: the cross product has magnitude sin(theta).
    return UnitVec3 { value = cross_vectors(left.vector(), right.vector()) }
}
```

Parallel unit vectors produce zero. The return type certifies a mathematical falsehood, and it compiles because construction literals are not sealed. Return `Vec3`, or make normalization a separately named fallible operation.

## Validate and Forget

```faber
fn validate_port(int port) → void ⇥ PortError {
    if port ≺ 1024 {
        throw variant Privileged { port = port }
    }
}

class PortPool {
    mut set<int> free

    fn release(int port) → bool ⇥ PortError {
        validate_port(port)
        let bool fresh ← not self.free.has(port)
        self.free.add(port)
        return fresh
    }
}
```

The check learns that the port is non-privileged, returns `void`, and keeps the original `int`. Every later function must either trust undocumented history, repeat the check, or carry a `⇥ PortError` channel that should already be impossible. Parse the primitive into a `ServicePort` and make trusted operations accept that type.

Do not mechanically wrap every checked value. If the fact is temporary, contextual, or owned completely by one sealed aggregate, preserve it in that boundary instead. The problem is discarded knowledge, not the spelling of the helper.

## A Stale Proof Token

```faber
class InitializationCompleted {
}

fn initialize() → InitializationCompleted {
    return InitializationCompleted {}
}

fn use_resource(InitializationCompleted proof) → void {
    # Assumes the resource is still initialized.
}
```

An empty token proves nothing: any module can write `InitializationCompleted { }`, and the token outlives the state it describes. A proof-bearing type must carry or consume the live state it certifies; see the typestate refactor in [design-transformations.md](design-transformations.md#7-must-call-a-before-b).

## Effects Moved Outside Their Consistency Boundary

```faber
fn transfer(mut Repository repository, Transfer command) → void ⇥ TransferError {
    let Accounts accounts ← repository.load_accounts(command.from, command.to)
    let TransferPlan plan ← plan_transfer(accounts, command)
    repository.apply(plan)
}
```

Another writer may change the accounts between the check and the write. Purity has introduced a time-of-check/time-of-use bug. Load or lock, decide, write, and commit inside the required transaction.

## Parameter Bag Instead of a Concept

```faber
class DoEverythingOptions {
    let string user_id
    let string raw_sql
    let int retry_count
    let bool send_email
    let string output_path
    let Logger logger
}
```

The fields do not form one coherent value. The bag hides excessive responsibility and permits combinations that may not make sense.

## Primitive and Boolean Soup

```faber
fn schedule() → void {
    set_timer("refresh", 5, true, false, true)
}
```

The call site does not communicate units or boolean meaning. Use enums, ranged types with unit-bearing names, or one cohesive options class when the configuration is genuinely one concept.

## Mechanical Helper Explosion

```faber
fn get_name(User user) → string {
    return user.name
}

fn name_is_empty(User user) → bool {
    return get_name(user).length() ≡ 0
}
```

The helpers add navigation without adding vocabulary, policy, safety, reuse, or a meaningful testing boundary.

## Wrapper Ladder

```faber
fn handle(Request request) → Response ⇥ RequestError {
    return process_request(request)
}

fn process_request(Request request) → Response ⇥ RequestError {
    return execute_request(request)
}
```

Pass-through layers make local reasoning worse unless they translate a boundary, own policy, or create an actual contract.

## Weakest Type, Missing Semantics

```faber
fn median(list<int> sorted) → int {
    if sorted.length() ≡ 0 then return 0
    return sorted[sorted.length() / 2]
}
```

`list<int>` is the weakest type that compiles, but the operation needs at least one element, so it invents `0` for the empty case. Ask for `list<int, size 1…∞>` and let the caller parse, or return `int ∪ none` honestly. The weakest type is not automatically the most truthful.

## Callback as Accidental Authority

```faber
fn for_each_item(list<Item> items, (Item) → void visit) → void {
    # visit may block, print, or reach any Norma effect module.
    for from items let item {
        visit(item)
    }
}
```

A closure makes the authority explicit but does not make the operation pure. Specify ordering, error, and early-stop semantics when they matter; a closure is its own frame and cannot throw on the caller's `⇥` channel. Prefer a `generator` when caller-controlled consumption is the better contract.

## Faked External Semantics

```faber
class AlwaysSaves implements Repository {
    fn save(Account account) → void ⇥ StoreError {
    }
}
```

This cannot prove uniqueness constraints, transaction isolation, query behavior, rollback, ordering, or connection failures. Use real hermetic adapters and shared contract tests where those behaviors matter.

## Common Rationalizations

| Rationalization | Reality |
| --- | --- |
| "Everything should be injectable." | Inject caller-owned policy, nondeterminism, or external authority; keep ordinary implementation details local. |
| "An interface makes it testable." | Deterministic concrete code is already testable through inputs and outputs in a `test` block. |
| "Invalid states should always be impossible." | Only encode invariants the type can establish and preserve without making legitimate restoration or evolution impossible. |
| "Effects belong at the top." | Effects belong at the narrowest boundary that preserves consistency, ownership, and lifecycle semantics. |
| "The weakest type is most reusable." | The truthful type includes length, range, ownership, and performance guarantees, not only the operations called today. |
| "`mut` costs nothing." | `mut` grants write-back to the caller's value; ask for it only when the contract changes that value. |
| "Small functions are easier to read." | Functions are easier to read when they encapsulate meaningful concepts; wrapper ladders increase cognitive load. |
| "The fake passed." | A fake proves agreement with the fake, not the external system. |
