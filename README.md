# zchess

Chess on an 8×8×8 board.

zchess is an experimental three-dimensional chess variant written in
[Faber](https://faberlang.dev). The rules are the code in `src/`. They carry
over from an earlier version of the game and will change as it gets played.

## The board

The board is a cube of 512 cells, eight along each axis. A cell is named by
its file, rank and tier, in that order.

| Axis | Labels   |
| ---- | -------- |
| File | A to H   |
| Rank | 1 to 8   |
| Tier | S to Z   |

`D4V` is file D, rank 4, tier V. Each tier is a full 8×8 board, and the eight
tiers are stacked in order from S to Z. The edges do not wrap.

## Developing

Requires Faber 1.17.0. From the repository root:

```console
$ faber check .
$ faber test .
$ faber format .
```

Coding agents start at `AGENTS.md`. The skills in `.agents/skills/` cover the
Faber documentation, the known 1.17.0 tool gaps, and how this project writes
commits, errors, tests and doc comments.

## License

MIT
