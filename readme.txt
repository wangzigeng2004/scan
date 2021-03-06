
A modified version of Scan 3.0 for lidraughts.org, by Stefan Schermann

Added handicap setting to the hub-command "level", taking a value of 0-3 (e.g. "level handicap=3"), which progressively disables the following quiescence search features:
0 = Full qs, no handicap
1 = No qs in quiet positions (threats/sacs)
2 = No qs for captures
3 = No bitbases

Added prepocessor directives that allow compilation to a PNaCl pexe, and for WASM / asm.js through Emscripten. Thanks again to Niklas Fiekas, for the inspiration taken from the equivalent Stockfish projects:
https://github.com/niklasf/stockfish.pexe
https://github.com/niklasf/stockfish.js

PNaCl:
-Multithreaded so uses the original hub_loop, stdin is replaced with calls to scan_command() from javascript
-A virtual filsesystem with scan.ini and eval is loaded over http
-Can be compiled with the Makefile in src/pnacl

Emscripten:
-Only single thread is supported, the application exits main() after initialization and maintains global/static state
-Each call to scan_command() from javascript enters hub_loop() and exits again immediatly afterwards, except when the "go" command launches search
-Search itself is chopped up as a series of emscripten_async_calls() to remain responsive to commands (e.g. "stop")
-A virtual filesystem containing scan.ini and eval is included at compile time, assumed to be in the "scan-assets" directory
-Necessary compilation flags for both targets, to setup the filesystem, exception handling, etc:
em++ bb_base.cpp bb_comp.cpp bb_index.cpp bit.cpp book.cpp common.cpp dxp.cpp eval.cpp fen.cpp game.cpp hash.cpp hub.cpp libmy.cpp list.cpp main.cpp move.cpp move_gen.cpp pos.cpp score.cpp search.cpp socket.cpp sort.cpp thread.cpp tt.cpp util.cpp var.cpp -ansi -pedantic -Wall -Wextra -std=c++11 -fno-rtti -O2 -flto -s TOTAL_MEMORY=1024MB --preload-file scan-assets@/ -s NO_EXIT_RUNTIME=1 --memory-init-file 0 -s EXPORTED_FUNCTIONS="['_main', '_scan_command']" -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']" -s DISABLE_EXCEPTION_CATCHING=0
-To target WASM: -s WASM=1 -o scan.html -s "BINARYEN_TRAP_MODE='allow'" -s "BINARYEN_METHOD='native-wasm'" -s BINARYEN_ASYNC_COMPILATION=1
-To target asm.js: -s WASM=0 -o scan.js
-It seems that --preload-file won't work in combination with --pre-js (or --post-js), therefore src/pre.js must be copy-pasted to the beginning of the resulting scan.js file (for both targets)


Planned changes: multipv support for analysis


The original readme.txt is included below, sources are distributed under the same license

---

Scan 3.0 Copyright (C) 2015-2017 Fabien Letouzey.
This program is distributed under the GNU General Public License version 3.
See license.txt for more details.

---

Today is 2017-07-11.
Scan is an international (10x10) draughts engine that uses the DamExchange Protocol (DXP) or text mode.  The name "Scan" comes from the scanning in evaluation that "divides" the board into 8 overlapping rectangles (2-26, 3-27, ..., 25-49) to judge positions.  Enjoy Scan!

Thanks a lot to Harm Jetten for helping with Windows compatibility and compilation, testing, hosting, etc (you name it, he did it) ...  His engine, Moby Dam, is also cross-platform and open-source!

Greetings to other game programmers; Gens una sumus.

Fabien Letouzey (fabien_letouzey@hotmail.com).

---

Running Scan

In Windows terminology, Scan is a "console application" (no graphics).  Text mode is the default; a DXP mode is also available with a command-line argument: "scan dxp".  Scan needs the configuration file "scan.ini" (described below) and data files in the "data" directory (opening book, evaluation weights, and bitbases).  Note that, due to their size,  bitbases require a separate copy (from Scan 2.0) or download for installation.

Most text-mode commands consist of a single letter (lower case):

0-2    -> number of computer players (e.g. 2 = auto-play)
(g)o   -> make the computer play your side
(u)ndo -> take back one ply
(r)edo -> replay a previous take-back, if no other move was played

