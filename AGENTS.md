# zchess

A Faber package. `faber.toml` is the manifest; source lives under `src/` with `src/main.fab` as the entry.

## Toolchain

Faber is experimental and ships breaking changes in minor releases. Check `faber --version` and read the matching documentation before writing or reviewing `.fab` source; do not rely on memory or older Faber code. Use `.agents/skills/faber` to find the documentation.

## Workflow

Before completion, run:

```bash
faber format .
faber check .
faber test .
```

Resolve all diagnostics and warnings before finishing. In Faber 1.17.0, `faber format` refuses files that import Norma modules; see `.agents/skills/faber` for this and other known tool gaps. Keep scratch packages and probes in `work/`, which is not committed.

## Skills

- `.agents/skills/faber`: what Faber is, where its documentation lives, and verified common mistakes.
- `.agents/skills/conventional-commits`: commit decisions, boundaries, staging, and messages.
- `.agents/skills/designing-trustworthy-functions-and-data`: functions, domain types, data structures, parsing boundaries, dependencies, effects, invariants, and abstraction levels.
- `.agents/skills/document-code`: documentation for functions, types, modules, and other named program units.
- `.agents/skills/error-messages`: user-facing and developer-facing error messages.
- `.agents/skills/self-explanatory-tests`: test design, organization, failure messages, and debugging.
