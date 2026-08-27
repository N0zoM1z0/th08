#include "solver_bridge.hpp"

#include "Global.hpp"
#include "Supervisor.hpp"

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

extern "C"
{
volatile uint32_t th08_solver_input_epoch = 0;
}

namespace th08
{
namespace modern
{
namespace
{

const uint32_t REQUEST_MAGIC = 0x51523854;  // "T8RQ" as little endian.
const uint32_t RESPONSE_MAGIC = 0x53523854; // "T8RS" as little endian.
const uint32_t SNAPSHOT_RELEASE_MAGIC = 0x4c523854; // "T8RL".
const uint32_t SNAPSHOT_MAGIC = 0x4e533854;         // "T8SN".
const uint16_t PROTOCOL_VERSION = 3;
const size_t REQUEST_SIZE = 80;
const size_t RESPONSE_SIZE = 32;
const size_t SNAPSHOT_RELEASE_SIZE = 24;
const uint16_t BOMB_MASK = 0x0002;
const uint16_t KNOWN_INPUT_MASK = 0x7ffd;
const uint16_t UP_MASK = 0x0010;
const uint16_t DOWN_MASK = 0x0020;
const uint16_t LEFT_MASK = 0x0040;
const uint16_t RIGHT_MASK = 0x0080;
const uint16_t SAFE_NEUTRAL_MASK = 0x0005; // Shot + Focus.
const uint32_t REQUEST_FLAG_REPLAY_TARGET_STAMPED = 1;
const uint32_t REQUEST_FLAG_LIVES_PRESERVED = 2;
const uint32_t REQUEST_FLAG_IMMUTABLE_SNAPSHOT_PRESENT = 4;
const uint32_t ORIGINAL_EXE_SIZE = 840704;
const uint32_t ORIGINAL_EXE_CHECKSUM = 2724749753U;

const uint16_t NO_SNAPSHOT_SLOT = 0xffff;
const uint16_t SNAPSHOT_VERSION = 1;
const uint32_t SNAPSHOT_HEADER_SIZE = 80;
const uint32_t SNAPSHOT_ENTRY_SIZE = 16;
const uint32_t SNAPSHOT_MAXIMUM_ENTRIES = 8192;
const uint32_t SNAPSHOT_DATA_OFFSET =
    SNAPSHOT_HEADER_SIZE + SNAPSHOT_MAXIMUM_ENTRIES * SNAPSHOT_ENTRY_SIZE;
const uint32_t SNAPSHOT_SLOT_CAPACITY = 32U * 1024U * 1024U;
const uint32_t SNAPSHOT_SLOT_COUNT = 2;
const uint32_t SNAPSHOT_FLAG_COMPLETE = 1;
const uint32_t RANGE_KIND_EXACT = 0;
const uint32_t RANGE_KIND_BULLET = 1;
const uint32_t RANGE_KIND_LASER = 2;
const uint32_t RANGE_KIND_ENEMY = 3;
const uint32_t RANGE_KIND_ITEM = 4;

const uint32_t ENGINE_FLAGS_ADDRESS = 0x0164d0b4;
const uint32_t GAMEPLAY_ACTIVE_FLAG = 0x00000004;
const uint32_t UPDATE_SERIAL_ADDRESS = 0x0160f428;
const uint32_t ENEMY_MANAGER_FRAME_ADDRESS = 0x0164d30c;
const uint32_t PLAYER_BASE = 0x017d5ef8;
const uint32_t PLAYER_SNAPSHOT_SIZE = 0x000e2b00;
const uint32_t PLAYER_PRIMARY_SHT_POINTER_OFFSET = 0x000e2a74;
const uint32_t PLAYER_SECONDARY_SHT_POINTER_OFFSET = 0x000e2a78;
const uint32_t PRIMARY_SHT_SIZE = 1584;
const uint32_t SECONDARY_SHT_SIZE = 3568;
const uint32_t SPELL_STATE_ADDRESS = 0x004ea670;
const uint32_t SPELL_STATE_SIZE = 0x114;
const uint32_t ECL_CONTEXT_ADDRESS = 0x004eccb8;
const uint32_t ECL_HEADER_SIZE = 0x48;
const uint32_t ECL_MAGIC = 0x800;
const uint32_t MAXIMUM_ECL_IMAGE_SIZE = 8U * 1024U * 1024U;
const uint32_t TIMELINE_RUNTIME_ADDRESS = 0x00f5a0c0;
const uint32_t BULLET_TEMPLATE_ADDRESS = 0x00f54e90;
const uint32_t BULLET_TEMPLATE_SIZE = 21 * 0x0d44;
const uint32_t BULLET_POOL_ADDRESS = 0x00f6f710;
const uint32_t BULLET_POOL_COUNT = 1536;
const uint32_t BULLET_STRIDE = 0x10b8;
const uint32_t BULLET_STATE_OFFSET = 0x0db8;
const uint32_t LASER_POOL_ADDRESS = 0x015b57c8;
const uint32_t LASER_POOL_COUNT = 256;
const uint32_t LASER_STRIDE = 0x059c;
const uint32_t LASER_ACTIVE_OFFSET = 0x0584;
const uint32_t ITEM_POOL_ADDRESS = 0x01653648;
const uint32_t ITEM_POOL_COUNT = 2096;
const uint32_t ITEM_STRIDE = 0x02e4;
const uint32_t ITEM_ACTIVE_OFFSET = 0x02d5;
const uint32_t ENEMY_MANAGER_TEMPLATE_ADDRESS = 0x0057d2f0;
const uint32_t ENEMY_POOL_ADDRESS = 0x005826c0;
const uint32_t ENEMY_POOL_COUNT = 480;
const uint32_t ENEMY_STRIDE = 0x53d0;
const uint32_t ENEMY_FLAGS_OFFSET = 0x3324;
const uint32_t ENEMY_ACTIVE_FLAG = 1;
const uint32_t ENEMY_AUXILIARY_POINTERS_OFFSET = 0x3384;
const uint32_t AUXILIARY_CONTEXT_SIZE = 0x230;
const uint32_t INDEXED_ENEMY_REGISTRY_ADDRESS = 0x00f54cc0;
const uint32_t INDEXED_ENEMY_REGISTRY_COUNT = 8;
const uint32_t INDEXED_ENEMY_TIMELINE_FIELD_OFFSET = 0x2d30;

struct SnapshotSlot
{
    SnapshotSlot()
        : data(NULL), generation(0), size(0), entryCount(0), leased(false)
    {
    }

