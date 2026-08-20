#pragma once

#include "diffbuild.hpp"
#include "inttypes.hpp"
#include "pbg/PbgFile.hpp"
#include "pbg/PbgMemory.hpp"

namespace th08
{
struct PbgArchiveHeader
{
    i32 numOfEntries;
    i32 fileTableOffset;
    i32 unk;
};

struct PbgArchiveEntry
{
    // FUNCTION: th08 0x4716e0 FOLDED
    PbgArchiveEntry()
    {
        filename = NULL;
    }

    ~PbgArchiveEntry();

    char *filename;
    u32 dataOffset;
    u32 decompressedSize;
    u32 unk;
};

class PbgArchive
{
  public:
    PbgArchive();
    ~PbgArchive();

    bool Load(LPCSTR filename);
    void Release();
    LPBYTE ReadDecompressEntry(LPCSTR filename, LPBYTE outBuffer);
    DWORD GetEntryDecompressedSize(LPCSTR filename);
    PbgArchiveEntry *FindEntry(LPCSTR filename);
    bool ParseHeader(LPCSTR filename);
    PbgArchiveEntry *AllocEntries(LPVOID entryBuffer, i32 count, u32 offset);
    char *CopyFileName(LPCSTR filename);

    static i32 SeekPastInt(LPVOID *ptr);
    static LPVOID SeekPastString(LPVOID *ptr);

  private:
    PbgArchiveEntry *m_Entries;
    i32 m_NumOfEntries;
    char *m_Filename;
    CPbgFile *m_FileAbstraction;
};

}; // namespace th08
