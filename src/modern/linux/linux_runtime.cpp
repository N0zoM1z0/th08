#include "modern/windows_runtime.hpp"

#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPTSTR, int);

namespace th08
{
struct AnmVm;
struct Effect;

int __fastcall EffectRandomSplashInit(AnmVm *);
int __fastcall EffectRandomSplashUpdate(AnmVm *);
int __fastcall EffectRandomSplashBigInit(AnmVm *);
int __fastcall EffectOrbitInit(AnmVm *);
int __fastcall EffectOrbitUpdate(AnmVm *);

int __fastcall UpdateExpandingWavyRadialTrail(AnmVm *);
int __fastcall UpdateExpandingPositiveDiagonalRadialTrail(AnmVm *);
int __fastcall UpdateExpandingNegativeDiagonalRadialTrail(AnmVm *);
int __fastcall UpdateExpandingOctagonalRadialTrail(AnmVm *);
int __fastcall UpdateExpandingTwelveSegmentRadialTrail(AnmVm *);
int __fastcall UpdateBarrierRadialEffect(AnmVm *);
int __fastcall InitializeBarrierRadialEffect(AnmVm *);
int __fastcall InitializeRotatingBarrierRadialEffect(AnmVm *);
int __fastcall UpdateExpandingOrthogonalRadialTrail(AnmVm *);

int __fastcall FUN_00426280(Effect *);
int __fastcall FUN_004264f0(Effect *);
int __fastcall FUN_00426720(Effect *);
int __fastcall FUN_00426990(Effect *);
int __fastcall InitializeRandomDirectionalOffset(Effect *);
int __fastcall UpdateDirectionalOffset60(Effect *);
int __fastcall TrackPlayerUntilAnimationEnds(Effect *);
int __fastcall UpdateDirectionalOffset240(Effect *);
int __fastcall FUN_00426d70(Effect *);
int __fastcall FUN_00426e70(Effect *);
int __fastcall InitializeDirectionalOffset(Effect *);
int __fastcall UpdateEasedDirectionalOffset(Effect *);
int __fastcall KeepTrailAlive(Effect *);
int __fastcall InitializeTrailOffset(Effect *);
int __fastcall InitializeRadialTrail(Effect *);
int __fastcall InitializeAlternateLayerRadialTrail(Effect *);
int __fastcall SyncRadialTrailRadius(Effect *);
int __fastcall SyncRadialTrailShape(Effect *);
int __fastcall UpdateTimedRadialTrail(Effect *);
int __fastcall UpdateFadingRadialTrail(Effect *);
int __fastcall SyncAnchoredRadialTrail(Effect *);

// This retail table entry points at an AnmVm member. On the 32-bit Linux ABI
// its code entry receives `this` as the first stack argument, matching the
// reconstructed effect callback invocation.
extern "C" int UpdatePulsingRadialTrailCallback(AnmVm *) asm("_ZN4th085AnmVm24UpdatePulsingRadialTrailEv");

namespace modern
{
namespace
{
int g_argumentCount;
char **g_arguments;
volatile sig_atomic_t g_reportingCrash;

void WriteCrashLine(int file, const char *line)
{
    if (line != NULL) write(file, line, strlen(line));
}

void ReportFatalSignal(int signalNumber, siginfo_t *signalInfo, void *)
{
    if (g_reportingCrash)
        _exit(128 + signalNumber);
    g_reportingCrash = 1;

    int file = open("modern-crash.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file >= 0)
    {
        char line[160];
        snprintf(line, sizeof(line), "signal=%d fault-address=%p pid=%ld\n", signalNumber,
                 signalInfo != NULL ? signalInfo->si_addr : NULL, static_cast<long>(getpid()));
        WriteCrashLine(file, line);

        void *frames[64];
        int frameCount = backtrace(frames, sizeof(frames) / sizeof(frames[0]));
        backtrace_symbols_fd(frames, frameCount, file);
        fsync(file);
        close(file);
    }

    signal(signalNumber, SIG_DFL);
    raise(signalNumber);
    _exit(128 + signalNumber);
}

void InstallSignalHandler(int signalNumber)
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = ReportFatalSignal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO | SA_RESETHAND;
    sigaction(signalNumber, &action, NULL);
}

struct ModernEffectTemplate
{
    int32_t scriptIdx;
    uintptr_t update;
    uintptr_t initialize;
};

uintptr_t CodeAddress(int (__fastcall *callback)(AnmVm *))
{
    return reinterpret_cast<uintptr_t>(callback);
}

uintptr_t CodeAddress(int (__fastcall *callback)(Effect *))
{
    return reinterpret_cast<uintptr_t>(callback);
}

void InitializeTargetData()
{
    static const ModernEffectTemplate effectTemplates[66] = {
        {28, 0, 0}, {29, 0, 0}, {30, 0, 0},
        {31, CodeAddress(EffectRandomSplashUpdate), CodeAddress(EffectRandomSplashBigInit)},
        {36, CodeAddress(EffectRandomSplashUpdate), CodeAddress(EffectRandomSplashInit)},
        {37, CodeAddress(EffectRandomSplashUpdate), CodeAddress(EffectRandomSplashInit)},
        {38, CodeAddress(EffectRandomSplashUpdate), CodeAddress(EffectRandomSplashInit)},
        {39, CodeAddress(EffectRandomSplashUpdate), CodeAddress(EffectRandomSplashInit)},
        {40, CodeAddress(EffectRandomSplashUpdate), CodeAddress(EffectRandomSplashInit)},
        {41, CodeAddress(EffectRandomSplashUpdate), CodeAddress(EffectRandomSplashInit)},
        {42, CodeAddress(EffectRandomSplashUpdate), CodeAddress(EffectRandomSplashInit)},
        {43, CodeAddress(EffectRandomSplashUpdate), CodeAddress(EffectRandomSplashInit)},
        {44, 0, 0},
        {45, CodeAddress(EffectOrbitUpdate), CodeAddress(EffectOrbitInit)},
        {45, CodeAddress(EffectOrbitUpdate), CodeAddress(EffectOrbitInit)},
        {45, CodeAddress(EffectOrbitUpdate), CodeAddress(EffectOrbitInit)},
        {0, 0, 0},
        {32, CodeAddress(UpdateDirectionalOffset60), CodeAddress(InitializeRandomDirectionalOffset)},
        {33, CodeAddress(UpdateDirectionalOffset240), CodeAddress(InitializeRandomDirectionalOffset)},
        {51, CodeAddress(FUN_00426d70), CodeAddress(FUN_00426e70)},
        {56, 0, 0},
        {52, CodeAddress(UpdateEasedDirectionalOffset), CodeAddress(InitializeDirectionalOffset)},
        {54, CodeAddress(TrackPlayerUntilAnimationEnds), 0},
        {104, CodeAddress(KeepTrailAlive), 0},
        {104, CodeAddress(KeepTrailAlive), 0},
        {35, 0, 0},
        {53, CodeAddress(UpdateEasedDirectionalOffset), CodeAddress(InitializeDirectionalOffset)},
        {34, CodeAddress(UpdateDirectionalOffset60), CodeAddress(InitializeRandomDirectionalOffset)},
        {57, 0, 0}, {58, 0, 0}, {59, 0, 0}, {60, 0, 0},
        {48, 0, 0}, {49, 0, 0}, {50, 0, 0},
        {88, CodeAddress(SyncRadialTrailRadius), CodeAddress(InitializeRadialTrail)},
        {88, CodeAddress(UpdateBarrierRadialEffect), CodeAddress(InitializeBarrierRadialEffect)},
        {92, CodeAddress(UpdateBarrierRadialEffect), CodeAddress(InitializeRotatingBarrierRadialEffect)},
        {71, 0, 0},
        {76, CodeAddress(SyncRadialTrailRadius), CodeAddress(InitializeRadialTrail)},
        {81, CodeAddress(SyncRadialTrailShape), CodeAddress(InitializeRadialTrail)},
        {82, CodeAddress(UpdatePulsingRadialTrailCallback), CodeAddress(InitializeRadialTrail)},
        {83, CodeAddress(UpdateExpandingWavyRadialTrail), CodeAddress(InitializeRadialTrail)},
        {83, CodeAddress(UpdateExpandingPositiveDiagonalRadialTrail), CodeAddress(InitializeRadialTrail)},
        {83, CodeAddress(UpdateExpandingNegativeDiagonalRadialTrail), CodeAddress(InitializeRadialTrail)},
        {83, CodeAddress(UpdateExpandingOctagonalRadialTrail), CodeAddress(InitializeRadialTrail)},
        {84, CodeAddress(UpdateExpandingTwelveSegmentRadialTrail), CodeAddress(InitializeRadialTrail)},
        {72, 0, 0},
        {85, CodeAddress(UpdateExpandingOrthogonalRadialTrail), CodeAddress(InitializeRadialTrail)},
        {86, CodeAddress(SyncRadialTrailRadius), CodeAddress(InitializeRadialTrail)},
        {80, CodeAddress(UpdateTimedRadialTrail), CodeAddress(InitializeRadialTrail)},
        {73, CodeAddress(FUN_004264f0), CodeAddress(FUN_00426280)},
        {77, CodeAddress(SyncRadialTrailRadius), CodeAddress(InitializeRadialTrail)},
        {88, CodeAddress(UpdateFadingRadialTrail), CodeAddress(InitializeRadialTrail)},
        {88, CodeAddress(UpdateFadingRadialTrail), CodeAddress(InitializeRadialTrail)},
        {87, CodeAddress(SyncRadialTrailShape), CodeAddress(InitializeRadialTrail)},
        {96, CodeAddress(SyncRadialTrailShape), CodeAddress(InitializeAlternateLayerRadialTrail)},
        {55, 0, 0},
        {100, CodeAddress(SyncRadialTrailShape), CodeAddress(InitializeAlternateLayerRadialTrail)},
        {78, CodeAddress(SyncRadialTrailRadius), CodeAddress(InitializeRadialTrail)},
        {102, 0, CodeAddress(InitializeTrailOffset)},
        {103, 0, CodeAddress(InitializeTrailOffset)},
        {75, 0, 0},
        {74, CodeAddress(FUN_00426990), CodeAddress(FUN_00426720)},
        {77, CodeAddress(SyncAnchoredRadialTrail), CodeAddress(InitializeRadialTrail)},
        {98, CodeAddress(SyncRadialTrailShape), CodeAddress(InitializeAlternateLayerRadialTrail)},
    };
    static const int32_t stageScoreTables[9] = {
        1000000, 1500000, 2000000, 2500000, 2500000, 3000000, 4000000, 6000000, 6660000,
    };
    static const uint32_t messageTextColors[12][4] = {
        {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
        {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
        {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
        {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
        {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
        {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
        {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
        {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
        {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
        {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
        {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
        {0x00e8f0ff, 0x00f0e8ff, 0x00ffe8f0, 0x00ffe8f0},
    };
    *reinterpret_cast<int32_t *>(0x004c6c3c) = 43;
    memcpy(reinterpret_cast<void *>(0x004c6d30), effectTemplates, sizeof(effectTemplates));
    memcpy(reinterpret_cast<void *>(0x004c7158), stageScoreTables, sizeof(stageScoreTables));
    memcpy(reinterpret_cast<void *>(0x004c7180), messageTextColors, sizeof(messageTextColors));
}
}

bool ConfigureDataDirectory()
{
    const char *directory = NULL;
    for (int index = 1; index < g_argumentCount; ++index)
    {
        if (strcmp(g_arguments[index], "--data-dir") == 0)
        {
            if (++index >= g_argumentCount)
            {
                fprintf(stderr, "th08-modern: --data-dir requires a directory path\n");
                return false;
            }
            directory = g_arguments[index];
        }
        else if (strncmp(g_arguments[index], "--data-dir=", 11) == 0)
        {
            directory = g_arguments[index] + 11;
        }
    }

    if (directory != NULL && (directory[0] == '\0' || chdir(directory) != 0))
    {
        fprintf(stderr, "th08-modern: unable to enter data directory: %s\n", directory);
        return false;
    }

    struct stat info;
    if (stat("th08.dat", &info) != 0 || !S_ISREG(info.st_mode))
    {
        fprintf(stderr, "th08-modern: selected directory does not contain th08.dat\n");
        return false;
    }
    unlink("modern-files.txt");
    unlink("modern-crash.txt");
    unlink("modern-render.txt");
    return true;
}

void InstallCrashReporter()
{
    InitializeTargetData();
    InstallSignalHandler(SIGSEGV);
    InstallSignalHandler(SIGABRT);
    InstallSignalHandler(SIGFPE);
    InstallSignalHandler(SIGILL);
    InstallSignalHandler(SIGBUS);
}

void LogArchiveRequest(const char *path)
{
    FILE *file = fopen("modern-files.txt", "ab");
    if (file == NULL)
        return;
    fprintf(file, "thread=%08lx path=%s\n", (unsigned long)GetCurrentThreadId(), path != NULL ? path : "<null>");
    fclose(file);
}

void SetArguments(int argc, char **argv)
{
    g_argumentCount = argc;
    g_arguments = argv;
}
} // namespace modern
} // namespace th08

int main(int argc, char **argv)
{
    th08::modern::SetArguments(argc, argv);
    return WinMain(NULL, NULL, NULL, 0);
}
