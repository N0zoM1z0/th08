#include "solver_bridge.hpp"

#include "Global.hpp"
#include "Supervisor.hpp"

#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

namespace th08
{
namespace modern
{
namespace
{

const uint32_t REQUEST_MAGIC = 0x51523854;  // "T8RQ" as little endian.
const uint32_t RESPONSE_MAGIC = 0x53523854; // "T8RS" as little endian.
const uint16_t PROTOCOL_VERSION = 1;
const size_t REQUEST_SIZE = 32;
const size_t RESPONSE_SIZE = 24;
const uint16_t BOMB_MASK = 0x0002;
const uint16_t KNOWN_INPUT_MASK = 0x7ffd;
const uint16_t UP_MASK = 0x0010;
const uint16_t DOWN_MASK = 0x0020;
const uint16_t LEFT_MASK = 0x0040;
const uint16_t RIGHT_MASK = 0x0080;
const uint32_t REQUEST_FLAG_REPLAY_TARGET_STAMPED = 1;
const uint32_t ORIGINAL_EXE_SIZE = 840704;
const uint32_t ORIGINAL_EXE_CHECKSUM = 2724749753U;

struct BridgeState
{
    BridgeState()
        : configured(false), initialized(false), failed(false), server(-1),
          client(-1), epoch(0)
    {
    }

