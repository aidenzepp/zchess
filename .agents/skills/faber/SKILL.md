---
name: faber
description: Use when writing, reviewing, or debugging Faber (`.fab`) source, `faber.toml` manifests, or `faber` CLI output in this repository, including diagnostics such as `SEM010` or `PARSE050`, Norma imports, or translating an idea from another language into Faber.
---

# Faber

Faber is an experimental, type-first, package-oriented language. Meaning lives in a semantic core (HIR); English, Latin, and other reader locales are renderings of it, and Radix, the compiler bundled in the `faber` CLI, emits Rust, Go, TypeScript, LLVM, and other targets from the same package. The language changes in minor releases until 2.0, so model priors and older Faber code are unreliable. Read the documentation for the installed version before writing or reviewing source.

## Where to look

Read in this order. Stop when the question is answered.

1. **Agent canon:** https://faberlang.dev/agents/index.md. The writing guide for functions, types, errors, control flow, generics, modules, packages, and libraries. Read its pages in order and follow only the links it names. Its programs are English-locale and checked against the current release.
2. **Grammar summary:** https://faberlang.dev/agents/grammar.md. Every form the canon uses on one page, with the "do not write" list.
3. **Corpus pages:** `https://faberlang.dev/en-US/corpus/<english-word>.html`. The canon does not cover these constructs; go straight to their pages: `class`, `interface`, `union`, `variant`, `match`, `enum`, `guard`, `describe`, `test`, `assert`. Each carries a checked source program and reject examples. Some pages are stubs that only say "Continue to <term>"; open the named page.
4. **Keyword catalog:** https://faberlang.dev/llms-full.txt. A lookup list for anything else, not a guide. Each entry's `Canonical` field is the Latin term (`genus`, `discretio`, `probandum`); an English file must use the English alias (`class`, `union`, `describe`). The entry's `Page` URL names the corpus page.
5. **Diagnostics:** `faber explain <CODE>` or `faber explain <CODE>.<slug>`, and https://faberlang.dev/agents/reference/index.md.
6. **Release notes:** https://faberlang.dev/en-US/releases/. Read the notes for the installed version before trusting older code or examples; breaking changes ship in minor releases.

Faber also publishes agent skills at https://faberlang.dev/.well-known/agent-skills/index.json (`install`, `language`, `packages`, `corpus`). They route to the same canon.

## Local toolchain

- `faber --version` reports the installed release. Match the documentation to it.
- `faber check .` lexes, parses, and type-checks the package. `faber run .` interprets it. `faber test .` runs `describe` / `test` suites. `faber format .` reports drift; `--write` applies it.
- `faber doc .` renders exported declarations with the `#` comment lines directly above them.
- Prove a snippet by putting it in a scratch package (`faber init work/<name>`) and running `faber check` on it. Delete the scratch package afterward.
- `faber check .` analyzes only files reachable from the entry. A file nothing imports can hold type errors and still pass; check it by path (`faber check src/<file>.fab`). `*.proba` test files are analyzed only by `faber test`.

## Known tool gaps in 1.17.0

Each was reproduced locally. Recheck after upgrading.

- `faber format --write` deletes `#` comments on union variants, enum members, and interface methods, although `faber doc` renders them. Document members in the comment on the type itself.
- `faber format` refuses any file that imports a Norma module (`norma:text`, `norma:json`, …) with `PKG001 unknown_library_module` and writes nothing, while `check` and `run` accept it.
- A `class` used as a `map` or `set` key passes `faber check`, then fails at run time with `value is not a valid map/set key`. Key by `string`, an integer, or a ranged integer type.

## Verify before trusting

- Code in this repository's history, other repositories, or memory may target an older release. Check it before copying it.
- A doc page and the installed CLI can disagree. The installed `faber check` is the authority for what compiles here; report the disagreement.
- Do not infer that a construct is missing from one failed guess. Search the catalog by English alias and by Latin term.
- A diagnostic can point away from its cause. A misspelled or wrong-locale keyword surfaces as `PARSE030` or `SEM001` on a later line. Check keyword spelling first.

## Common mistakes

Each row was confirmed against `faber check` 1.17.0; the code is the diagnostic the wrong form produces.

| Wrong | Right | Diagnostic |
|---|---|---|
| `name: int`, `int?` | `int name`, `int ∪ none` | `PARSE001` |
| `//` comments, trailing `#` | `#` on its own line | |
| `let x = 0` | `let int x ← 0`, or `let _ x ← 0` to infer | `PARSE010 binding_type_slot_required` |
| Runtime `const int n ← 0`, `var` | `let` (immutable) or `mut`; `const int N = 3` is compile-time only | `PARSE010 runtime_const_retired`, `var_retired` |
| `mut` binding at the top of a file | State starts in `main` and lives in functions | `SEM062 top_level_binding` |
| `list<int, 4>` | `list<int, size 4>` | `SEM008 list_count_needs_size` |
| Bare `⇥` call outside `main` | `do` / `catch`, or a `⊥` recovery value | `SEM011 failable_call_requires_handler` |
| `f(n)` for a `mut` parameter | `f(mut n)`; calls repeat `mut` and `own` | `SEM057 call_marker_missing` |
| Latin catalog term in an English file (`discretio`, `genus`) | English alias (`union`, `class`) | `PARSE030`, `SEM001`, often on a later line |
| `match` missing a variant | Cover every variant, or add `default` | `SEM040 missing_variants` |
| `variant Rook ∷ Piece` where the type is already known | `variant Rook` | `WARN005 unnecessary_cast` |
| `a ∨ b`, `¬a` on `bool` (these are bitwise) | `a or b`, `not a` | `SEM011 integer_operands_required` |
