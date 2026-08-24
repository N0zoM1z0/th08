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

int __fastcall FUN_0040e040(AnmVm *);
int __fastcall FUN_0040e120(AnmVm *);
int __fastcall FUN_0040e200(AnmVm *);
int __fastcall FUN_0040e2d0(AnmVm *);
int __fastcall FUN_00410bb0(AnmVm *);
int __fastcall FUN_004114e0(AnmVm *);
int __fastcall FUN_00411720(AnmVm *);
int __fastcall FUN_00411a80(AnmVm *);
int __fastcall FUN_00413070(AnmVm *);

int __fastcall FUN_00426280(Effect *);
int __fastcall FUN_004264f0(Effect *);
int __fastcall FUN_00426720(Effect *);
int __fastcall FUN_00426990(Effect *);
int __fastcall FUN_00426b20(Effect *);
int __fastcall FUN_00426bb0(Effect *);
int __fastcall FUN_00426c40(Effect *);
int __fastcall FUN_00426c90(Effect *);
int __fastcall FUN_00426d70(Effect *);
int __fastcall FUN_00426e70(Effect *);
int __fastcall FUN_004270c0(Effect *);
int __fastcall FUN_004271a0(Effect *);
int __fastcall FUN_00427250(Effect *);
int __fastcall FUN_00427260(Effect *);
int __fastcall FUN_004272e0(Effect *);
int __fastcall FUN_00427970(Effect *);
int __fastcall FUN_00427990(Effect *);
int __fastcall FUN_004279d0(Effect *);
int __fastcall FUN_00427a60(Effect *);
int __fastcall FUN_00427ae0(Effect *);
int __fastcall FUN_00427b50(Effect *);

// This retail table entry points at an AnmVm member. On the 32-bit Linux ABI
// its code entry receives `this` as the first stack argument, matching the
// reconstructed effect callback invocation.
extern "C" int AnmVmUpdate0040eb50(AnmVm *) asm("_ZN4th085AnmVm12FUN_0040eb50Ev");

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

struct SpellPracticeMusic
{
    int32_t lastSpell;
    int32_t track;
    const char *path;
    int32_t visible;
    int32_t alternate;
};

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