    unsigned char *data;
    uint64_t generation;
    uint32_t size;
    uint32_t entryCount;
    bool leased;
};

struct BridgeState
{
    BridgeState()
        : configured(false), initialized(false), failed(false), server(-1),
          client(-1), inputEpoch(0), lastPublishedInputEpoch(0),
          deadlineMisses(0), lateResponses(0), droppedRequests(0),
          droppedSnapshots(0), nextSnapshotGeneration(0),
          preserveLives(true)
    {
    }

    bool configured;
    bool initialized;
    bool failed;
    int server;
    int client;
    uint64_t inputEpoch;
    uint64_t lastPublishedInputEpoch;
    uint64_t deadlineMisses;
    uint64_t lateResponses;
    uint64_t droppedRequests;
    uint64_t droppedSnapshots;
    uint64_t nextSnapshotGeneration;
    bool preserveLives;
    SnapshotSlot snapshots[SNAPSHOT_SLOT_COUNT];
};

BridgeState g_bridge;

uint64_t MonotonicMicroseconds()
{
    struct timespec value;
    if (clock_gettime(CLOCK_MONOTONIC, &value) != 0)
        return 0;
    return static_cast<uint64_t>(value.tv_sec) * 1000000ULL +
           static_cast<uint64_t>(value.tv_nsec / 1000);
}

uint32_t SaturatingU32(uint64_t value)
{
    return value > 0xffffffffULL ? 0xffffffffU : static_cast<uint32_t>(value);
}

void PutU16(unsigned char *output, size_t offset, uint16_t value)
{
    output[offset] = static_cast<unsigned char>(value);
    output[offset + 1] = static_cast<unsigned char>(value >> 8);
}

void PutU32(unsigned char *output, size_t offset, uint32_t value)
{
    for (unsigned int index = 0; index < 4; ++index)
        output[offset + index] = static_cast<unsigned char>(value >> (index * 8));
}

void PutU64(unsigned char *output, size_t offset, uint64_t value)
{
    for (unsigned int index = 0; index < 8; ++index)
        output[offset + index] = static_cast<unsigned char>(value >> (index * 8));
}

uint16_t GetU16(const unsigned char *input, size_t offset)
{
    return static_cast<uint16_t>(input[offset]) |
           static_cast<uint16_t>(input[offset + 1]) << 8;
}

uint32_t GetU32(const unsigned char *input, size_t offset)
{
    uint32_t value = 0;
    for (unsigned int index = 0; index < 4; ++index)
        value |= static_cast<uint32_t>(input[offset + index]) << (index * 8);
    return value;
}

uint64_t GetU64(const unsigned char *input, size_t offset)
{
    uint64_t value = 0;
    for (unsigned int index = 0; index < 8; ++index)
        value |= static_cast<uint64_t>(input[offset + index]) << (index * 8);
    return value;
}

uint16_t LoadU16(uint32_t address)
{
    uint16_t value;
    memcpy(&value, reinterpret_cast<const void *>(address), sizeof(value));
    return value;
}

uint32_t LoadU32(uint32_t address)
{
    uint32_t value;
    memcpy(&value, reinterpret_cast<const void *>(address), sizeof(value));
    return value;
}

bool AddressRangeLooksUsable(uint32_t address, uint32_t size)
{
    return address >= 0x00010000U && size > 0 &&
           static_cast<uint64_t>(address) + size <= 0x100000000ULL;
}

struct SnapshotBuilder
{
    SnapshotBuilder(unsigned char *slotData, uint64_t slotGeneration,
                    uint64_t slotSourceEpoch)
        : data(slotData), generation(slotGeneration),
          sourceEpoch(slotSourceEpoch), dataCursor(SNAPSHOT_DATA_OFFSET),
          entryCount(0), bulletCount(0), laserCount(0), enemyCount(0),
          itemCount(0), auxiliaryCount(0), failed(false)
    {
        memset(data, 0, SNAPSHOT_DATA_OFFSET);
    }

