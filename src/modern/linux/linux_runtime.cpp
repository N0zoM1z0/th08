#include "modern/windows_runtime.hpp"

#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPTSTR, int);

namespace th08
{
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

} // namespace

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