void CopyTargetString(uintptr_t address, const char *value)
{
    memcpy(reinterpret_cast<void *>(address), value, strlen(value) + 1);
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
        {32, CodeAddress(FUN_00426bb0), CodeAddress(FUN_00426b20)},
        {33, CodeAddress(FUN_00426c90), CodeAddress(FUN_00426b20)},
        {51, CodeAddress(FUN_00426d70), CodeAddress(FUN_00426e70)},
        {56, 0, 0},
        {52, CodeAddress(FUN_004271a0), CodeAddress(FUN_004270c0)},
        {54, CodeAddress(FUN_00426c40), 0},
        {104, CodeAddress(FUN_00427250), 0},
        {104, CodeAddress(FUN_00427250), 0},
        {35, 0, 0},
        {53, CodeAddress(FUN_004271a0), CodeAddress(FUN_004270c0)},
        {34, CodeAddress(FUN_00426bb0), CodeAddress(FUN_00426b20)},
        {57, 0, 0}, {58, 0, 0}, {59, 0, 0}, {60, 0, 0},
        {48, 0, 0}, {49, 0, 0}, {50, 0, 0},
        {88, CodeAddress(FUN_00427990), CodeAddress(FUN_004272e0)},
        {88, CodeAddress(FUN_004114e0), CodeAddress(FUN_00411720)},
        {92, CodeAddress(FUN_004114e0), CodeAddress(FUN_00411a80)},
        {71, 0, 0},
        {76, CodeAddress(FUN_00427990), CodeAddress(FUN_004272e0)},
        {81, CodeAddress(FUN_004279d0), CodeAddress(FUN_004272e0)},
        {82, CodeAddress(AnmVmUpdate0040eb50), CodeAddress(FUN_004272e0)},
        {83, CodeAddress(FUN_0040e040), CodeAddress(FUN_004272e0)},
        {83, CodeAddress(FUN_0040e120), CodeAddress(FUN_004272e0)},
        {83, CodeAddress(FUN_0040e200), CodeAddress(FUN_004272e0)},
        {83, CodeAddress(FUN_0040e2d0), CodeAddress(FUN_004272e0)},
        {84, CodeAddress(FUN_00410bb0), CodeAddress(FUN_004272e0)},
        {72, 0, 0},
        {85, CodeAddress(FUN_00413070), CodeAddress(FUN_004272e0)},
        {86, CodeAddress(FUN_00427990), CodeAddress(FUN_004272e0)},
        {80, CodeAddress(FUN_00427a60), CodeAddress(FUN_004272e0)},
        {73, CodeAddress(FUN_004264f0), CodeAddress(FUN_00426280)},
        {77, CodeAddress(FUN_00427990), CodeAddress(FUN_004272e0)},
        {88, CodeAddress(FUN_00427ae0), CodeAddress(FUN_004272e0)},
        {88, CodeAddress(FUN_00427ae0), CodeAddress(FUN_004272e0)},
        {87, CodeAddress(FUN_004279d0), CodeAddress(FUN_004272e0)},
        {96, CodeAddress(FUN_004279d0), CodeAddress(FUN_00427970)},
        {55, 0, 0},
        {100, CodeAddress(FUN_004279d0), CodeAddress(FUN_00427970)},
        {78, CodeAddress(FUN_00427990), CodeAddress(FUN_004272e0)},
        {102, 0, CodeAddress(FUN_00427260)},
        {103, 0, CodeAddress(FUN_00427260)},
        {75, 0, 0},
        {74, CodeAddress(FUN_00426990), CodeAddress(FUN_00426720)},
        {77, CodeAddress(FUN_00427b50), CodeAddress(FUN_004272e0)},
        {98, CodeAddress(FUN_004279d0), CodeAddress(FUN_00427970)},
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
    static const int32_t stageMusicContexts[9][3] = {
        {1, 2, 0}, {3, 4, 0}, {5, 6, 0}, {7, 8, 0}, {7, 9, 0},
        {10, 11, 0}, {12, 13, 15}, {12, 14, 15}, {16, 17, 0},
    };
    static const SpellPracticeMusic spellPracticeMusic[] = {
        {1, 1, "th08_00.mid", 0, 0}, {12, 2, "th08_03.mid", 1, 0},
        {16, 3, "th08_04.mid", 0, 0}, {31, 4, "th08_05.mid", 1, 0},
        {35, 5, "th08_06.mid", 0, 0}, {53, 6, "th08_07.mid", 1, 0},
        {76, 8, "th08_09.mid", 1, 0}, {99, 9, "th08_10.mid", 1, 0},
        {118, 11, "th08_12.mid", 1, 0}, {122, 12, "th08_13.mid", 0, 0},
        {142, 13, "th08_14.mid", 1, 0}, {146, 15, "th08_13b.mid", 2, 1},
        {150, 12, "th08_13.mid", 0, 0}, {170, 14, "th08_15.mid", 1, 0},
        {190, 15, "th08_13b.mid", 2, 1}, {193, 16, "th08_18.mid", 0, 0},
        {204, 17, "th08_19.mid", 1, 0}, {222, 20, "th08_20.mid", 2, 0},
        {-1, 0, "", 0, 0},
    };

    *reinterpret_cast<int32_t *>(0x004c6c3c) = 43;
    memcpy(reinterpret_cast<void *>(0x004c6d30), effectTemplates, sizeof(effectTemplates));
    memcpy(reinterpret_cast<void *>(0x004c7158), stageScoreTables, sizeof(stageScoreTables));
    memcpy(reinterpret_cast<void *>(0x004c7180), messageTextColors, sizeof(messageTextColors));
    memcpy(reinterpret_cast<void *>(0x004c7240), stageMusicContexts, sizeof(stageMusicContexts));
    memcpy(reinterpret_cast<void *>(0x004c7670), spellPracticeMusic, sizeof(spellPracticeMusic));

    CopyTargetString(0x004b4ca0, "etama.anm");
    CopyTargetString(0x004b5820, "replay/th8_00.rpy");
    CopyTargetString(0x004b5834, "error: spell card initialization failed\n");
    CopyTargetString(0x004b5864, "error: 2D initialization failed\n");
    CopyTargetString(0x004b588c, "error: effect initialization failed\n");
    CopyTargetString(0x004b58b8, "error: enemy initialization failed\n");
    CopyTargetString(0x004b58dc, "error: bullet initialization failed\n");
    CopyTargetString(0x004b5904, "error: background initialization failed\n");
    CopyTargetString(0x004b5930, "error: player initialization failed\n");
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
