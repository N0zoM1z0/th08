#pragma once
#include <windows.h>

struct IPersistFile
{
    virtual HRESULT Load(LPCWSTR, DWORD) = 0;
    virtual ULONG Release() = 0;
};

struct IShellLink
{
    virtual HRESULT QueryInterface(REFIID, void **) = 0;
    virtual HRESULT GetPath(LPSTR, int, WIN32_FIND_DATA *, DWORD) = 0;
    virtual ULONG Release() = 0;
};
