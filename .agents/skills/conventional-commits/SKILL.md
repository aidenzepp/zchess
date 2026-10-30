---
name: conventional-commits
description: Use when staging or committing changes in a git repo; split work into targeted commits with idiomatic conventional-commit prefixes and a clear DAG story.
---

# Conventional Commits

Use this skill when preparing commits.

The goal is a commit history that reads like a small design document. Each commit should be targeted, reviewable, and meaningful on its own. A commit does not need to be large; it needs to represent one coherent reason for change.

Before preparing commits, read [references/examples.md](references/examples.md). Its labeled POSITIVE and NEGATIVE pairs define acceptance boundaries for prefixes, subjects, commit scope, staging, and useful bodies.

## Commit Prefixes

Use these prefixes as the default vocabulary:

- `feat:` for new product behavior, new developer-facing capability, or new reusable API surface
- `fix:` for correctness fixes, regressions, crashes, broken workflows, or user-visible bugs
- `refactor:` for behavior-preserving structure, naming, boundary, or API reshaping
- `chore:` for repository maintenance, dependency updates, generated metadata, tooling, config, or cleanup
- `test:` for test-only changes or reusable test infrastructure
- `docs:` for documentation-only changes
- `style:` for presentation-only UI/CSS changes that do not alter behavior
- `perf:` for measurable performance work
- `build:` for package manifest (`faber.toml`), compiler, target, or artifact pipeline changes
- `ci:` for CI workflow changes
- `revert:` for explicit reversions

Prefer the most specific truthful prefix. Avoid vague subjects such as `update`, `cleanup`, `misc`, or `changes`.

Optional scopes are fine when they clarify the boundary (`feat(search):`, `fix(fen):`). Do not invent noisy scopes.

## Split Commits By Story

Before staging, inspect the whole change:

```bash
git status --short
git diff --stat
git diff
```

Then split by story boundary:

- separate unrelated concerns, even when they touched nearby files
- separate tooling/config/dependency changes from product or library behavior
- separate reusable API additions from their first usage when each is independently meaningful
- keep tests with the behavior when the tests are the proof for that behavior; Faber tests are usually `describe`/`test` blocks in the same `.fab` file as the code they prove, so the fix and its proof often share a file
- split reusable test infrastructure into its own `test:` commit when it supports later commits
- keep mechanical formatting separate only when it would obscure the semantic diff
- exclude demos, scratch files, generated churn, and local experiments unless the user explicitly wants them kept

Each commit should represent one coherent change and leave the repository working. Keep everything required for that change together. Separate unrelated changes so each can be reverted independently. Dependencies between commits are acceptable; make them clear rather than splitting tightly coupled work into broken intermediate states.

Atomic means one complete change, not necessarily a small number of files.

## Stage Carefully

Stage by path or hunk so the index matches the intended story:

```bash
git add path/to/file
git add -p
git diff --cached --stat
git diff --cached
```

Avoid `git add .` unless the working tree has already been inspected and every changed file belongs in the same commit.

Because a Faber source file often holds both code and its tests, one file can carry two stories. Stage by hunk in that case, and keep each behavior hunk with the `test` hunk that proves it.

## Write The Message

Use an imperative, concise subject:

```text
feat: add perft node counts to the CLI
fix: reject castling through an attacked square
refactor: split move generation into its own module
```

Add a body only when the commit needs context that is not obvious from the diff:

- why this boundary exists
- what was deliberately excluded
- what migration or compatibility trade-off was chosen
- what verification proves the change

## Verify The Boundary

Before committing, run the narrowest useful checks for the staged story. In a Faber package the normal hygiene commands are:

```bash
faber format .
faber check .
faber test .
```

`faber format .` reports drift and exits 1 without writing; `faber format . --write` applies it. `faber check .` lexes, parses, and type-checks. `faber test .` runs the `describe`/`test` suites.

These commands read the working tree, not the index. After hunk staging, the staged state has not been checked until the unstaged hunks are set aside (for example `git stash push --keep-index`, run the checks, then `git stash pop`).

If a check is skipped, say why in the handoff. If a commit is intentionally not green on its own because it is part of a local migration sequence, make that explicit before committing.

## Final Pass

Before reporting completion:

- confirm `git status --short`
- confirm the new log tells the story in order
- report the commit hashes and the checks that ran
- do not push unless the user confirms the remote target
