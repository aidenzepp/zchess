# Parse, Don't Validate

Use this guide when code checks raw input but keeps using the same weak representation afterward.

## The Information Test

Validation and parsing may perform the same runtime check. Their contracts differ:

```faber
fn validate_non_empty(list<int> values) → void ⇥ string {
    reject values.is_empty() throw "expected at least one value"
}

fn parse_non_empty(list<int> values) → list<int, size 1…∞> ⇥ string {
    reject values.is_empty() throw "expected at least one value"
    return values ↦ list<int, size 1…∞>
}

fn first_reading(list<int, size 1…∞> readings) → int {
    return readings.first()
}
```

The validator returns `void` and discards what it learned. The parser returns a representation that preserves the fact. Downstream code accepting `list<int, size 1…∞>` gets a `first()` that returns `int`, not `int ∪ none`, because emptiness is no longer representable.

Treat parsing as the conversion of less-structured data into more-structured data. Its input need not be text. Useful parse boundaries include:

- primitive to ranged or domain value
- raw list to sized list or invariant-bearing collection
- JSON `value` to domain entity
- stored record to restored aggregate
- CLI operands to application command
- bytes or values imported from a target-language module to native representations

## Boundary Shape

Keep weak data and fallibility at the edge:

```faber
fn parse_request(RawRequest raw) → Request ⇥ RequestError {
    reject raw.path ≡ "" throw variant MissingPath
    return Request { path = raw.path }
}

fn execute(Request request, mut State state) → ExecutionResult {
    state.served ← state.served + 1
    return ExecutionResult { path = request.path }
}
```

Do not spread `⇥ RequestError` through `execute` when `Request` already proves the relevant facts. A different operational error may still be legitimate.

In Faber, common parser shapes are:

- a named function `parse_*(Raw raw) → Domain ⇥ DomainError` with a `union` payload
- an `@ conversion fn` from the raw type to the domain type, so the boundary can write `raw ↦ Domain ⊥ fallback`; one declared with `⇥` makes every `↦` failable
- a conversion into a ranged or sized type: `n ↦ trapping<u16, 1024…65535> ⊥ fallback`, `xs ↦ list<int, size 1…∞>`
- a fallible aggregate constructor function

A bare `↦` that propagates its failure needs a `⇥ string` channel (`SEM016 conversio_failure_channel_textus_required`). For a typed payload, check explicitly and `throw variant …` before converting.

Mark the representation `@ private`, and remember that any importing module can still write the construction literal: prefer a ranged or sized field type when it can carry the invariant.

## Choose the Right Proof Owner

### Value owns the proof

Use a domain value when the fact is intrinsic and should travel:

```faber
fn reflect(Vec3 incident, UnitVec3 normal) → Vec3 {
    let Vec3 n ← normal.vector()
    return subtract(incident, multiply(n, 2.0 * dot(incident, n)))
}
```

The signature removes invalid calls and the corresponding error path from trusted code. `PortPool.release(ServicePort port) → bool` and `AssetCatalog.is_asset_of_type(AssetName name, AssetType expected) → bool` in [examples.md](examples.md) have the same shape.

### Aggregate owns the proof

Use a private aggregate when the rule is meaningful only inside the aggregate:

```faber
type NodeId = u32

class DependencyGraph {
    @ private
    mut map<NodeId, set<NodeId>> edges
}
```

If every method that adds or removes edges preserves graph membership rules and raw edges never escape as proof-bearing values, a separate `ValidatedEdge` may add nothing.

### Live state owns the proof

Some facts are observations, not durable value properties:

- a username is currently available
- an account currently has sufficient funds
- a row still has a particular version
- a caller currently holds a permission

Do not parse these into timeless wrappers that can become stale. Keep the check and action in the same transaction, lock, lease, guard, or version-checked operation.

## Normalize Only When the Domain Defines It

Parsing may establish a canonical representation when equivalence is part of the domain:

```faber
fn parse_asset_name(string raw) → AssetName ⇥ AssetNameError {
    reject raw.length() ≡ 0 throw variant Empty
    reject raw.contains(" ") throw variant ContainsSpace
    return AssetName { text = raw.lowercase() }
}
```

Do not silently repair ambiguous input. Trimming, case folding, path resolution, Unicode normalization, unit conversion, saturating stores, and lossy `↦ … via` rounding require an explicit policy.

## Construction Is Not the Only Boundary

Preserve the invariant across:

- methods that change `@ private` fields
- construction literals written in other modules
- JSON, TOML, and file parsing
- restoration from storage
- migrations and bulk imports
- values imported from target-language modules (`import python …`)
- arithmetic and transformations

An operation must return the strongest type it can actually prove. A cross product of two unit vectors is not necessarily a unit vector. A previously available username is not necessarily still available.

## Review Procedure

1. Search for `validate`, `is_valid`, functions returning `void ⇥ …` that only check, repeated `require` / `reject` on the same fact, primitive-heavy signatures, and `union` error variants described as impossible.
2. State exactly what each check learns.
3. Decide whether the fact belongs to a value, aggregate, or live consistency boundary.
4. Make the boundary return that proof owner.
5. Change trusted function signatures to accept the stronger representation.
6. Remove redundant checks and impossible error variants.
7. Test rejection at the parse boundary and preservation through every public operation and restoration path in `describe` / `test` blocks.

## Sources

- [Parse, don't validate — Alexis King](https://lexi-lambda.github.io/blog/2019/11/05/parse-don-t-validate/) — the original type-driven-design explanation: parsing preserves information that validation returning unit discards.
- [Faber corpus: `list<T, size lo‥hi>`](https://faberlang.dev/en-US/corpus/list%3CT%2C%20size%20lo%E2%80%A5hi%3E.html) — a length range whose minimum is one makes `first()` and `last()` total.
- [Faber corpus: `trapping<W, lo‥hi>`](https://faberlang.dev/en-US/corpus/trapping%3CW%2C%20lo%E2%80%A5hi%3E.html) — a ranged integer checked at every store.
- [Faber corpus: `conversio-functio`](https://faberlang.dev/en-US/corpus/conversio-functio.html) — `@ conversion` functions behind `↦`, including failable ones.
