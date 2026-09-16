# Larpfish
![Larpfish Logo](./Larpfish.png)

[Play against Larpfish on Lichess!](https://lichess.org/@/Larpfish/rated)

Written by Tyler Chew.

## Overview
Larpfish is a UCI-compatible chess engine, written from scratch in C. Hosted online via [lichess-bot](https://github.com/lichess-bot-devs/lichess-bot): reaching 2100+ ELO after 2.2K online games as of writing.

Larpfish is currently run on an AMD Ryzen 5 2500U CPU.

## Building
This project uses GCC built-in intrinsics for bit counting/scanning, and thus requires a compiler that supports them. 

Build with gcc:
```
make
```

This generates the `Larpfish` executable, along with other testing/generating tools (found under the `tools/` directory).

Run the chess engine under a UCI-supported chess GUI, or directly via:
```
./Larpfish
```

## Technical Features

- **Board Representation**
    - A hybrid bitboard/8x8 mailbox board representation
- **Move Generation**
    - From-to 16-bit move encoding
    - Pseudo-legal move generation with precomputed attack tables and fancy magic bitboards
- **Evaluation**
    - Handcrafted evaluation function taken from [Tomasz Michniewski's Simplified Evaluation Function](https://chessprogramming.org/Simplified_Evaluation_Function)
    - Material counting
    - Piece-square tables (PSTs)
    - Tapered evaluation for game phase detection in king PSTs
- **Search**
    - Negamax search with alpha-beta
    - Iterative deepening (ID)
    - Quiescence search (Q-search)
    - Transposition table (TT)
    - Principal Variation Search (PVS)
    - Reverse Futility Pruning (RFP)
    - Null Move Pruning (NMP)
    - Late Move Reductions (LMR)
    - Improving Heuristic
- **Move Ordering**
    - PV move
    - MVV-LVA captures
    - Killer moves
    - History heuristic
- **UCI Compatibility**
    - Basic UCI support with the following commands: `uci`, `isready`, `setoption name Hash value (1|N)`, `ucinewgame`, `position`, `go`, `stop`, `quit`
    - Opening/endgame tablebases handled via lichess-bot
    - Basic time management heuristic used: `base / 20 + increment / 2` time per move

Perft testing is implemented to validate move generation, and SPRT is run via [fastchess](https://github.com/Disservin/fastchess/tree/master) to test ELO strength gains.

## Next Steps/Known Limitations
- NNUE evaluation
- Further search pruning extensions
- Cross-compiler compatibility
