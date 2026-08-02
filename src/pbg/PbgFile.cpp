#include "th_pch.h"

#include "pbg/PbgFile.hpp"

namespace th08
{
DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 3, g_PbgFileSeekModes) = {FILE_BEGIN, FILE_CURRENT, FILE_END};
DIFFABLE_STATIC_ARRAY_ASSIGN(char *, 3, g_PbgFileOpenModes) = {"r", "w", "a"};

CPbgFile::CPbgFile()
{
    m_hFile = INVALID_HANDLE_VALUE;
    m_DesiredAccess = 0;
}

CPbgFile::~CPbgFile()
{
    Close();
}

#pragma var_order(curMode, goToEnd, filePathBuffer, creationDisposition)
bool CPbgFile::Open(const char *filename, char *mode)
{
    DWORD creationDisposition;
    BOOL goToEnd = FALSE;
    char filePathBuffer[MAX_PATH];

    Close();

    char *curMode;
    for (curMode = mode; *curMode != '\0'; curMode++)
    {
        if (*curMode == 'r')
        {
            m_DesiredAccess = GENERIC_READ;
            creationDisposition = OPEN_EXISTING;
            break;
        }
        if (*curMode == 'w')
        {
            DeleteFileA(filename);
            m_DesiredAccess = GENERIC_WRITE;
            creationDisposition = CREATE_ALWAYS;
            break;
        }
        if (*curMode == 'a')
        {
            goToEnd = TRUE;
            m_DesiredAccess = GENERIC_WRITE;
            creationDisposition = OPEN_ALWAYS;
            break;
        }
    }

    if (*curMode == '\0')
    {
        return false;
    }

    GetFullFilePath(filePathBuffer, filename);
    m_hFile = CreateFileA(filePathBuffer, m_DesiredAccess, FILE_SHARE_READ, NULL, creationDisposition,
                          FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    if (goToEnd)
    {
        SetFilePointer(m_hFile, 0, NULL, FILE_END);
    }
    return true;
}

void CPbgFile::Close()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
        m_DesiredAccess = 0;
    }
}

DWORD CPbgFile::Read(LPVOID data, DWORD dataLen)
{
    DWORD numBytesRead = 0;

    if (m_DesiredAccess != GENERIC_READ)
    {
        return 0;
    }

    ReadFile(m_hFile, data, dataLen, &numBytesRead, NULL);
    return numBytesRead;
}

bool CPbgFile::Write(LPVOID data, DWORD dataLen)
{
    DWORD outWritten = 0;

    if (m_DesiredAccess != GENERIC_WRITE)
    {
        return false;
    }

    WriteFile(m_hFile, data, dataLen, &outWritten, NULL);
    return dataLen == outWritten ? true : false;
}

DWORD CPbgFile::Tell()
{
    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    return SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
}

DWORD CPbgFile::GetSize()
{
    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    return GetFileSize(m_hFile, NULL);
}

bool CPbgFile::Seek(DWORD offset, DWORD seekFrom)
{
    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    SetFilePointer(m_hFile, offset, NULL, seekFrom);
    return true;
}

#pragma var_order(data, dataLen, oldLocation)
HGLOBAL CPbgFile::ReadWholeFile(DWORD maxSize)
{
    if (m_DesiredAccess != GENERIC_READ)
    {
        return NULL;
    }

    DWORD dataLen = GetSize();
    if (dataLen > maxSize)
    {
        return NULL;
    }

    HGLOBAL data = GlobalAlloc(LPTR, dataLen);
    if (data == NULL)
    {
        return NULL;
    }

    DWORD oldLocation = Tell();
    if (Seek(oldLocation, g_PbgFileSeekModes[0]) == 0)
    {
        return NULL;
    }

    if (Read(data, dataLen) == 0)
    {
        if (data)
        {
            GlobalFree(data);
            data = NULL;
        }
        return NULL;
    }

    Seek(oldLocation, g_PbgFileSeekModes[0]);
    return data;
}

void CPbgFile::GetFullFilePath(char *buffer, const char *filename)
{
    if (strchr(filename, ':') != NULL)
    {
        strcpy(buffer, filename);
    }
    else
    {
        GetModuleFileNameA(NULL, buffer, MAX_PATH);

        char *endOfModulePath = strrchr(buffer, '\\');
        if (endOfModulePath == NULL)
        {
            strcpy(buffer, "");
        }

        endOfModulePath[1] = '\0';
        strcat(buffer, filename);
    }
}
}; // namespace th08
