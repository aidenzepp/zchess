# Code Documentation Examples

Use these examples as labeled acceptance data. The text immediately above each snippet states the evidence that determines its label. Each snippet passes `faber check` 1.17 once the types it names exist.

## Types, members, and parameters

### NEGATIVE — Member comments restate names and do not survive formatting

**Decisive evidence:** Every comment only repeats a name, and nothing says what the `Piece` payload means. `faber format` deletes variant comments, so even accurate text there would vanish.

```faber
# The Occupant union.
@ public
union Occupant {
    # Empty.
    Empty,
    # Piece.
    Piece {
        Colour colour
        Kind kind
    }
}
```

### POSITIVE — The type comment owns its variant meanings

**Decisive evidence:** The variants stay structural. The union comment survives formatting and reaches `faber doc`, so it defines each variant and the invariant once.

```faber
# What stands on one square of a board.
#
# `Empty` marks a square no piece occupies. `Piece` carries the occupant's
# `colour` and `kind`; a square never holds more than one piece.
@ public
union Occupant {
    Empty,
    Piece {
        Colour colour
        Kind kind
    }
}
```

### POSITIVE — A function owns its `mut` write-back and failure contract

**Decisive evidence:** The signature already shows that `board` is written back and that failure carries a `string`. The comment adds what the caller observes after return and after a throw, and what the payload means.

```faber
# Plays `move` on `board` for the side to move.
#
# On return, `board` holds the resulting position with the other side to
# move. An illegal `move` throws before `board` changes; the payload quotes
# the move for the player.
@ public
fn apply_move(mut Board board, Move move) → void ⇥ string {
    require is_legal(board, move) throw "§ is not a legal move"(move.uci)
    board.history.append(move.uci)
    board.white_to_move ← not board.white_to_move
}
```

### POSITIVE — An interface owns its reusable boundary

**Decisive evidence:** Callers consume this interface independently of any implementation. `faber format` deletes interface method comments, so the interface comment records the method contract once for every caller.

```faber
# The source of moves for one side of a game, such as a player at the
# terminal or a UCI engine.
#
# `next_move(board)` returns the move this side plays in `board`, in UCI long
# algebraic form such as `e2e4`. It throws when the source can no longer
# answer, such as a closed engine process; the payload explains why for the
# game log.
@ public
interface MoveSource {
    fn next_move(Board board) → string ⇥ string
}
```

## File-local helpers and module constants

### NEGATIVE — Documentation decorates names without adding a contract

**Decisive evidence:** These comments only turn identifiers into sentences. They do not state which layers share the limit or which coordinate convention the helper assumes.

```faber
# The fifty move plies.
const int FIFTY_MOVE_PLIES = 100

# Square index.
fn square_index(int file, int rank) → int {
    return rank * 8 + file
}
```

### POSITIVE — Concise comments explain system role and guarantee

**Decisive evidence:** The constant identifies the consumers that must agree. The helper states what each parameter means and the exact result without optional additions.

```faber
# The halfmove clock at which draw detection and the claim prompt both open a
# fifty-move draw claim.
const int FIFTY_MOVE_PLIES = 100

# Converts board coordinates to the board-array index.
#
# `file` 0 is the a-file and `rank` 0 is White's first rank. Returns 0 for a1
# through 63 for h8.
fn square_index(int file, int rank) → int {
    return rank * 8 + file
}
```

## Sibling functions

Faber has no overloading; a second `fn` with the same name is `SEM005` `duplicate_definition`. Functions that accept different representations are separate units with separate contracts.

### NEGATIVE — Shared boilerplate obscures distinct contracts

**Decisive evidence:** Both comments are the same sentence. Neither distinguishes the accepted input, says when the first throws or what its payload carries, or says what a `null` from the second means.

```faber
# Parses a square.
@ public
fn square_at(int file, int rank) → Square ⇥ string {
    reject file ≺ 0 or file ≥ 8 throw "file § is off the board"(file)
    reject rank ≺ 0 or rank ≥ 8 throw "rank § is off the board"(rank)
    return Square { file = file, rank = rank }
}

# Parses a square.
@ public
fn square_from_index(int index) → Square ∪ none {
    if index ≺ 0 or index ≥ 64 then return null
    return Square { file = index % 8, rank = index / 8 }
}
```

### POSITIVE — Each function owns its differing contract

**Decisive evidence:** Each comment states its input and the meaning of its failure path: what the `⇥` payload tells a reader, and when `null` occurs and what it signals. Neither repeats the signature's types.

```faber
# Builds the square at a user-supplied file and rank.
#
# Throws when either coordinate falls outside 0 to 7; the payload names the
# offending coordinate for the player.
@ public
fn square_at(int file, int rank) → Square ⇥ string {
    reject file ≺ 0 or file ≥ 8 throw "file § is off the board"(file)
    reject rank ≺ 0 or rank ≥ 8 throw "rank § is off the board"(rank)
    return Square { file = file, rank = rank }
}

# Converts a board-array index from move generation to its square.
#
# Returns `null` when `index` falls outside 0 to 63, which marks a generated
# step off the board rather than bad user input.
@ public
fn square_from_index(int index) → Square ∪ none {
    if index ≺ 0 or index ≥ 64 then return null
    return Square { file = index % 8, rank = index / 8 }
}
```

## Optional additions

### NEGATIVE — Additions are present only to look complete

**Decisive evidence:** The relationship to `Board` is already the parameter type, the usage is ordinary, and the signature has no `⇥`, so "Never throws" restates it. These lines add no navigation or contract information.

