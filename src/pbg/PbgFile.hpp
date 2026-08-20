#pragma once

#include "diffbuild.hpp"
#include "inttypes.hpp"
#include <windows.h>

namespace th08
{
class IPbgFile
{
  public:
    IPbgFile();

    virtual bool Open(const char *filename, char *mode) = 0;
    virtual void Close() = 0;
    virtual DWORD Read(LPVOID data, DWORD dataLen) = 0;
    virtual bool Write(LPVOID data, DWORD dataLen) = 0;
    virtual DWORD Tell() = 0;
    virtual DWORD GetSize() = 0;
    virtual bool Seek(DWORD offset, DWORD seekFrom) = 0;
    virtual ~IPbgFile();
};

class CPbgFile : public IPbgFile
{
  public:
    CPbgFile();
    virtual ~CPbgFile();

    virtual bool Open(const char *filename, char *mode);
    virtual void Close();
    virtual DWORD Read(LPVOID data, DWORD dataLen);
    virtual bool Write(LPVOID data, DWORD dataLen);
    virtual DWORD Tell();
    virtual DWORD GetSize();
    virtual bool Seek(DWORD offset, DWORD seekFrom);

    virtual HGLOBAL ReadWholeFile(DWORD maxSize);
    static void GetFullFilePath(char *buffer, const char *filename);

    DWORD ReadInt(i32 *outData);

  protected:
    HANDLE m_hFile;

  private:
    DWORD m_DesiredAccess;
};

DIFFABLE_EXTERN_ARRAY(char *, 3, g_PbgFileOpenModes);
DIFFABLE_EXTERN_ARRAY(i32, 3, g_PbgFileSeekModes);
}; // namespace th08
