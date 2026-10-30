---
name: document-code
description: Use when writing or reviewing Faber (`.fab`) functions, methods, classes, unions, enums, interfaces, module constants, `main` entries, or other named program units, and when debugging, upstream documentation, source inspection, or experiments reveal a non-obvious constraint the code must preserve.
---

# Document Code

Make each program unit's contract and system role clear before a reader studies its implementation.

## Required coverage

Add a doc comment to every module-scope `fn`, `class`, `union`, `enum`, `interface`, and `const`, public or file-private, and to every class method. Bindings inside a function body are not program units.

Enum members, union variants and their payload fields, and interface methods are structural parts of their type, so the type's comment owns their meanings. `faber format` deletes comments on members, variants, and interface methods; a payload field comment is `PARSE060`. Class fields and methods keep their own comments.

A `main` entry's user contract is its `@ description` and each operand and option `description`, which the built program prints as `--help`; a `#` comment above `main` holds maintainer facts only. A `describe` or `test` name is its documentation, and a `#` line directly inside a `describe` body is `PARSE060`.

## Format every doc comment

A doc comment is the unbroken run of `#` lines directly above a declaration and its annotations:

```faber
# The largest number of plies a stored game may hold.
@ public
const int MAX_GAME_PLIES = 1024
```

- A blank line ends the run; lines above it are not attached.
- Put the run above `@ public`. `faber format` moves a comment written between an annotation and its declaration above the annotation.
- Separate paragraphs with a bare `#` line.
- Never use `//` or a `#` after code on the same line.

`faber doc` prints each `@ public` declaration's signature with its comment verbatim beneath it. File-private units, `main`, and tests are not rendered but follow the same rules. `faber check .` skips files the entry does not import; check a new file by path.

## Write in the repository's voice

- Open with a direct, complete sentence that ends with a period. `faber doc --json` takes it as the unit's `summary`.
- Start functions and methods with an active verb such as “Builds,” “Plays,” or “Converts.”
- Start types and constants with a definitional noun phrase such as “A square…” or “The halfmove clock…”.
- Add one concise follow-up only for a mental model, invariant, rationale, ecosystem fit, or ownership boundary.
- Describe the consumer-facing contract, not the implementation sequence.
- Reuse the exact vocabulary of the domain, protocol, and Faber construct.
- Omit filler. A comment that only expands the symbol's name is incomplete documentation.

## Preserve consequential knowledge

After debugging, reading documentation or upstream source, or running experiments reveals new knowledge, re-review the documentation on the affected code even when an existing comment already looks complete.

Embed the knowledge when all three conditions hold:

1. It changes which implementation is correct, safe, or interoperable.
2. It is not apparent from the current code, signatures, or ordinary ecosystem knowledge.
3. A reasonable maintainer could remove the constraint or repeat costly discovery without it.

Write qualifying knowledge for a first-time reader:

1. State the program unit's current contract or role.
2. Give the causal chain as present facts: the governing invariant, why it requires the current design, and the guarantee that design provides.
3. Add navigation for the causal chain: name a local owning unit when the relationship crosses a module boundary, canonical documentation for public behavior, and pinned upstream source when exact implementation matters.

The explanation must stand alone without knowledge of past implementations or the incidents, prior defaults, successes, and failures that led to it. Translate discovery history into present-tense invariants; do not write “previously,” “this fixes,” “after this failed,” or similar chronology.

Put the knowledge in the smallest owning program unit's doc comment. Use a `#` comment inside a body only when the constraint belongs to one statement or to execution order and moving it outward would obscure the relationship.

Do not add a knowledge comment when the code or signature already makes the fact clear, when the compiler, another skill, or a lint rule owns the policy, or when the information is only temporary debugging evidence. Those comments add maintenance cost without improving understanding.

## Document what the signature cannot prove

A signature already states parameter types, borrow markers, the result type, the failure payload type (`⇥ E`), possible absence (`∪ none`), and sized types such as `list<T, size N>`. Never restate those. Write what they cannot say:

| Program unit             | Required documentation                                                                                                   |
| ------------------------ | ------------------------------------------------------------------------------------------------------------------------ |
| Function, method, helper | Each parameter named in backticks with its role, unit, or domain constraint, and the semantic result.                    |
| `⇥ E` failure            | The conditions that throw and what the payload tells its consumer, not “throws a string.”                                |
| `T ∪ none` result        | When the result is `null` and what that state means, not “may return null.”                                             |
| `mut` parameter          | What the caller observes after return, including after a throw.                                                          |
| `class`                  | What it models and its invariants. Comment a field only for a constraint, unit, or meaning the class comment lacks.     |
| `union`, `enum`          | What the type represents, then each variant, member, or payload field whose name does not settle its meaning.            |
| `interface`              | The boundary, then every method's contract as for a function. Implementing classes do not repeat it.                    |
| Module `const`           | The invariant, policy, shared consumers, or system role the value establishes.                                           |
| `main` entry             | `@ description` says what the program does; each operand and option `description` says what the value must be.          |

For a `void` function, state the result only when completion establishes a side-effect contract. Document each fact once at the surface that owns it, not on every function that accepts the type.

Faber has no overloading; a second `fn` with the same name is `SEM005` `duplicate_definition`. Sibling functions for different representations each own their contract; do not copy one comment across them.

Add these only when their condition holds:

- “See `unit`” for a useful relationship to a real unit, counterpart, or protocol.
- An inline `test` for non-obvious public usage. A test is a checked example; prose in a comment is not.
- For a deprecated unit, a first sentence that says so and names the replacement. Faber 1.17 has no deprecation annotation.

Small file-local helpers still receive a concise purpose plus their parameters and result, without optional additions.

## Cite authoritative sources

Add a link whenever a reader would otherwise need to rediscover the external rule behind the code, such as a chess rule, file format, protocol, engine-specific value, or Faber toolchain behavior.

- Link canonical documentation for the supported public behavior.
- Also link a versioned source permalink when the exact implementation or emitted value matters.
- Put each URL on its own `#` line at the end of the comment; name repository units in backticks.
- Place the link on the smallest unit that owns the sourced behavior.
- Omit generic homepages and links that add nothing beyond an import or parameter type.

## Labeled examples

**REQUIRED REFERENCE:** Read [references/examples.md](references/examples.md) before writing or reviewing code documentation. Treat each `POSITIVE` and `NEGATIVE` label as acceptance data: reproduce the documented contract quality, not merely the comment shape.

## Review

Confirm every required unit and parameter is documented; member meanings sit on their owning type; every comment run is contiguous, above its annotations, and survives `faber format`; `⇥` and `∪ none` prose explains conditions and meaning; and links sit beside sourced behavior. Read `faber doc` output for changed public units as a consumer would. After learning a non-obvious constraint, confirm the owning comment carries its present-tense causal chain. Reject comments that narrate syntax, restate signatures or names, recount history, or repeat policy the compiler enforces.
