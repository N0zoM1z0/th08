# TH08 1.00d replay corpus

This directory preserves the public recordings used by the native replay gate.
Each file retains its original bytes and archive filename. The collection is
3,605,499 bytes (about 3.4 MiB), added to this repository on 2026-10-11.

| Directory | Files | Purpose |
| --- | ---: | --- |
| [accepted/](accepted/) | 108 | All 12 shot types, four main difficulties through Final A and Final B, plus Extra |
| [retail-incomplete/](retail-incomplete/) | 3 | Reproduce incomplete playback observed in the original game under Wine |

The accepted recordings cover each recorded stage along its route, including
both Stage 4 branches. The diagnostic recordings have separate failure
observations and are excluded from passing results. See the
[replay guide](../../../../docs/REPLAY_TESTING.md) for the comparison procedure,
published trace expectations, and diagnostic commands.

## Credits and provenance

Recordings come from the [Royalflare Archive at Maribel Hearn](https://maribelhearn.com/royalflare/th08).
[CREDITS.md](CREDITS.md) lists every recording's author, play date, upload date
when available, and original download link. Author names preserve the archive's
credit, including capitalization and spelling.

The [accepted manifest](../../../../config/replay-fixtures.json) and
[diagnostic manifest](../../../../config/replay-rejected-fixtures.json) retain
SHA-256 hashes, file sizes, source URLs, and decoded replay metadata. `recordedBy`
is the archive's author credit; `player` is the name stored in the replay header,
which can be a placeholder. `playedAt` is the archive's Play Date;
`submittedAt` records the upload date when known. The manifests preserve the
archive's date strings without assigning a timezone.

Recording rights remain with their authors. The repository's MIT license
covers its source and documentation; it does not assign that license to these
third-party recordings. TH08 belongs to Team Shanghai Alice. Obtain the original
Japanese 1.00d executable and game data separately for runtime testing.

## Use and maintain

The suite uses these committed files without downloading them. It validates
their hashes and decoded metadata before playback. A file already present in
the writable `build/replay-fixtures/` cache is validated first; additional
fixtures can be downloaded into that cache from their pinned manifest URLs.
Generated traces, logs, and runtime directories stay under `build/`.

Validate all recordings and the attribution index without Wine or game data:

```bash
python3 scripts/validate-replay-fixtures.py
```

For a corpus update, preserve the original file, add its provenance to the
appropriate manifest, and regenerate the credit index:

```bash
python3 scripts/validate-replay-fixtures.py --write-credits
python3 scripts/ci.py
```

The three bundled title-screen demos are read from the user's game archive.
Their comparison is documented in the replay guide.
