#include "modern/windows_runtime.hpp"

#include <dbghelp.h>
#include <shellapi.h>
#include <stdio.h>
#include <wchar.h>
#include <windows.h>

namespace th08
{
namespace modern
{
namespace
{

void WriteLine(HANDLE file, const char *line)
{
    DWORD written;
    WriteFile(file, line, static_cast<DWORD>(strlen(line)), &written, NULL);
}

bool ShowDataDirectoryError(const wchar_t *message, const wchar_t *path)
{
    wchar_t text[1024];
    if (path != NULL)
        _snwprintf(text, sizeof(text) / sizeof(text[0]) - 1, L"%s\n\n%s", message, path);
    else
        _snwprintf(text, sizeof(text) / sizeof(text[0]) - 1, L"%s", message);
    text[sizeof(text) / sizeof(text[0]) - 1] = L'\0';
    MessageBoxW(NULL, text, L"TH08 reconstruction", MB_OK | MB_ICONERROR);
    return false;
}

LONG WINAPI ReportUnhandledException(EXCEPTION_POINTERS *exceptionInfo)
{
    HANDLE file = CreateFileA("modern-crash.txt", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
        return EXCEPTION_EXECUTE_HANDLER;

    char line[256];
    CONTEXT context = *exceptionInfo->ContextRecord;
    snprintf(line, sizeof(line), "exception=%08lx address=%p thread=%08lx\r\n",
             exceptionInfo->ExceptionRecord->ExceptionCode,
             exceptionInfo->ExceptionRecord->ExceptionAddress,
             GetCurrentThreadId());
    WriteLine(file, line);

#if defined(_M_IX86) || defined(__i386__)
    snprintf(line, sizeof(line), "eip=%08lx esp=%08lx ebp=%08lx eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx\r\n",
             context.Eip, context.Esp, context.Ebp, context.Eax, context.Ebx, context.Ecx, context.Edx);
    WriteLine(file, line);

    STACKFRAME64 frame;
    ZeroMemory(&frame, sizeof(frame));
    frame.AddrPC.Offset = context.Eip;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Esp;
    frame.AddrStack.Mode = AddrModeFlat;

    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    SymInitialize(process, NULL, TRUE);

    for (unsigned int depth = 0; depth < 64 && frame.AddrPC.Offset != 0; ++depth)
    {
        snprintf(line, sizeof(line), "#%02u %08llx\r\n", depth,
                 static_cast<unsigned long long>(frame.AddrPC.Offset));
        WriteLine(file, line);

        if (!StackWalk64(IMAGE_FILE_MACHINE_I386, process, thread, &frame, &context, NULL,
                         SymFunctionTableAccess64, SymGetModuleBase64, NULL))
            break;
    }

    SymCleanup(process);
#endif

    FlushFileBuffers(file);
    CloseHandle(file);
    return EXCEPTION_EXECUTE_HANDLER;
}

} // namespace

bool ConfigureDataDirectory()
{
    int argumentCount = 0;
    wchar_t **arguments = CommandLineToArgvW(GetCommandLineW(), &argumentCount);
    if (arguments == NULL)
        return ShowDataDirectoryError(L"Unable to parse the command line.", NULL);

    const wchar_t *dataDirectory = NULL;
    const wchar_t optionPrefix[] = L"--data-dir=";
    for (int index = 1; index < argumentCount; ++index)
    {
        if (wcscmp(arguments[index], L"--data-dir") == 0)
        {
            if (++index >= argumentCount)
            {
                LocalFree(arguments);
                return ShowDataDirectoryError(L"--data-dir requires a directory path.", NULL);
            }
            dataDirectory = arguments[index];
        }
        else if (wcsncmp(arguments[index], optionPrefix,
                         sizeof(optionPrefix) / sizeof(optionPrefix[0]) - 1) == 0)
        {
            dataDirectory = arguments[index] + sizeof(optionPrefix) / sizeof(optionPrefix[0]) - 1;
        }
    }

    if (dataDirectory == NULL)
    {
        LocalFree(arguments);
        return true;
    }
    if (*dataDirectory == L'\0')
    {
        LocalFree(arguments);
        return ShowDataDirectoryError(L"--data-dir cannot be empty.", NULL);
    }

    DWORD attributes = GetFileAttributesW(dataDirectory);
    if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        wchar_t invalidPath[1024];
        lstrcpynW(invalidPath, dataDirectory, sizeof(invalidPath) / sizeof(invalidPath[0]));
        LocalFree(arguments);
        return ShowDataDirectoryError(L"The data directory does not exist:", invalidPath);
    }

    if (!SetCurrentDirectoryW(dataDirectory))
    {
        wchar_t inaccessiblePath[1024];
        lstrcpynW(inaccessiblePath, dataDirectory, sizeof(inaccessiblePath) / sizeof(inaccessiblePath[0]));
        LocalFree(arguments);
        return ShowDataDirectoryError(L"Unable to enter the data directory:", inaccessiblePath);
    }
    LocalFree(arguments);

    attributes = GetFileAttributesW(L"th08.dat");
    if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        return ShowDataDirectoryError(L"The selected directory does not contain th08.dat.", NULL);
    return true;
}

void InstallCrashReporter()
{
    DeleteFileA("modern-files.txt");
    SetUnhandledExceptionFilter(ReportUnhandledException);
}

void LogArchiveRequest(const char *path)
{
    HANDLE file = CreateFileA("modern-files.txt", FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
        return;

    char line[512];
    snprintf(line, sizeof(line), "thread=%08lx path=%s\r\n", GetCurrentThreadId(), path != NULL ? path : "<null>");
    WriteLine(file, line);
    CloseHandle(file);
}

} // namespace modern
} // namespace th08