time <n> -> fixed time limit; 10s by default

(h)elp -> find a few other commands

And of course you can type a move in standard notation.  Just pressing return can be used for forced moves.

A note about scores.  +/- 89.xx means reaching a winning/losing endgame soon.  +/- 99.xx means reaching the absolute end of the game soon.  Those values differ from Scan 2.0

Scan also has a Hub mode with a new protocol: "scan hub", which is used by the Hub GUI (separate download).  Programmers can use it to control Scan in an automated way; the description of the protocol can be found in "protocol.txt".

---

Configuration

You can edit the text file "scan.ini" to change settings; you need to re-launch Scan in that case.  Here are the parameters.

variant (NEW): selects the rules to apply.  "normal" for international draughts.  However a lot of draws occur with those rules, even with somewhat weaker opponents.  "killer" (Killer draughts) and "bt" (breakthrough draughts: the first player who makes a king wins) are attempts to make the game more interesting at high level.  Scan should be very strong in Killer draughts and the "normal" rules are actually only supported as a legacy feature (sorry for the fans).  By contrast, BT support is experimental and not well tested.  IMPORTANT: changing the rules only makes sense if both players are aware of it (just like chess vs. draughts).

book, book-ply (NEW), book-margin: you can (de)activate the opening book here.  Randomness will only be applied to the first "book-ply" plies (half moves); subsequent moves will always be the best ones.  I used "book-ply = 4" during the Computer Olympiads, and it also covers Scan 2.0's behaviour.  "book-margin" acts as a randomness factor, for example: 0 = best move (for tournaments with pre-selected opening positions), 1 = small randomness (for serious games), 4 = fairly random (for casual games).  Note that equally-good moves are always picked at random, even after the first "book-ply" moves.

threads: how many cores to use for search (SMP).  Avoid hyper-threading (not tested).

tt-size: the number of entries in the transposition table will be 2 ^ tt-size.  Every entry takes 16 bytes so tt-size = 26 corresponds to 1 GiB; that's what I used during the Computer Olympiad.  Use smaller values for fast games.  Every time you increase it by one, the size of the table will double.

bb-size: use endgame bitbases (win/loss/draw only) of up to "bb-size" pieces (0 = no bitbases).  If you want maximum strength, use 6 (7 for BT variant).  This will take about 2 GiB of RAM though.  If Scan takes too much time to initialise or too much memory, select 5.  Note that bitbases require a separate copy (from Scan 2.0) or download for installation into the "data" directory.

The other options are all related to the DamExchange Protocol (DXP), and are the same as in Scan 2.0

dxp-server: for two programs to communicate, one has to be the server and the other one the client ("caller" to use a phone analogy).

dxp-host & dxp-port: dxp-host is the IP address (in numerical form such as 127.0.0.1) of the server to connect to (in client mode).  It has no effect in server mode.  dxp-port affects both modes.

dxp-initiator: in addition to client/server, one program has to start the games (initiator) and the other only answers requests (follower).  Scan's initiator mode is very basic.  It will launch an infinite match from the starting position, switching sides after each game.  Presumably other programs have a more advanced initiator mode and you should use that when possible.

dxp-time & dxp-moves: time control (only for the initiator).  Time is in minutes.  0 moves indicate no move limit: the game will be played to the bitter end (not recommended).

dxp-board & dxp-search: whether Scan should display the board and/or search information after each move.  Setting both to true, you can follow the games in text mode.  With both set to false, Scan is more silent.

---

Compilation

The source code uses C++11 and should be mostly cross-platform.  I provided the Clang Makefile I use on Mac; it is compatible with GCC with the possible exception of -flto (link-time optimisation).  The source code is also known to work with Visual Studio.

---

History

2015-04-10, version 1.0 (private release)

2015-07-19, version 2.0
- added opening book
- added endgame tables (6 pieces)
- added LMR (more pruning)
- added parallel search
- added game phase in evaluation
- added bitboard move generation
- added DXP

2017-07-11, version 3.0
- added Killer and BT variants
- improved evaluation
- improved QS (opponent-can-capture positions)
- improved speed
- improved bitbase probing (keep searching for an exact win after a BB win)
- improved Hub protocol (see protocol.txt)
- cleaned up code (stricter types and immutable position classes)

