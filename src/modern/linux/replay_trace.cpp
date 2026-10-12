// Optional test recorder. State reads preserve the retail observer boundary.
#include "replay_trace.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"
#include "Gui.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace th08
{
namespace modern
{
namespace
{
const int FIELD_COUNT = 25;
struct Trace
{
    bool initialized;
    bool finished;
    FILE *reference;
    FILE *rows;
    const char *completionPath;
    u32 count;
    u32 maximum;
    u32 origins[MAX_STAGES];
    u32 frames[MAX_STAGES];
    u32 scores[MAX_STAGES];
    int stages[MAX_STAGES];
    int stageCount;
};
Trace trace;

u32 FloatBits(const f32 &value)
{
    u32 bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

void Finish(const char *error, bool diagnostic, bool natural)
{
    if (trace.finished || !trace.reference)
        return;
    trace.finished = true;
    if (!error && !diagnostic)
    {
        unsigned char extra;
        if (fread(&extra, 1, 1, trace.reference) != 0)
            error = "Portable playback ended before the retail trace";
    }
    fclose(trace.reference);
    fclose(trace.rows);
    FILE *completion = fopen(trace.completionPath, "w");
    if (!completion)
    {
        fprintf(stderr, "Cannot write portable replay completion metadata\n");
        exit(1);
    }
    fprintf(completion, "{\"rows\":%u,\"ended\":%s,\"diagnosticComplete\":%s,\"naturalReplayExit\":%s,\"errors\":[",
            trace.count, !error && natural ? "true" : "false", diagnostic ? "true" : "false", natural ? "true" : "false");
    if (error)
        fprintf(completion, "\"%s\"", error);
    fprintf(completion, "],\"stages\":[");
    for (int i = 0; i < trace.stageCount; ++i)
        fprintf(completion, "%s%d", i ? "," : "", trace.stages[i]);
    const char *names[] = {"stageFrames", "endScores", "rngOrigins"};
    u32 *values[] = {trace.frames, trace.scores, trace.origins};
    for (int group = 0; group < 3; ++group)
    {
        fprintf(completion, "%s\"%s\":{", group ? "," : "],", names[group]);
        for (int i = 0; i < trace.stageCount; ++i)
        {
            int stage = trace.stages[i];
            fprintf(completion, "%s\"%d\":%u", i ? "," : "", stage, values[group][stage]);
        }
        fprintf(completion, "}");
    }
    fprintf(completion, "}\n");
    fclose(completion);
    fprintf(stderr, "Portable replay stopped after %u calculations: %s\n", trace.count,
            error ? error : diagnostic ? "diagnostic prefix" : "natural replay exit");
}

void Initialize()
{
    if (trace.initialized)
        return;
    trace.initialized = true;
    const char *reference = getenv("TH08_REPLAY_TRACE_REFERENCE");
    if (!reference || !reference[0])
        return;
    const char *rows = getenv("TH08_REPLAY_TRACE_ROWS");
    trace.completionPath = getenv("TH08_REPLAY_TRACE_COMPLETION");
    if (!rows || !trace.completionPath)
    {
        fprintf(stderr, "Replay trace output paths are missing\n");
        exit(1);
    }
    trace.reference = fopen(reference, "rb");
    trace.rows = fopen(rows, "wb");
    if (!trace.reference || !trace.rows)
    {
        fprintf(stderr, "Cannot open portable replay trace files\n");
        exit(1);
    }
    const char *maximum = getenv("TH08_REPLAY_TRACE_MAXIMUM");
    trace.maximum = maximum ? static_cast<u32>(strtoul(maximum, NULL, 10)) : 0;
    // Equivalent to choosing the public fast-forward option in the Replay menu.
    g_GameManager.replayMode = 2;
}
}

void ReplayTraceBeginCalculation()
{
    Initialize();
    if (trace.reference && !trace.finished && g_ReplayManager && g_ReplayManager->frameCounter == 0)
    {
        int stage = g_GameManager.currentStage;
        if (stage >= 0 && stage < MAX_STAGES)
            trace.origins[stage] = g_Rng.ReplayTraceGenerationCount();
    }
}

bool ReplayTraceRecord(ReplayManager *manager)
{
    if (!trace.reference || trace.finished || !g_GameManager.flags.isReplay || g_GameManager.flags.isDemoMode)
        return false;
    int stage = g_GameManager.currentStage;
    if (stage < 0 || stage >= MAX_STAGES || !g_GameManager.globals)
    {
        Finish("Invalid replay stage or globals", false, false);
        return true;
    }
    if (!trace.stageCount || trace.stages[trace.stageCount - 1] != stage)
    {
        if (trace.stageCount == MAX_STAGES || trace.frames[stage])
        {
            Finish("Replay revisited a captured stage", false, false);
            return true;
        }
        trace.stages[trace.stageCount++] = stage;
    }
    const ZunGlobals &globals = *g_GameManager.globals;
    u32 row[FIELD_COUNT] = {
        trace.count + 1, static_cast<u32>(stage), static_cast<u32>(manager->frameCounter),
        g_GameManager.gameplayFrameCounter, globals.score, static_cast<u32>(globals.graze),
        FloatBits(globals.deaths), FloatBits(globals.livesRemaining), FloatBits(globals.bombsRemaining), FloatBits(globals.playerPower),
        FloatBits(g_Player.position.x), FloatBits(g_Player.position.y), FloatBits(g_Player.position.z),
        static_cast<u8>(g_Player.playerState), g_Rng.GetSeed(), g_Rng.ReplayTraceGenerationCount() - trace.origins[stage],
        g_GuiMessageInputCurrent, static_cast<u32>(globals.pointItemsCollected), static_cast<u32>(globals.currentTimeOrbs),
        static_cast<u32>(globals.youkaiGauge), static_cast<u8>(globals.clockTime), static_cast<u32>(globals.pointItemValue),
        g_GameManager.character, g_GameManager.shotType, static_cast<u32>(g_GameManager.difficulty)
    };
    if (fwrite(row, sizeof(u32), FIELD_COUNT, trace.rows) != FIELD_COUNT)
    {
        Finish("Cannot write portable replay rows", false, false);
        return true;
    }
    ++trace.count;
    trace.frames[stage] = row[2];
    trace.scores[stage] = row[4];
    u32 expected[FIELD_COUNT];
    if (fread(expected, sizeof(u32), FIELD_COUNT, trace.reference) != FIELD_COUNT || memcmp(row, expected, sizeof(row)))
    {
        Finish("Replay diverged; inspect first-difference.json", false, false);
        return true;
    }
    if (trace.maximum && trace.count >= trace.maximum)
    {
        Finish(NULL, true, false);
        return true;
    }
    if (trace.count % 10000 == 0)
        fprintf(stderr, "Portable replay: %u equal calculations, stage index %d, frame %d\n", trace.count, stage, manager->frameCounter);
    return false;
}

void ReplayTraceFinish()
{
    if (trace.reference && !trace.finished && trace.count)
    {
        Finish(NULL, false, true);
    }
}

bool ReplayTraceShouldExit()
{
    return trace.reference && trace.finished;
}
}
}