    bool AddRange(uint32_t address, uint32_t size, uint32_t kind)
    {
        const uint64_t alignedEnd =
            (static_cast<uint64_t>(dataCursor) + size + 3U) & ~3ULL;
        if (failed || !AddressRangeLooksUsable(address, size) ||
            entryCount >= SNAPSHOT_MAXIMUM_ENTRIES ||
            alignedEnd > SNAPSHOT_SLOT_CAPACITY)
        {
            failed = true;
            return false;
        }
        const uint32_t entryOffset =
            SNAPSHOT_HEADER_SIZE + entryCount * SNAPSHOT_ENTRY_SIZE;
        PutU32(data, entryOffset + 0, address);
        PutU32(data, entryOffset + 4, size);
        PutU32(data, entryOffset + 8, dataCursor);
        PutU32(data, entryOffset + 12, kind);
        memcpy(data + dataCursor, reinterpret_cast<const void *>(address), size);
        if (alignedEnd > static_cast<uint64_t>(dataCursor) + size)
            memset(data + dataCursor + size, 0,
                   static_cast<size_t>(alignedEnd - dataCursor - size));
        dataCursor = static_cast<uint32_t>(alignedEnd);
        ++entryCount;
        if (kind == RANGE_KIND_BULLET)
            ++bulletCount;
        else if (kind == RANGE_KIND_LASER)
            ++laserCount;
        else if (kind == RANGE_KIND_ENEMY)
            ++enemyCount;
        else if (kind == RANGE_KIND_ITEM)
            ++itemCount;
        return true;
    }

    bool AddExact(uint32_t address, uint32_t size)
    {
        return AddRange(address, size, RANGE_KIND_EXACT);
    }

    void AddAuxiliaryContexts(uint32_t enemyAddress)
    {
        for (uint32_t index = 0; index < 4 && !failed; ++index)
        {
            const uint32_t pointer = LoadU32(
                enemyAddress + ENEMY_AUXILIARY_POINTERS_OFFSET + index * 4);
            if (pointer != 0)
            {
                if (AddExact(pointer, AUXILIARY_CONTEXT_SIZE))
                    ++auxiliaryCount;
            }
        }
    }

