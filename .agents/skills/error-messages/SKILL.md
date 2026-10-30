---
name: error-messages
description: Write or review user-facing and developer-facing error messages. Use when adding validation errors, API errors, CLI errors, UI alerts, empty/error states, permission failures, quota errors, retry failures, or any copy that tells a user something went wrong.
---

# Error Messages

Use this skill to make errors clear, actionable, and appropriate to the audience.

Good error messages are not copy polish after the fact. First inspect the error boundary: what failed, who can fix it, what state is preserved, whether retry can work, and what structured details are available.

## Core Test

Every error should answer these questions as directly as the surface allows:

- What happened?
- Why did it happen, when known and safe to reveal?
- What can the user do next?
- What state was preserved or lost?
- Where can the user go if the primary fix does not work?

If the code cannot answer those questions, improve the error model before rewriting prose when scope allows.

## Workflow

### 1. Identify the audience

Decide which surface you are writing for:

- End-user UI: use plain product language and avoid implementation detail.
- Developer API: be concise, precise, and structured.
- CLI/log output: include enough context for search, automation, and debugging.
- Internal operator tooling: include identifiers, affected resource, and next diagnostic step.

Do not make one string serve every audience if the system has separate user, developer, log, and machine-readable surfaces.

### 2. Inspect the real failure

Before writing the message, find:

- the failing operation
- the invalid input or missing requirement
- the expected constraint or allowed range
- whether the failure is caused by the user, product, network, dependency, permission, quota, or incompatible state
- whether the user's data or progress was saved
- whether retry, changing input, changing permissions, contacting support, or waiting is actually useful

Do not offer "Try again" when the system can already tell retry will fail.

### 3. Write the message

Use this structure when the surface can support it:

```text
<what happened>
<why it happened or what constraint was violated>
<what to do next>
<fallback or support/debug identifier when useful>
```

For compact inline errors, collapse the structure into one sentence:

```text
Enter a 5-digit ZIP code
```

For blocking dialogs or API errors, include more context:

```text
Unable to connect your account
Your changes were saved, but we could not connect the account because the provider is unavailable. Try again later or contact support if this keeps happening.
```

### 4. Preserve state and blame carefully

- Say what was preserved: "Your changes were saved."
- Say what failed without shaming the user.
- Prefer instructions over accusations: "Enter a name" instead of "You did not enter a name."
- Avoid cute tone, humor, exclamation points, and excessive apologies.
- Use "we" sparingly; focus on the user's task and next action.

### 5. Make invalid input specific

When input is invalid, show both the offending value and the expected requirement when safe:

```text
The specified bid ($5) is below the minimum bid ($8)
```

For sensitive values, identify the field and constraint without echoing secrets:

```text
Password must include at least 12 characters
```

For long values, truncate or use progressive disclosure instead of flooding the user.

### 6. Make API errors structured

For APIs and machine callers, avoid making clients parse prose. Provide:

- stable error code or reason
- human-readable message
- field-level violations when applicable
- whole-error context such as resource, region, limit, actual value, retry time, draft ID, or support link when relevant
- item-specific details such as field violations, row violations, missing permissions, or quota violations

Keep dynamic facts in `details` or `context` if they also appear in the human message.

Prefer a small boundary payload unless the repo already has a stronger local convention:

```json
{
  "code": "stable_machine_category",
  "message": "Safe, audience-appropriate text for this boundary.",
  "details": [],
  "context": {}
}
```

Use the fields this way:

- `code`: stable machine-readable category.
- `message`: safe, audience-appropriate human text for this boundary.
- `details`: repeated or item-specific repair data, such as fields, rows, missing permissions, or violated limits.
- `context`: curated whole-error state, such as preserved draft IDs, affected resource, retry time, request ID, help URL, or available actions.

Do not add top-level fields such as `requestId`, `helpUrl`, or `actions` by default. Put optional, situation-specific facts in `context` unless the existing API contract already standardizes them.

In Faber, declare each boundary payload as concrete `@ json` classes and serialize with `↦ json` and `json.stringify` from `norma:json`. Faber 1.17 rejects generic `list<D>` fields and raw `json` fields inside an `@ json` class (`SEM018:json_genus_field_type_unsupported`), so each boundary gets its own typed `details` and `context` classes. Keep it that way: context should be curated, not an open bag.

For Faber internals, the `⇥` payload type is the error contract between functions:

- Use a `union` with one variant per failure that needs a different fix, or a small `class`, not `⇥ string`. A string payload forces callers to compare prose.
- Variants carry facts (path, field, limit, actual value), not finished sentences.
- `match` over a union is exhaustive. Adding a failure kind fails `faber check` (`SEM040:missing_variants`) at every boundary that has not chosen its code and copy. Avoid `default` arms in boundary renderers so this keeps working.
- Do not invent one universal application error type. Give each layer the payload its callers branch on.
- Normalize at the API, UI, CLI, or SDK edge, either inside `catch err` or through an `@ conversion fn` from the domain payload to the boundary payload. With that conversion, a bare call propagates across layers. Without it, the call fails with `SEM011:failable_call_conversion_absent`.
- Reserve `panic` for broken invariants. It cannot be recovered, and under `faber run` it prints `abort: <message>` to stderr and exits 70. A `⇥` error that escapes `main` does the same and prints the raw payload, such as `abort: MoveError.Illegal { san = "e5" }`. A user-facing CLI catches and renders every error before `main` returns.

For Faber CLIs:

- Write error text to stderr with `warn`. `warn` prints the string as given, with no prefix. Keep stdout for results.
- Set the exit code with `main args argv exit <expr>`. The expression runs after the body and cannot see the body's locals, so put the work in a function that returns the code: `main args argv exit run(argv.name) { }`.
- Reuse the exit codes the `faber` CLI itself uses: 64 bad arguments, 65 bad input data, 66 missing input, 69 unavailable, 70 internal, 73 output, 78 configuration.

Self-supporting multi-sentence messages belong where the code owns the final human surface: API response mapping, UI rendering, CLI output, SDK-facing errors, or operator tools. Deep domain, repository, storage, queue, crypto, or generic library code should usually return compact errors plus structured facts and causes.

### 7. Place and format the error

- Put field errors next to the field.
- Summarize multi-field form errors at the top and mark each field.
- Keep the user's input intact whenever possible.
- Use color plus text or icon cues; do not rely on color alone.
- Use progressive disclosure or docs links for long explanations.
- Keep terminology consistent across the product.

## Anti-Patterns

Avoid:

- `Something went wrong`
- `Unknown error`
- `Invalid input`
- `Permission denied`
- `Try again later` with no reason or fallback
- blame-shifting to the user, server, provider, or third party
- implementation jargon on end-user surfaces
- one generic catch-all for distinct failures with different fixes
- a human-readable sentence as the only machine-readable API contract

## Review Checklist

Before calling the error done, check:

- The message matches the actual condition.
- The next action is concrete and possible.
- Any invalid value, missing permission, limit, or constraint is named.
- The user is reassured about preserved data when relevant.
- The surface has an escape hatch: retry, edit, wait, docs, contact support, or error ID.
- API/CLI errors include stable identifiers for search and automation.
- Faber `⇥` payloads are typed (`union` or `class`), not `string`, wherever a caller branches. CLI errors go to stderr and set a distinct exit code.
- The tone is neutral, respectful, concise, and not cute.
- The message is close to the error and accessible without color alone.

See `references/examples.md` for concrete rewrites and code patterns.
