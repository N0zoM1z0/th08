# Native macOS guide

## Status: in progress

There is no macOS executable or release package yet. The current playable
backend is Linux-specific, and the repository does not claim that the Linux ELF
or Windows bring-up executable runs natively on macOS.

The planned native port will reuse the reconstructed authored game sources
behind a macOS window, input, audio, renderer, and packaging boundary. The
player-facing installation and data-directory commands will be documented here
after that backend exists and has been tested on real macOS hardware.

Follow the platform sequence and shared portability constraints in [Playable
reconstruction ports](PORTING.md). The expected data contract remains a legally
obtained original Japanese TH08 1.00d directory containing `th08.dat` and
`thbgm.dat`; no copyrighted game data will be bundled with a future package.
