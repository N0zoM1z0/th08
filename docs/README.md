# Documentation

Start with the project guide, a platform guide, or the agent session route.
All routes use the same evidence and acceptance rules. The
[homepage](../README.md#ai-agent-workflow) explains the AI agent workflow.

## Understand the project

1. Read the [project and engine guide](PROJECT_GUIDE.md) for the build products,
   runtime relationships, and a glossary.
2. Use [architecture and binary inventory](ARCHITECTURE.md) for the target
   identity, repository structure, and validation order.
3. Pick a subsystem in the [semantic index](SEMANTIC_INDEX.md) to reach its
   declarations and evidence. Use the [source map](SOURCE_MAP.md) when you need
   the production owner or comparison object.

These pages provide a short route into the project. The investigation archives
are available when you need to examine how a conclusion was reached.

## Play or build a product

| Goal | Start here |
| --- | --- |
| Download and play on Linux | [Linux player guide](PLAY_LINUX.md) |
| Build the Linux i386 executable from `main` | [Linux engineering guide](LINUX_PORTING.md#one-command-setup-and-play) |
| Build or investigate the native 64-bit port | [`port/portable-64bit` documentation](https://github.com/N0zoM1z0/th08/blob/port/portable-64bit/docs/PORTABLE_64BIT.md) |
| Understand Windows or macOS release readiness | [Windows guide](PLAY_WINDOWS.md), [macOS guide](PLAY_MACOS.md) |
| Play or develop the browser port | [TH08 Web project](https://github.com/N0zoM1z0/th08-web) |
| Compare the developer build products | [Port scope and products](PORTING.md#scope-and-products), [VC7 build modes](BUILD_MATCHING.md#builds) |

The Linux release packages and this branch's i386 source build have different
architecture requirements. Follow the guide for the product you selected.
[LINUX_PACKAGE.md](LINUX_PACKAGE.md) is the README shipped inside the i386
package; it assumes you are reading from that extracted archive.

## Develop and verify

For a first development session, follow the agent session route below as well.
The target, ABI, evidence, and validation contracts apply to every contributor.
The [conceptual guide](PROJECT_GUIDE.md) is available for orientation.

| Question | Canonical reference |
| --- | --- |
| How do I build and compare one function? | [Build and exact matching](BUILD_MATCHING.md#object-comparison) |
| What does an accepted result prove? | [Acceptance language](RE_WORKFLOW.md#acceptance-language), [acceptance rules](BUILD_MATCHING.md#acceptance-rules) |
| Which command answers my question? | [Tool recipes](TOOLS.md#choose-the-command-by-question) |
| How do names and types get accepted? | [Semantic reconstruction policy](SEMANTIC_RECONSTRUCTION.md) |
| How do I reproduce the native Windows prerequisite? | [Windows i386 runtime procedure](WINDOWS_I386_RUNTIME.md) |
| Where are reusable compiler and link lessons? | [Knowledge map](KNOWLEDGE_BASE.md#existing-subject-index) |

## Agent session route

1. Read [AGENTS.md](../AGENTS.md) for the target, ABI, single-writer, and
   checkpoint rules.
2. Read the [current handoff](RE_HANDOFF.md) and check live state with the
   [session-start commands](TOOLS.md#start-every-writable-session). The
   [target facts](ARCHITECTURE.md#exact-target) identify the only supported
   reconstruction executable.
3. Apply the [RE workflow](RE_WORKFLOW.md#bounded-reconstruction-loop), or the
   [semantic policy](SEMANTIC_RECONSTRUCTION.md#bounded-batch-workflow) for an
   already-authored field/protocol family.
4. Find the owner in [SOURCE_MAP.md](SOURCE_MAP.md#source-families), evidence in
   [SEMANTIC_INDEX.md](SEMANTIC_INDEX.md#subsystems), and the applicable
   [validation recipe](SOURCE_MAP.md#validation-recipes).

This route provides the operating instructions for a fresh engineering session.

## Authoritative state

| Information | Authoritative location |
| --- | --- |
| Current phase and immediate next work | [RE_HANDOFF.md](RE_HANDOFF.md) |
| Authored mappings and source selection | [mapping.csv](../config/mapping.csv), [implemented.csv](../config/implemented.csv) |
| Authored comparison definitions and accepted results | [match-units.toml](../config/match-units.toml), [matches.csv](../config/matches.csv) |
| Target-linked library provenance and results | [library-provenance.toml](../config/library-provenance.toml), [library-match-units.toml](../config/library-match-units.toml), [library-matches.csv](../config/library-matches.csv) |
| Generated progress views | [Authored progress](PROGRESS.md), [library progress](LIBRARY_PROGRESS.md) |

Use the [status reporter](TOOLS.md#choose-the-command-by-question) for live
ledger-derived figures. Source presence, a successful build, and an accepted
comparison describe different states. A historical result remains evidence
for its recorded scope and checkpoint.

## Reference catalog

| Subject | References |
| --- | --- |
| Runtime concepts and current owners | [Project guide](PROJECT_GUIDE.md), [semantic index](SEMANTIC_INDEX.md), [source map](SOURCE_MAP.md) |
| ANM resources and Effect ownership | [ANM namespaces](ANM_RESOURCE_INDEX.md), [Effect storage](EFFECT_STORAGE.md) |
| Reconstruction method and tool safety | [RE workflow](RE_WORKFLOW.md), [semantic policy](SEMANTIC_RECONSTRUCTION.md), [tools](TOOLS.md), [IDA database attestation](IDA_MCP.md) |
| Compiler and whole-image reference material | [VC7 patterns](VC7_ZUN_PATTERNS.md), [build/matching corpus](BUILD_MATCHING.md#compiler-pattern-corpus), [whole-image lessons](WHOLE_IMAGE_RECONSTRUCTION.md) |
| Focused matching notes | [Player](PLAYER_MATCHING.md), [GameManager](GAME_MANAGER_MATCHING.md), [stage menus](STAGE_MENU_MATCHING.md) |
| Native runtime evidence | [Windows i386 procedure](WINDOWS_I386_RUNTIME.md), [owner audit](OWNER_AUDIT.md), [runtime issues](RUNTIME_ISSUES.md) |
| Port engineering | [Port scope](PORTING.md), [Linux i386 engineering](LINUX_PORTING.md) |
| Methods reusable in another title | [Semantic and readability playbook](SEMANTIC_PLAYBOOK.md) |

## Investigation archives

- [Semantic history](SEMANTIC_HISTORY.md#completed-batches) records completed
  batches. The [semantic index](SEMANTIC_INDEX.md) links directly to relevant
  records.
- [RunEcl notes](RUNECL_FUNCTION_EXACT_NOTES.md#formal-relocation-manifest)
  preserve the full investigation; that link opens the formal final result.
- [Handoff history](RE_HANDOFF_HISTORY.md) preserves earlier project states;
  [RE_HANDOFF.md](RE_HANDOFF.md) is the current entry point.
- [Whole-image case studies](WHOLE_IMAGE_RECONSTRUCTION.md#translation-unit-layout)
  preserve measured experiments and their generalization limits.

Use the [knowledge map](KNOWLEDGE_BASE.md#promote-knowledge-instead-of-accumulating-scratch)
when recording a new lesson or choosing where to keep its evidence.