    uint32_t Finish(uint32_t managerFrame, uint32_t updateSerial)
    {
        if (failed)
            return 0;
        PutU32(data, 0, SNAPSHOT_MAGIC);
        PutU16(data, 4, SNAPSHOT_VERSION);
        PutU16(data, 6, SNAPSHOT_HEADER_SIZE);
        PutU64(data, 8, generation);
        PutU64(data, 16, sourceEpoch);
        PutU32(data, 24, managerFrame);
        PutU32(data, 28, updateSerial);
        PutU32(data, 32, dataCursor);
        PutU32(data, 36, entryCount);
        PutU32(data, 40, SNAPSHOT_HEADER_SIZE);
        PutU32(data, 44, SNAPSHOT_DATA_OFFSET);
        PutU32(data, 48, SNAPSHOT_FLAG_COMPLETE);
        PutU32(data, 52, bulletCount);
        PutU32(data, 56, laserCount);
        PutU32(data, 60, enemyCount);
        PutU32(data, 64, itemCount);
        PutU32(data, 68, auxiliaryCount);
        return dataCursor;
    }

    unsigned char *data;
    uint64_t generation;
    uint64_t sourceEpoch;
    uint32_t dataCursor;
    uint32_t entryCount;
    uint32_t bulletCount;
    uint32_t laserCount;
    uint32_t enemyCount;
    uint32_t itemCount;
    uint32_t auxiliaryCount;
    bool failed;
};

void ReleaseAllSnapshots()
{
    for (uint32_t index = 0; index < SNAPSHOT_SLOT_COUNT; ++index)
        g_bridge.snapshots[index].leased = false;
}

bool AllocateSnapshotStorage()
{
    for (uint32_t index = 0; index < SNAPSHOT_SLOT_COUNT; ++index)
    {
        if (g_bridge.snapshots[index].data == NULL)
        {
            g_bridge.snapshots[index].data =
                static_cast<unsigned char *>(malloc(SNAPSHOT_SLOT_CAPACITY));
            if (g_bridge.snapshots[index].data == NULL)
                return false;
        }
    }
    return true;
}

void PackDynamicRootRanges(SnapshotBuilder *builder)
{
    const uint32_t runState = LoadU32(0x0160f510);
    if (runState != 0)
    {
        builder->AddExact(runState + 0x74, 4);
        builder->AddExact(runState + 0x80, 4);
        builder->AddExact(runState + 0x98, 4);
    }

    const uint32_t frscreen = LoadU32(0x0160f430);
    if (frscreen != 0)
    {
        builder->AddExact(frscreen + 0x21814, 12);
        builder->AddExact(frscreen + 0x22d78, 4);
    }

    const uint32_t primarySht =
        LoadU32(PLAYER_BASE + PLAYER_PRIMARY_SHT_POINTER_OFFSET);
    const uint32_t secondarySht =
        LoadU32(PLAYER_BASE + PLAYER_SECONDARY_SHT_POINTER_OFFSET);
    if (primarySht != 0)
        builder->AddExact(primarySht, PRIMARY_SHT_SIZE);
    if (secondarySht != 0)
        builder->AddExact(secondarySht, SECONDARY_SHT_SIZE);

    const uint32_t eclBase = LoadU32(ECL_CONTEXT_ADDRESS);
    const uint32_t subroutineTable = LoadU32(ECL_CONTEXT_ADDRESS + 4);
    if (eclBase != 0 && subroutineTable == eclBase + ECL_HEADER_SIZE &&
        LoadU32(eclBase) == ECL_MAGIC)
    {
        const uint16_t timelineCount = LoadU16(eclBase + 6);
        if (timelineCount <= 15)
        {
            const uint32_t eclEnd = LoadU32(eclBase + 8 + timelineCount * 4);
            if (eclEnd > eclBase &&
                eclEnd - eclBase <= MAXIMUM_ECL_IMAGE_SIZE)
                builder->AddExact(eclBase, eclEnd - eclBase);
        }
    }
}

void PackActivePools(SnapshotBuilder *builder)
{
    builder->AddRange(
        ENEMY_MANAGER_TEMPLATE_ADDRESS, ENEMY_STRIDE, RANGE_KIND_ENEMY);
    if (LoadU32(ENEMY_MANAGER_TEMPLATE_ADDRESS + ENEMY_FLAGS_OFFSET) &
        ENEMY_ACTIVE_FLAG)
        builder->AddAuxiliaryContexts(ENEMY_MANAGER_TEMPLATE_ADDRESS);

    for (uint32_t slot = 0; slot < ENEMY_POOL_COUNT && !builder->failed; ++slot)
    {
        const uint32_t address = ENEMY_POOL_ADDRESS + slot * ENEMY_STRIDE;
        if (LoadU32(address + ENEMY_FLAGS_OFFSET) & ENEMY_ACTIVE_FLAG)
        {
            builder->AddRange(address, ENEMY_STRIDE, RANGE_KIND_ENEMY);
            builder->AddAuxiliaryContexts(address);
        }
    }
    for (uint32_t slot = 0; slot < BULLET_POOL_COUNT && !builder->failed; ++slot)
    {
        const uint32_t address = BULLET_POOL_ADDRESS + slot * BULLET_STRIDE;
        if (LoadU16(address + BULLET_STATE_OFFSET) != 0)
            builder->AddRange(address, BULLET_STRIDE, RANGE_KIND_BULLET);
    }
    for (uint32_t slot = 0; slot < LASER_POOL_COUNT && !builder->failed; ++slot)
    {
        const uint32_t address = LASER_POOL_ADDRESS + slot * LASER_STRIDE;
        if (LoadU32(address + LASER_ACTIVE_OFFSET) != 0)
            builder->AddRange(address, LASER_STRIDE, RANGE_KIND_LASER);
    }
    for (uint32_t slot = 0; slot < ITEM_POOL_COUNT && !builder->failed; ++slot)
    {
        const uint32_t address = ITEM_POOL_ADDRESS + slot * ITEM_STRIDE;
        if (*reinterpret_cast<const unsigned char *>(address + ITEM_ACTIVE_OFFSET))
            builder->AddRange(address, ITEM_STRIDE, RANGE_KIND_ITEM);
    }

    const uint32_t spellFlags = LoadU32(SPELL_STATE_ADDRESS);
    const uint32_t spellOwner = LoadU32(SPELL_STATE_ADDRESS + 4);
    const uint32_t enemyPoolEnd = ENEMY_POOL_ADDRESS + ENEMY_POOL_COUNT * ENEMY_STRIDE;
    if ((spellFlags & 1) != 0 && spellOwner != 0 &&
        spellOwner != ENEMY_MANAGER_TEMPLATE_ADDRESS &&
        !(spellOwner >= ENEMY_POOL_ADDRESS && spellOwner < enemyPoolEnd))
    {
        builder->AddRange(spellOwner, ENEMY_STRIDE, RANGE_KIND_ENEMY);
        builder->AddAuxiliaryContexts(spellOwner);
    }
}

bool IsKnownEnemyRecord(uint32_t pointer, uint32_t externalSpellOwner)
{
    if (pointer == ENEMY_MANAGER_TEMPLATE_ADDRESS ||
        pointer == externalSpellOwner)
        return true;
    const uint32_t enemyPoolEnd =
        ENEMY_POOL_ADDRESS + ENEMY_POOL_COUNT * ENEMY_STRIDE;
    return pointer >= ENEMY_POOL_ADDRESS && pointer < enemyPoolEnd &&
           (pointer - ENEMY_POOL_ADDRESS) % ENEMY_STRIDE == 0;
}

void PackIndexedEnemyFields(SnapshotBuilder *builder)
{
    const uint32_t spellFlags = LoadU32(SPELL_STATE_ADDRESS);
    const uint32_t spellOwner =
        (spellFlags & 1) != 0 ? LoadU32(SPELL_STATE_ADDRESS + 4) : 0;
    for (uint32_t index = 0;
         index < INDEXED_ENEMY_REGISTRY_COUNT && !builder->failed;
         ++index)
    {
        const uint32_t pointer =
            LoadU32(INDEXED_ENEMY_REGISTRY_ADDRESS + index * 4);
        if (pointer == 0)
            continue;
        if (!IsKnownEnemyRecord(pointer, spellOwner))
        {
            builder->failed = true;
            return;
        }
        // The future timeline inventory dereferences these fields even when a
        // registry entry names an inactive slot.  Preserve those exact stale
        // bytes instead of letting sparse-pool reconstruction zero them.
        builder->AddExact(pointer + ENEMY_FLAGS_OFFSET, 4);
        builder->AddExact(pointer + INDEXED_ENEMY_TIMELINE_FIELD_OFFSET, 2);
    }
}

bool BuildSnapshot(SnapshotSlot *slot, uint64_t generation, uint64_t sourceEpoch)
{
    SnapshotBuilder builder(slot->data, generation, sourceEpoch);
    builder.AddExact(0x0160f428, 0x118);
    builder.AddExact(0x0164d0a8, 0x20);
    builder.AddExact(0x0164d2cc, 0x44);
    builder.AddExact(0x0164d520, 0x18);
    builder.AddExact(SPELL_STATE_ADDRESS, SPELL_STATE_SIZE);
    builder.AddExact(0x017ce8e0, 4);
    builder.AddExact(ECL_CONTEXT_ADDRESS, 8);
    builder.AddExact(TIMELINE_RUNTIME_ADDRESS, 0x100);
    builder.AddExact(0x00f54e1c, 0x14);
    builder.AddExact(0x00f54cc0, 0x20);
    builder.AddExact(BULLET_TEMPLATE_ADDRESS, BULLET_TEMPLATE_SIZE);
    builder.AddExact(PLAYER_BASE, PLAYER_SNAPSHOT_SIZE);
    PackDynamicRootRanges(&builder);
    PackActivePools(&builder);
    PackIndexedEnemyFields(&builder);
    const uint32_t managerFrame = LoadU32(ENEMY_MANAGER_FRAME_ADDRESS);
    const uint32_t updateSerial = LoadU32(UPDATE_SERIAL_ADDRESS);
    const uint32_t size = builder.Finish(managerFrame, updateSerial);
    if (size == 0)
        return false;
    slot->generation = generation;
    slot->size = size;
    slot->entryCount = builder.entryCount;
    return true;
}

bool AcquireSnapshot(uint16_t *slotIndex)
{
    if ((LoadU32(ENGINE_FLAGS_ADDRESS) & GAMEPLAY_ACTIVE_FLAG) == 0)
        return false;
    for (uint16_t index = 0; index < SNAPSHOT_SLOT_COUNT; ++index)
    {
        SnapshotSlot *slot = &g_bridge.snapshots[index];
        if (slot->leased)
            continue;
        const uint64_t generation = ++g_bridge.nextSnapshotGeneration;
        if (!BuildSnapshot(slot, generation, g_bridge.inputEpoch))
        {
            ++g_bridge.droppedSnapshots;
            return false;
        }
        slot->leased = true;
        *slotIndex = index;
        return true;
    }
    ++g_bridge.droppedSnapshots;
    return false;
}

bool SetNonblocking(int file)
{
    const int flags = fcntl(file, F_GETFL, 0);
    return flags >= 0 && fcntl(file, F_SETFL, flags | O_NONBLOCK) == 0;
}

void CloseClient(const char *message)
{
    if (message != NULL)
        fprintf(stderr, "th08-modern: online solver client closed: %s\n", message);
    if (g_bridge.client >= 0)
    {
        close(g_bridge.client);
        g_bridge.client = -1;
    }
    ReleaseAllSnapshots();
}

void FailBridge(const char *message)
{
    if (!g_bridge.failed)
        fprintf(stderr, "th08-modern: online solver bridge failed: %s\n", message);
    g_bridge.failed = true;
    CloseClient(NULL);
    if (g_bridge.server >= 0)
    {
        close(g_bridge.server);
        g_bridge.server = -1;
    }
}

void InitializeBridge()
{
    if (g_bridge.initialized)
        return;
    g_bridge.initialized = true;

    const char *path = getenv("TH08_SOLVER_SOCKET");
    g_bridge.configured = path != NULL && path[0] != '\0';
    if (!g_bridge.configured)
        return;

    const char *preserveLives = getenv("TH08_SOLVER_PRESERVE_LIVES");
    if (preserveLives != NULL)
    {
        if (strcmp(preserveLives, "0") == 0)
            g_bridge.preserveLives = false;
        else if (strcmp(preserveLives, "1") != 0)
        {
            FailBridge("TH08_SOLVER_PRESERVE_LIVES must be 0 or 1");
            return;
        }
    }

    if (strlen(path) >= sizeof(sockaddr_un::sun_path))
    {
        FailBridge("TH08_SOLVER_SOCKET is too long");
        return;
    }

    if (!AllocateSnapshotStorage())
    {
        FailBridge("unable to allocate immutable snapshot slots");
        return;
    }

    g_bridge.server = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (g_bridge.server < 0 || !SetNonblocking(g_bridge.server))
    {
        FailBridge("unable to create non-blocking Unix packet socket");
        return;
    }

    struct sockaddr_un address;
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, path);
    if (bind(g_bridge.server, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) != 0)
    {
        FailBridge("unable to bind Unix socket; remove only the stale requested path");
        return;
    }
    if (listen(g_bridge.server, 1) != 0)
    {
        FailBridge("unable to listen on Unix socket");
        return;
    }

