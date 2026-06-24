# Chess Engine — Design Notes

## Board Representation
- Bitboard with LERF-mapping

## Move Generation
- Lookup attack tables via precomputation and magic bitboards

## Known limitations / future work

### MILESTONE 1:
- [x] Create board struct with 6 arrays of bitboards
- [x] Create simple bit helper functions (set, clear, get)
- [x] Create print board helper function
- [x] Parse FEN to corresponding board struct
- [x] Encode board struct to corresponding FEN
- [x] Build test harness for FEN and board representation

### MILESTONE 2:
- [x] Create move representation
- [x] Create move helper getter/setter functions (from/to)
- [ ] Create psuedo-legal move generation
- [x] Create make_move and unmake_move functions
- [ ] Build Perft test harness for move generation
