#include "th_pch.h"

#include "Global.hpp"
#include "i18n.hpp"
#include "pbg/Lzss.hpp"
#include "pbg/PbgArchive.hpp"
#include "utils.hpp"

namespace th08
{
PbgArchive::PbgArchive()
{
    m_Entries = NULL;
    m_NumOfEntries = 0;
    m_Filename = NULL;
    m_FileAbstraction = NULL;
}

PbgArchive::~PbgArchive()
{
    Release();
}

bool PbgArchive::Load(LPCSTR filename)
{
    Release();
    utils::DebugPrint("info : %s open arcfile\r\n", filename);

    m_FileAbstraction = NewEx(CPbgFile());
    if (m_FileAbstraction == NULL)
    {
        return false;
    }

    if (ParseHeader(filename))
    {
        m_Filename = CopyFileName(filename);
        if (m_Filename != NULL)
        {
            return true;
        }
    }

    utils::DebugPrint("info : %s not found\r\n", filename);
    Release();
    return false;
}

void PbgArchive::Release()
{
    if (m_Filename != NULL)
    {
        utils::DebugPrint("info : %s close arcfile\r\n", m_Filename);
    }
    MemFree(m_Filename);
    DeleteArray(m_Entries);
    DeleteEx(m_FileAbstraction);
    m_NumOfEntries = 0;
}

#pragma var_order(entry, decompressedSize, decompressedData, compressedData, compressedSize)
LPBYTE PbgArchive::ReadDecompressEntry(LPCSTR filename, LPBYTE outBuffer)
{
    LPBYTE decompressedData;
    LPBYTE compressedData = NULL;
    u32 compressedSize;
    u32 decompressedSize;

    if (m_FileAbstraction == NULL)
    {
        return NULL;
    }

    PbgArchiveEntry *entry = FindEntry(filename);
    if (entry == NULL)
    {
        goto entry_read_error;
    }

    if (m_FileAbstraction->Open(m_Filename, g_PbgFileOpenModes[0]) == false)
    {
        goto entry_read_error;
    }

    compressedSize = entry[1].dataOffset - entry->dataOffset;
    decompressedSize = entry->decompressedSize;

    compressedData = (LPBYTE)MemAlloc(compressedSize);
    if (compressedData == NULL)
    {
        goto entry_read_error;
    }
    if (m_FileAbstraction->Seek(entry->dataOffset, g_PbgFileSeekModes[0]) == false)
    {
        goto entry_read_error;
    }
    if (m_FileAbstraction->Read(compressedData, compressedSize) == 0)
    {
        goto entry_read_error;
    }

    decompressedData = Lzss::Decode(compressedData, compressedSize, outBuffer, decompressedSize);
    MemFree(compressedData);
    return decompressedData;

entry_read_error:
    utils::DebugPrint("info : %s error\r\n", m_Filename);
    MemFree(compressedData);
    return NULL;
}

DWORD PbgArchive::GetEntryDecompressedSize(LPCSTR filename)
{
    PbgArchiveEntry *entry = FindEntry(filename);
    if (entry != NULL)
        return entry->decompressedSize;
    return 0;
}

PbgArchiveEntry *PbgArchive::FindEntry(LPCSTR filename)
{
    if (m_Entries == NULL)
    {
        return NULL;
    }

    PbgArchiveEntry *entry = m_Entries;
    for (i32 i = m_NumOfEntries; i > 0; i--, entry++ /* PBG why */)
    {
        if (_stricmp(filename, entry->filename) == 0)
            return entry;
    }
    return NULL;
}

#pragma var_order(entryBuffer, decompressedSize, magic, size, fileTableOffset, fileTableBuffer, header,                \
                  decryptedHeader, decryptedFileTable)
