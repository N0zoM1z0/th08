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
const uint16_t PROTOCOL_VERSION = 2;
const size_t REQUEST_SIZE = 56;
const size_t RESPONSE_SIZE = 32;
const uint16_t BOMB_MASK = 0x0002;
const uint16_t KNOWN_INPUT_MASK = 0x7ffd;
const uint16_t UP_MASK = 0x0010;
const uint16_t DOWN_MASK = 0x0020;
const uint16_t LEFT_MASK = 0x0040;
const uint16_t RIGHT_MASK = 0x0080;
const uint16_t SAFE_NEUTRAL_MASK = 0x0005; // Shot + Focus.
const uint32_t REQUEST_FLAG_REPLAY_TARGET_STAMPED = 1;
const uint32_t REQUEST_FLAG_LIVES_PRESERVED = 2;
const uint32_t ORIGINAL_EXE_SIZE = 840704;
const uint32_t ORIGINAL_EXE_CHECKSUM = 2724749753U;

struct BridgeState
{
    BridgeState()
        : configured(false), initialized(false), failed(false), server(-1),
          client(-1), inputEpoch(0), lastPublishedInputEpoch(0),
          deadlineMisses(0), lateResponses(0), droppedRequests(0),
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
    bool preserveLives;
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
        if ((message.msg_flags & MSG_TRUNC) != 0 ||
            count != static_cast<ssize_t>(sizeof(response)) ||
            GetU32(response, 0) != RESPONSE_MAGIC ||
            GetU16(response, 4) != PROTOCOL_VERSION ||
            GetU16(response, 6) != RESPONSE_SIZE ||
            GetU16(response, 26) != 0 ||
            GetU32(response, 28) != 0)
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
    uint32_t requestFlags = REQUEST_FLAG_REPLAY_TARGET_STAMPED;
    if (g_bridge.preserveLives)
        requestFlags |= REQUEST_FLAG_LIVES_PRESERVED;
    PutU32(request, 32, requestFlags);
    PutU32(request, 36, SaturatingU32(g_bridge.deadlineMisses));
    PutU32(request, 40, SaturatingU32(g_bridge.lateResponses));
    PutU32(request, 44, SaturatingU32(g_bridge.droppedRequests));
    PutU64(request, 48, MonotonicMicroseconds());

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
        ++g_bridge.droppedRequests;
        return;
    }
    CloseClient("unable to publish complete snapshot packet");
}

bool SolverBridgePreserveLives()
{
    InitializeBridge();
    return g_bridge.configured && g_bridge.preserveLives;
}

} // namespace modern
} // namespace th08
