#pragma once

#include "diffbuild.hpp"
#include "inttypes.hpp"
#include "pbg/PbgFile.hpp"
#include "pbg/PbgMemory.hpp"

#include <stddef.h>

namespace th08
{
struct PbgArchiveHeader
{
    i32 encodedEntryCount;
    i32 encodedFileTableOffset;
    i32 encodedFileTableDecompressedSize;
};
C_ASSERT(sizeof(PbgArchiveHeader) == 0xc);
C_ASSERT(offsetof(PbgArchiveHeader, encodedFileTableDecompressedSize) == 0x8);

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
    // Copied from each table record, but never read by the retail loader.
    u32 unconsumedMetadata;
};
C_ASSERT(sizeof(PbgArchiveEntry) == 0x10);
C_ASSERT(offsetof(PbgArchiveEntry, unconsumedMetadata) == 0xc);

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