bool PbgArchive::ParseHeader(LPCSTR filename)
{
    LPBYTE entryBuffer;
    i32 decompressedSize;
    i32 magic;
    DWORD size;
    i32 fileTableOffset;
    union {
        PbgArchiveHeader asStruct;
        BYTE asBytes[sizeof(PbgArchiveHeader)];
    } header;
    LPBYTE fileTableBuffer;
    LPBYTE decryptedHeader;
    LPBYTE decryptedFileTable;

    fileTableBuffer = NULL;
    entryBuffer = NULL;

    if (m_FileAbstraction == NULL)
    {
        return false;
    }
    if (!m_FileAbstraction->Open(filename, g_PbgFileOpenModes[0]))
    {
        goto parse_error;
    }
    if (m_FileAbstraction->ReadInt(&magic) == 0)
    {
        goto parse_error;
    }
    if (magic != 'ZGBP')
    {
        goto parse_error;
    }
    if (m_FileAbstraction->Read(header.asBytes, sizeof(header)) == 0)
    {
        goto parse_error;
    }

    decryptedHeader = FileSystem::Decrypt(header.asBytes, sizeof(header), 0x1b, 0x37, sizeof(header), 0x400);
    memcpy(&header.asStruct, decryptedHeader, sizeof(header));
    g_ZunMemory.Free(decryptedHeader);

    m_NumOfEntries = header.asStruct.numOfEntries - 123456;
    fileTableOffset = header.asStruct.fileTableOffset - 345678;
    decompressedSize = header.asStruct.unk - 567891;

    if (m_NumOfEntries <= 0)
    {
        goto parse_error;
    }

    size = m_FileAbstraction->GetSize();
    if (fileTableOffset >= size)
    {
        goto parse_error;
    }
    size -= fileTableOffset;

    m_FileAbstraction->Seek(fileTableOffset, g_PbgFileSeekModes[0]);

    fileTableBuffer = (LPBYTE)MemAlloc(size);
    if (fileTableBuffer == NULL)
    {
        goto parse_error;
    }
    if (m_FileAbstraction->Read(fileTableBuffer, size) == 0)
    {
        goto parse_error;
    }

    decryptedFileTable = FileSystem::Decrypt(fileTableBuffer, size, 0x3e, 0x9b, 0x80, 0x400);
    MemFree(fileTableBuffer);
    fileTableBuffer = decryptedFileTable;

    entryBuffer = Lzss::Decode(fileTableBuffer, size, NULL, decompressedSize);
    if (entryBuffer == NULL)
    {
        goto parse_error;
    }

    m_Entries = AllocEntries(entryBuffer, m_NumOfEntries, fileTableOffset);
    if (m_Entries == NULL)
    {
        goto parse_error;
    }

    g_ZunMemory.Free(fileTableBuffer);
    MemFree(entryBuffer);
    return true;

parse_error:
    g_ZunMemory.Free(fileTableBuffer);
    MemFree(entryBuffer);
    DeleteEx(m_FileAbstraction);
    utils::DebugPrint(TH_ERR_ARCFILE_CORRUPTED, filename);

    while (false)
        ; // Yes this is correct. No, I don't get it either.

    return false;
}

#pragma var_order(entryData, i, buffer)
PbgArchiveEntry *PbgArchive::AllocEntries(LPVOID entryBuffer, i32 count, u32 dataOffset)
{
    LPVOID entryData;
    int i;
    PbgArchiveEntry *buffer = NULL;

    buffer = new PbgArchiveEntry[count + 1]();
    if (buffer == NULL)
    {
        goto buffer_alloc_error;
    }

    entryData = entryBuffer;
    for (i = 0; i < count; i++)
    {
        buffer[i].filename = CopyFileName((char *)entryData);
        SeekPastString(&entryData);
        buffer[i].dataOffset = *(u32 *)entryData;
        SeekPastInt(&entryData);
        buffer[i].decompressedSize = *(u32 *)entryData;
        SeekPastInt(&entryData);
        buffer[i].unk = *(u32 *)entryData;
        SeekPastInt(&entryData);
    }

    buffer[count].dataOffset = dataOffset;
    buffer[count].decompressedSize = 0;
    return buffer;

buffer_alloc_error:
    DeleteArray(buffer);
    return NULL;
}

char *PbgArchive::CopyFileName(LPCSTR filename)
{
    char *mem = (char *)MemAlloc(strlen(filename) + 1);
    if (mem != NULL)
    {
        strcpy(mem, filename);
    }
    return mem;
}

// FUNCTION: th08 0x4751e0
DWORD CPbgFile::ReadInt(i32 *outData)
{
    return Read(outData, 4);
}

i32 PbgArchive::SeekPastInt(LPVOID *ptr)
{
    *ptr = (i32 *)*ptr + 1;
    return *(i32 *)*ptr;
}

LPVOID PbgArchive::SeekPastString(LPVOID *ptr)
{
    *ptr = (char *)*ptr + (strlen((char *)*ptr) + 1);
    return *ptr;
}

// FUNCTION: th08 0x475270
PbgArchiveEntry::~PbgArchiveEntry()
{
    MemFree(filename);
}
}; // namespace th08