```faber
# Lists the legal moves.
#
# See `Board`.
#
# Example: `legal_moves(board)`.
#
# Never throws.
@ public
fn legal_moves(Board board) → list<Move> {
    return candidate_moves(board)
}
```

### POSITIVE — Additions appear only where they change understanding

**Decisive evidence:** The link names the counterpart that consumes these moves. The ordering guarantee is a contract callers rely on. The usage example is a `test`, which the toolchain checks and runs; a prose example in the comment would not be.

```faber
# Lists the moves the side to move may play on `board`.
#
# The order follows `candidate_moves`, so it is stable for one position and
# perft counts compare across runs. See `apply_move`, which accepts exactly
# these moves.
@ public
fn legal_moves(Board board) → list<Move> {
    mut list<Move> moves ← empty
    for from candidate_moves(board) let move {
        if is_legal(board, move) {
            moves.append(move)
        }
    }
    return moves
}

test "the starting position has twenty legal moves" {
    assert legal_moves(start_board()).length() ≡ 20
}
```

## Consequential learned constraints

### NEGATIVE — Incident history assumes context the reader does not have

**Decisive evidence:** The comment names an earlier comparison and a bug instead of the two current rules that shape the key. Even with useful links, a first-time reader must reconstruct the causal chain.

```faber
# Replaces the FEN comparison that missed threefold repetitions.
#
# This fixes the draw-claim bug by dropping the en passant square sometimes.
#
# https://handbook.fide.com/chapter/E012023
# https://www.saremba.de/chessgml/standards/pgn/pgn-complete.htm#c16.1.3
@ public
fn repetition_key(Position position) → string {
    let string base ← "§ § §"(position.pieces, position.side, position.castling)
    if can_capture_en_passant(position) then return "§ §"(base, position.en_passant)
    return base
}
```

### POSITIVE — Present facts make the current design self-contained

**Decisive evidence:** The comment states the FEN rule, the repetition rule, and the resulting key without referring to any earlier implementation. Canonical sources let the reader verify both rules.

```faber
# Builds the key that identifies `position` for repetition claims.
#
# FEN records an en passant square after every double pawn push, even when no
# pawn can capture there. FIDE Article 9.2.3 treats positions as the same only
# when the same moves are possible. The key therefore keeps the en passant
# square only when an en passant capture is legal, so positions that differ by
# an unusable square compare equal.
#
# https://handbook.fide.com/chapter/E012023
# https://www.saremba.de/chessgml/standards/pgn/pgn-complete.htm#c16.1.3
@ public
fn repetition_key(Position position) → string {
    let string base ← "§ § §"(position.pieces, position.side, position.castling)
    if can_capture_en_passant(position) then return "§ §"(base, position.en_passant)
    return base
}
```

### NEGATIVE — A local comment repeats policy already owned elsewhere

**Decisive evidence:** The `mut` call marker is visible, and the compiler already requires it (`SEM057` `call_marker_missing`). The inline comment adds no local constraint or causal knowledge.

```faber
# Plays the opening move 1. e4 on `board`.
fn play_king_pawn(mut Board board) → void ⇥ string {
    # Passes `mut` because Faber requires the marker at every call.
    apply_move(mut board, Move { uci = "e2e4" })
}
```

### POSITIVE — Self-evident, enforced policy needs no local commentary

**Decisive evidence:** The marker is directly visible and no unusual local behavior depends on it, so the code is the clearest explanation.

```faber
# Plays the opening move 1. e4 on `board`.
fn play_king_pawn(mut Board board) → void ⇥ string {
    apply_move(mut board, Move { uci = "e2e4" })
}
```

## Authoritative source links

### NEGATIVE — A generic homepage does not explain the sourced behavior

**Decisive evidence:** The comment claims an engine-owned value but links only to a homepage. A reader must rediscover the protocol and cannot verify which engine version emits the mirrored value.

```faber
# Stockfish best moves.
#
# https://stockfishchess.org
@ public
union BestMove {
    Play {
        string uci
    },
    NoLegalMove
}
```

### POSITIVE — Documentation and versioned source make provenance verifiable

**Decisive evidence:** The protocol text defines the supported `bestmove` line. The versioned source permalink proves the exact `(none)` token this type mirrors, which the protocol does not define.

```faber
# The move an engine reports on its final `bestmove` line.
#
# `Play` carries a UCI long algebraic move such as `e2e4`. `NoLegalMove` is
# Stockfish's `bestmove (none)`, sent when the position has no legal move;
# the protocol text names `0000` only for a null move.
#
# https://backscattering.de/chess/uci/#engine-bestmove
# https://github.com/official-stockfish/Stockfish/blob/sf_17/src/uci.cpp#L407-L412
@ public
union BestMove {
    Play {
        string uci
    },
    NoLegalMove
}
```

## Comment format

### NEGATIVE — A blank line detaches the comment

**Decisive evidence:** The sentence is correct, but the blank line ends the comment run, so `faber doc` renders the constant with no documentation and readers cannot tell which unit the line describes.

```faber
# The deepest search the engine adapter requests, in plies.

@ public
const int MAX_SEARCH_DEPTH = 64
```

### POSITIVE — The comment run sits directly above the annotation

**Decisive evidence:** The `#` lines touch the declaration's first annotation, so `faber doc` attaches them:

```faber
# The deepest search the engine adapter requests, in plies.
@ public
const int MAX_SEARCH_DEPTH = 64
```

`faber doc` renders:

```text
@ public
const int MAX_SEARCH_DEPTH = 64
    The deepest search the engine adapter requests, in plies.
```
