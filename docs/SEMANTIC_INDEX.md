# Current semantic index

Choose a subsystem to reach its current declaration, production owner, and
supporting evidence. For an explanation of how the systems fit together, read
the [project guide](PROJECT_GUIDE.md). For compiler/object ownership, use the
[source map](SOURCE_MAP.md#source-families).

The linked semantic batches are dated evidence records. Their measurements
and intermediate names describe that checkpoint; current declarations live in
the linked source, and accepted state lives in the ledgers.

## Subsystems

| Subsystem | Declaration / production owner | Evidence | Key distinction |
| --- | --- | --- | --- |
| ANM interpreter and rendering | [AnmManager.hpp](../src/AnmManager.hpp), [AnmManager.cpp](../src/AnmManager.cpp) | [Resource namespaces](ANM_RESOURCE_INDEX.md); [projection and draw protocol](SEMANTIC_HISTORY.md#anm-projection-draw-and-texture-strip-protocol--2026-08-27) | Manager slots, file-local script IDs, sprite IDs, and VM array indices are different namespaces. |
| ECL interpreter | [EclManager.hpp](../src/EclManager.hpp), [EclRun.cpp](../src/EclRun.cpp), [low](../src/EclRunLow.inl) / [high](../src/EclRunHigh.inl) handlers | [Formal RunEcl result](RUNECL_FUNCTION_EXACT_NOTES.md#formal-relocation-manifest); [include protocol](SOURCE_MAP.md#reading-eclrun) | Included handler bodies share an outer lexical frame and labels. |
| ECL operands/dependencies | [EclOperands.hpp](../src/EclOperands.hpp), [integer](../src/EclOperandsInt.cpp) / [float](../src/EclOperandsFloat.cpp) resolvers, [dependencies](../src/EclDependencies.cpp), [helpers](../src/EclHelpers.cpp), [EX handlers](../src/EclExIns.cpp) | [Interpreter contexts](SEMANTIC_HISTORY.md#enemy-ecl-interpreter-and-subroutine-state--2026-08-26); [Enemy-owned execution](SEMANTIC_HISTORY.md#enemy-owned-ecl-execution-and-typed-ex-dispatch--2026-08-27) | Physical TU ownership follows target emission, not the class named by a function. |
| Enemy and timeline | [EnemyManager.hpp](../src/EnemyManager.hpp), [manager](../src/EnemyManager.cpp), [update](../src/EnemyManagerUpdate.cpp), [timeline](../src/EnemyTimeline.cpp) | [Motion controller](SEMANTIC_HISTORY.md#enemy-motion-controller--2026-08-26); [pool and orchestration](SEMANTIC_HISTORY.md#enemymanager-pool-and-orchestration-state--2026-08-26) | TH06/TH07 names remain corroboration until TH08 dataflow confirms them. |
| Effect pool and callbacks | [EclManager.hpp](../src/EclManager.hpp), [EffectManager.cpp](../src/EffectManager.cpp) | [Storage contract](EFFECT_STORAGE.md); [factory and draw layers](SEMANTIC_HISTORY.md#effect-factory-draw-layers-and-radial-trail-protocol--2026-08-27) | `vector1`…`vector7` have roles local to each callback family. |
| Replay | [ReplayManager.hpp](../src/ReplayManager.hpp), [ReplayManager.cpp](../src/ReplayManager.cpp) | [Runtime protocol](SEMANTIC_HISTORY.md#replay-runtime-protocol-and-residual-absolute-owner-closure--2026-08-27); [stream ownership](SEMANTIC_HISTORY.md#replay-serialization-and-stream-ownership--2026-08-27) | `LoadReplayData` consumes its encoded input and returns a separately owned allocation. |
| Screen effects | [ScreenEffect.hpp](../src/ScreenEffect.hpp), [ScreenEffect.cpp](../src/ScreenEffect.cpp) | [Variant parameters](SEMANTIC_HISTORY.md#screeneffect-variant-parameters--2026-08-26); [lifecycle](SEMANTIC_HISTORY.md#screeneffect-lifecycle-and-visual-modes--2026-08-27) | `RegisterChain` parameters are a tagged protocol; `duration` stores amplitude for shake envelope. |
| Player, shots, and bombs | [Player.hpp](../src/Player.hpp), [Player.cpp](../src/Player.cpp), [PlayerBomb.cpp](../src/PlayerBomb.cpp) | [Matching notes](PLAYER_MATCHING.md); [movement and shooting](SEMANTIC_HISTORY.md#player-movement-collision-options-and-shooting--2026-08-26); [bomb work items](SEMANTIC_HISTORY.md#playerbomb-callback-and-work-item-protocol--2026-08-26) | Exact-only option bodies also exist in [PlayerOptionProbe.cpp](../src/PlayerOptionProbe.cpp). |
| Bullets and lasers | [BulletManager.hpp](../src/BulletManager.hpp), [BulletManager.cpp](../src/BulletManager.cpp) | [Bullet lifecycle](SEMANTIC_HISTORY.md#bullet-core-lifecycle-and-ecl-controls--2026-08-26); [Laser lifecycle](SEMANTIC_HISTORY.md#bulletmanager-laser-lifecycle--2026-08-26) | Serialized instruction shapes and runtime pool objects must remain distinct. |
| GUI and dialogue | [Gui.hpp](../src/Gui.hpp), [Gui.cpp](../src/Gui.cpp), [ASCII declarations](../src/AsciiManager.hpp) | [Message and HUD model](SEMANTIC_HISTORY.md#gui-message-boss-hud-and-stage-clear-model--2026-08-27); [runtime issues](RUNTIME_ISSUES.md) | ANM resource owner and visual caller are not necessarily the same object. |
| Background and camera | [Background.hpp](../src/Background.hpp), [Background.cpp](../src/Background.cpp) | [Camera and stage model](SEMANTIC_HISTORY.md#background-camera-stage-and-spell-background-model--2026-08-26) | Camera mode flags are behaviorally observed; adjacent-version labels are secondary evidence. |
| Game state and score | [GameManager.hpp](../src/GameManager.hpp), [GameManager.cpp](../src/GameManager.cpp) | [Runtime state](SEMANTIC_HISTORY.md#gamemanager-core-runtime-and-setup-state--2026-08-26); [matching notes](GAME_MANAGER_MATCHING.md); [owner audit](OWNER_AUDIT.md) | Setup/score exact probes duplicate production bodies; canonical storage is the aggregate manager. |
| Title, result, and persistence | [TitleScreen](../src/TitleScreen.hpp), [ResultScreen](../src/ResultScreen.hpp), [ScoreDat](../src/ScoreDat.hpp) | [Build ownership](SOURCE_MAP.md#source-families); [result and persistence](SEMANTIC_HISTORY.md#result-and-score-persistence-tails--2026-08-27) | Title/replay probes are exact-only and shared `.inl` fragments have explicit ownership. |
| Audio and MIDI | [SoundPlayer](../src/SoundPlayer.hpp), [Midi](../src/Midi.hpp), [Supervisor](../src/Supervisor.hpp) | [MIDI and streaming audio](SEMANTIC_HISTORY.md#midi-timeline-and-streaming-audio-protocol--2026-08-27) | `src/modern/` implements platform adaptation; original semantics are established from target evidence. |
| Callback scheduler | [Global.hpp](../src/Global.hpp), [Global.cpp](../src/Global.cpp) | [Frame walkthrough](PROJECT_GUIDE.md#a-frame-through-the-engine); [priority protocol](SEMANTIC_HISTORY.md#calcdraw-scheduler-priority-protocol--2026-08-27) | Calculation and drawing have separate priorities and active callback lists. |

## Symbol lookup

For a symbol-level question, search the target address through `src`, `config`,
and `docs`. The address disambiguates provisional or overloaded names:

```bash
rg -n "0x0045EA00|ExecuteScript" src config docs
```

For acceptance language and the required exact/portable oracle pair, use
[SEMANTIC_RECONSTRUCTION.md](SEMANTIC_RECONSTRUCTION.md#two-oracle-acceptance)
and [RE_WORKFLOW.md](RE_WORKFLOW.md#acceptance-language). Return to the
[documentation index](README.md) for the other reading routes.