    bool configured;
    bool initialized;
    bool failed;
    int server;
    int client;
    uint64_t epoch;
};

BridgeState g_bridge;
pthread_mutex_t g_clockMutex = PTHREAD_MUTEX_INITIALIZER;
uint64_t g_totalPausedMicroseconds;
uint64_t g_pauseStartedMicroseconds;
unsigned int g_pauseDepth;

uint64_t RealMicroseconds()
{
    struct timeval value;
    gettimeofday(&value, NULL);
    return static_cast<uint64_t>(value.tv_sec) * 1000000ULL +
           static_cast<uint64_t>(value.tv_usec);
}

void BeginSolverWait()
{
    const uint64_t now = RealMicroseconds();
    pthread_mutex_lock(&g_clockMutex);
    if (g_pauseDepth++ == 0)
        g_pauseStartedMicroseconds = now;
    pthread_mutex_unlock(&g_clockMutex);
}

void EndSolverWait()
{
    const uint64_t now = RealMicroseconds();
    pthread_mutex_lock(&g_clockMutex);
    if (g_pauseDepth != 0 && --g_pauseDepth == 0)
    {
        if (now >= g_pauseStartedMicroseconds)
            g_totalPausedMicroseconds += now - g_pauseStartedMicroseconds;
        g_pauseStartedMicroseconds = 0;
    }
    pthread_mutex_unlock(&g_clockMutex);
}

uint32_t PausedMilliseconds()
{
    const uint64_t now = RealMicroseconds();
    pthread_mutex_lock(&g_clockMutex);
    uint64_t paused = g_totalPausedMicroseconds;
    if (g_pauseDepth != 0 && now >= g_pauseStartedMicroseconds)
        paused += now - g_pauseStartedMicroseconds;
    pthread_mutex_unlock(&g_clockMutex);
    paused /= 1000;
    return paused > 0xffffffffULL ? 0xffffffffU : static_cast<uint32_t>(paused);
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

bool WriteAll(int file, const unsigned char *data, size_t size)
{
    while (size != 0)
    {
        const ssize_t written = send(file, data, size, MSG_NOSIGNAL);
        if (written < 0 && errno == EINTR)
            continue;
        if (written <= 0)
            return false;
        data += written;
        size -= static_cast<size_t>(written);
    }
    return true;
}

bool ReadAll(int file, unsigned char *data, size_t size)
{
    while (size != 0)
    {
        const ssize_t count = recv(file, data, size, 0);
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0)
            return false;
        data += count;
        size -= static_cast<size_t>(count);
    }
    return true;
}

void FailBridge(const char *message)
{
    if (!g_bridge.failed)
        fprintf(stderr, "th08-modern: solver bridge failed: %s\n", message);
    g_bridge.failed = true;
    if (g_bridge.client >= 0)
    {
        close(g_bridge.client);
        g_bridge.client = -1;
    }
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

    if (strlen(path) >= sizeof(sockaddr_un::sun_path))
    {
        FailBridge("TH08_SOLVER_SOCKET is too long");
        return;
    }

    g_bridge.server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (g_bridge.server < 0)
    {
        FailBridge("unable to create Unix socket");
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

    // ReplayManager::AddedCallback will later copy these fields.  This labels
    // the replay for the canonical target; it is not an ELF identity claim.
    g_Supervisor.exeSize = ORIGINAL_EXE_SIZE;
    g_Supervisor.exeChecksum = ORIGINAL_EXE_CHECKSUM;
    fprintf(stderr, "th08-modern: solver bridge listening on %s\n", path);
}

bool EnsureClient()
{
    if (g_bridge.client >= 0)
        return true;
    g_bridge.client = accept(g_bridge.server, NULL, NULL);
    if (g_bridge.client < 0)
    {
        FailBridge("unable to accept solver connection");
        return false;
    }
    fprintf(stderr, "th08-modern: solver bridge connected\n");
    return true;
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

bool ExchangeInput(uint16_t *inputMask)
{
    if (!EnsureClient())
        return false;

    unsigned char request[REQUEST_SIZE];
    memset(request, 0, sizeof(request));
    PutU32(request, 0, REQUEST_MAGIC);
    PutU16(request, 4, PROTOCOL_VERSION);
    PutU16(request, 6, REQUEST_SIZE);
    PutU64(request, 8, ++g_bridge.epoch);
    PutU16(request, 16, g_CurFrameInput);
    PutU16(request, 18, g_LastFrameInput);
    PutU16(request, 20, g_Rng.GetSeed());
    PutU32(request, 24, REQUEST_FLAG_REPLAY_TARGET_STAMPED);
    PutU32(request, 28, PausedMilliseconds());

    unsigned char response[RESPONSE_SIZE];
    if (!WriteAll(g_bridge.client, request, sizeof(request)) ||
        !ReadAll(g_bridge.client, response, sizeof(response)))
    {
        FailBridge("solver disconnected during an input transaction");
        return false;
    }

    if (GetU32(response, 0) != RESPONSE_MAGIC ||
        GetU16(response, 4) != PROTOCOL_VERSION ||
        GetU16(response, 6) != RESPONSE_SIZE ||
        GetU64(response, 8) != g_bridge.epoch)
    {
        FailBridge("invalid or stale solver response");
        return false;
    }

    const uint16_t mask = GetU16(response, 16);
    if (!InputMaskIsValid(mask))
    {
        FailBridge("response contains Bomb, unknown, or contradictory input");
        return false;
    }
    *inputMask = mask;
    return true;
}

} // namespace

bool SolverBridgeReadInput(uint16_t *inputMask)
{
    if (inputMask == NULL)
        return false;
    *inputMask = 0;
    InitializeBridge();
    if (!g_bridge.configured)
        return false;
    if (g_bridge.failed)
        return true;

    BeginSolverWait();
    const bool exchanged = ExchangeInput(inputMask);
    EndSolverWait();
    if (!exchanged)
        *inputMask = 0;
    return true;
}

bool SolverBridgePreserveLives()
{
    InitializeBridge();
    return g_bridge.configured;
}

uint64_t SolverBridgeVirtualMicroseconds(uint64_t realMicroseconds)
{
    pthread_mutex_lock(&g_clockMutex);
    uint64_t paused = g_totalPausedMicroseconds;
    if (g_pauseDepth != 0 && realMicroseconds >= g_pauseStartedMicroseconds)
        paused += realMicroseconds - g_pauseStartedMicroseconds;
    pthread_mutex_unlock(&g_clockMutex);
    return realMicroseconds >= paused ? realMicroseconds - paused : 0;
}

} // namespace modern
} // namespace th08