    // ReplayManager::AddedCallback will later copy these fields. This labels
    // the replay for the canonical target; it is not an ELF identity claim.
    g_Supervisor.exeSize = ORIGINAL_EXE_SIZE;
    g_Supervisor.exeChecksum = ORIGINAL_EXE_CHECKSUM;
    fprintf(stderr,
            "th08-modern: online solver bridge listening on %s (preserve lives: %s)\n",
            path, g_bridge.preserveLives ? "yes" : "no");
}

void TryAcceptClient()
{
    if (g_bridge.server < 0 || g_bridge.client >= 0)
        return;
    while (true)
    {
        const int client = accept(g_bridge.server, NULL, NULL);
        if (client >= 0)
        {
            if (!SetNonblocking(client))
            {
                close(client);
                FailBridge("unable to make solver client non-blocking");
                return;
            }
            g_bridge.client = client;
            ReleaseAllSnapshots();
            fprintf(stderr, "th08-modern: online solver bridge connected\n");
            return;
        }
        if (errno == EINTR)
            continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        FailBridge("unable to accept solver connection");
        return;
    }
}

bool InputMaskIsValid(uint16_t mask)
{
    if ((mask & BOMB_MASK) != 0 || (mask & ~KNOWN_INPUT_MASK) != 0)
        return false;
    if ((mask & UP_MASK) != 0 && (mask & DOWN_MASK) != 0)
        return false;
    if ((mask & LEFT_MASK) != 0 && (mask & RIGHT_MASK) != 0)
        return false;
    return true;
}

uint16_t HeldFallbackMask()
{
    const uint16_t held = g_CurFrameInput;
    return InputMaskIsValid(held) ? held : SAFE_NEUTRAL_MASK;
}

bool DrainResponses(uint64_t inputEpoch, uint16_t *inputMask)
{
    bool applied = false;
    while (g_bridge.client >= 0)
    {
        unsigned char response[RESPONSE_SIZE];
        struct iovec vector;
        vector.iov_base = response;
        vector.iov_len = sizeof(response);
        struct msghdr message;
        memset(&message, 0, sizeof(message));
        message.msg_iov = &vector;
        message.msg_iovlen = 1;
        const ssize_t count = recvmsg(
            g_bridge.client,
            &message,
            MSG_DONTWAIT);
        if (count < 0 && errno == EINTR)
            continue;
        if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            break;
        if (count == 0)
        {
            CloseClient("solver disconnected");
            break;
        }
        if ((message.msg_flags & MSG_TRUNC) != 0 || count < 8)
        {
            CloseClient("invalid response packet");
            break;
        }

        const uint32_t magic = GetU32(response, 0);
        if (magic == SNAPSHOT_RELEASE_MAGIC)
        {
            if (count != static_cast<ssize_t>(SNAPSHOT_RELEASE_SIZE) ||
                GetU16(response, 4) != PROTOCOL_VERSION ||
                GetU16(response, 6) != SNAPSHOT_RELEASE_SIZE ||
                GetU16(response, 18) != 0 || GetU32(response, 20) != 0)
            {
                CloseClient("invalid snapshot release packet");
                break;
            }
            const uint64_t generation = GetU64(response, 8);
            const uint16_t slotIndex = GetU16(response, 16);
            if (slotIndex >= SNAPSHOT_SLOT_COUNT ||
                !g_bridge.snapshots[slotIndex].leased ||
                g_bridge.snapshots[slotIndex].generation != generation)
            {
                CloseClient("snapshot release does not own its exact lease");
                break;
            }
            g_bridge.snapshots[slotIndex].leased = false;
            continue;
        }
        if (count != static_cast<ssize_t>(sizeof(response)) ||
            magic != RESPONSE_MAGIC ||
            GetU16(response, 4) != PROTOCOL_VERSION ||
            GetU16(response, 6) != RESPONSE_SIZE ||
            GetU16(response, 26) != 0 || GetU32(response, 28) != 0)
        {
            CloseClient("invalid response packet");
            break;
        }

        const uint64_t sourceEpoch = GetU64(response, 8);
        const uint64_t targetEpoch = GetU64(response, 16);
        const uint16_t mask = GetU16(response, 24);
        if (sourceEpoch + 1 != targetEpoch || !InputMaskIsValid(mask))
        {
            CloseClient("response contains an invalid epoch or input mask");
            break;
        }
        if (targetEpoch < inputEpoch)
        {
            ++g_bridge.lateResponses;
            continue;
        }
        if (targetEpoch > inputEpoch)
        {
            CloseClient("response targets an unrequested future epoch");
            break;
        }
        *inputMask = mask;
        applied = true;
    }
    return applied;
}

} // namespace

bool SolverBridgeReadInput(uint16_t *inputMask)
{
    if (inputMask == NULL)
        return false;
    InitializeBridge();
    if (!g_bridge.configured)
        return false;

    *inputMask = HeldFallbackMask();
    ++g_bridge.inputEpoch;
    th08_solver_input_epoch = static_cast<uint32_t>(g_bridge.inputEpoch);
    if (g_bridge.failed)
    {
        *inputMask = SAFE_NEUTRAL_MASK;
        return true;
    }

    TryAcceptClient();
    const bool applied = DrainResponses(g_bridge.inputEpoch, inputMask);
    if (g_bridge.client < 0)
        *inputMask = SAFE_NEUTRAL_MASK;
    if (!applied && g_bridge.inputEpoch > 1)
        ++g_bridge.deadlineMisses;
    return true;
}

void SolverBridgePublishSnapshot()
{
    InitializeBridge();
    if (!g_bridge.configured || g_bridge.failed || g_bridge.inputEpoch == 0 ||
        g_bridge.lastPublishedInputEpoch == g_bridge.inputEpoch)
        return;

    TryAcceptClient();
    if (g_bridge.client < 0)
        return;

    uint16_t snapshotSlot = NO_SNAPSHOT_SLOT;
    const bool snapshotPresent = AcquireSnapshot(&snapshotSlot);
    SnapshotSlot *snapshot =
        snapshotPresent ? &g_bridge.snapshots[snapshotSlot] : NULL;

    unsigned char request[REQUEST_SIZE];
    memset(request, 0, sizeof(request));
    PutU32(request, 0, REQUEST_MAGIC);
    PutU16(request, 4, PROTOCOL_VERSION);
    PutU16(request, 6, REQUEST_SIZE);
    PutU64(request, 8, g_bridge.inputEpoch);
    PutU64(request, 16, g_bridge.inputEpoch + 1);
    PutU16(request, 24, g_CurFrameInput);
    PutU16(request, 26, g_LastFrameInput);
    PutU16(request, 28, g_Rng.GetSeed());
    PutU16(request, 30, snapshotSlot);
    uint32_t requestFlags = REQUEST_FLAG_REPLAY_TARGET_STAMPED;
    if (g_bridge.preserveLives)
        requestFlags |= REQUEST_FLAG_LIVES_PRESERVED;
    if (snapshotPresent)
        requestFlags |= REQUEST_FLAG_IMMUTABLE_SNAPSHOT_PRESENT;
    PutU32(request, 32, requestFlags);
    PutU32(request, 36, SaturatingU32(g_bridge.deadlineMisses));
    PutU32(request, 40, SaturatingU32(g_bridge.lateResponses));
    PutU32(request, 44, SaturatingU32(g_bridge.droppedRequests));
    PutU64(request, 48, MonotonicMicroseconds());
    if (snapshotPresent)
    {
        PutU64(request, 56, snapshot->generation);
        PutU32(
            request,
            64,
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(snapshot->data)));
        PutU32(request, 68, snapshot->size);
        PutU32(request, 72, snapshot->entryCount);
    }
    PutU32(request, 76, SaturatingU32(g_bridge.droppedSnapshots));

    g_bridge.lastPublishedInputEpoch = g_bridge.inputEpoch;
    const ssize_t written = send(
        g_bridge.client,
        request,
        sizeof(request),
        MSG_DONTWAIT | MSG_NOSIGNAL);
    if (written == static_cast<ssize_t>(sizeof(request)))
        return;
    if (written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    {
        if (snapshotPresent)
            snapshot->leased = false;
        ++g_bridge.droppedRequests;
        return;
    }
    if (snapshotPresent)
        snapshot->leased = false;
    CloseClient("unable to publish complete snapshot packet");
}

bool SolverBridgePreserveLives()
{
    InitializeBridge();
    return g_bridge.configured && g_bridge.preserveLives;
}

} // namespace modern
} // namespace th08
