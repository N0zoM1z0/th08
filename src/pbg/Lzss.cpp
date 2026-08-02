#include "th_pch.h"

#include "pbg/Lzss.hpp"
#include "pbg/PbgMemory.hpp"

#define LZSS_BREAKEVEN 3
#define LZSS_LOOKAHEAD_MAX ((1 << LZSS_LENGTH_BITS) + LZSS_BREAKEVEN - 1)
#define LZSS_DICTSIZE_MASK (LZSS_DICTSIZE - 1)
#define LZSS_DICTPOS_MOD(pos, amount) ((pos + amount) & LZSS_DICTSIZE_MASK)

namespace th08
{
Lzss::TreeNode Lzss::m_Tree[LZSS_DICTSIZE + 1];
u8 Lzss::m_Dict[LZSS_DICTSIZE];

/**
 * \brief Advance the write head forward by one bit.
 */
#define ENCODE_ADVANCE_WRITE_HEAD                                                                                      \
    outBitMask >>= 1;                                                                                                  \
    if (outBitMask == 0)                                                                                               \
    {                                                                                                                  \
        *outCursor++ = outBits;                                                                                        \
        checksum += outBits;                                                                                           \
        outBits = 0;                                                                                                   \
        outBitMask = 0x80;                                                                                             \
    }

/**
 * \brief Pack and write a single bit to the output buffer.
 * \param bit Bit value to write (1 or 0)
 */
#define ENCODE_PACK_BIT(bit)                                                                                           \
    if (bit)                                                                                                           \
    {                                                                                                                  \
        outBits |= outBitMask;                                                                                         \
    }                                                                                                                  \
    ENCODE_ADVANCE_WRITE_HEAD;

/**
 * \brief Pack and write a sequence of bits to the output buffer.
 * \param bitCount Number of bits to write
 * \param writeOneIf A conditional expression evaluated for each bit that will evaluate to true if that bit should be 1
 */
#define ENCODE_PACK_BITS(bitCount, writeOneIf)                                                                         \
    bitfieldMask = 0x1 << (bitCount - 1);                                                                              \
    while (bitfieldMask != 0)                                                                                          \
    {                                                                                                                  \
        if (writeOneIf)                                                                                                \
            outBits |= outBitMask;                                                                                     \
        ENCODE_ADVANCE_WRITE_HEAD;                                                                                     \
        bitfieldMask >>= 1;                                                                                            \
    }

#pragma var_order(outBits, out, outCursor, matchOffset, i, bytesToCopyToDict, outBitMask, inCursor, matchLength,       \
                  checksum, maxMatchLength, dictValue, dictHead, bitfieldMask)
/**
 * \brief Compresses data using LZSS and writes the output to a buffer.
 * \param in Input buffer
 * \param inSize Size of input data in bytes
 * \param out Output buffer (if NULL, a buffer will allocated automatically)
 */
u8 *Lzss::Encode(u8 *in, i32 inSize, i32 *outSize)
{
    u8 outBitMask;
    u32 outBits;
    // NOTE: For some reason, this value is discarded after encoding is complete instead of being returned
    u32 checksum;
    u8 *out;
    u8 *inCursor, *outCursor;
    u32 dictHead;
    i32 i;
    i32 maxMatchLength;
    i32 matchLength;
    i32 matchOffset;
    i32 bytesToCopyToDict;
    i32 dictValue;
    u32 bitfieldMask;

    outBitMask = 0x80;
    outBits = 0;
    checksum = 0;

    out = (LPBYTE)MemAlloc(inSize * 2);
    if (out == NULL)
    {
        return NULL;
    }

    inCursor = in;
    outCursor = out;
    *outSize = 0;
    InitEncoderState();
    dictHead = 1;

    for (i = 0; i < LZSS_LOOKAHEAD_MAX; i++)
    {
        // If past the end of the input data
        if (inCursor - in >= inSize)
        {
            // Signal value to mark end of input data
            dictValue = -1;
        }
        else
        {
            dictValue = *inCursor++;
        }

        // If past the end of the input data
        if (dictValue == -1)
        {
            break;
        }

        m_Dict[dictHead + i] = dictValue;
    }

    maxMatchLength = i;
    InitTree(dictHead);

    matchLength = 0;
    matchOffset = 0;

    while (maxMatchLength > 0)
    {
        // Ensure match length does not go past the end of the input data
        if (matchLength > maxMatchLength)
        {
            matchLength = maxMatchLength;
        }

        // If current match length does not at least break even, encode byte literal
        if (matchLength <= LZSS_BREAKEVEN - 1)
        {
            bytesToCopyToDict = 1;

            ENCODE_PACK_BIT(1);
            ENCODE_PACK_BITS(8, (bitfieldMask & m_Dict[dictHead]) != 0);
        }
        // Otherwise, encode length/offset pair
        else
        {
            ENCODE_PACK_BIT(0);
            ENCODE_PACK_BITS(LZSS_OFFSET_BITS, (bitfieldMask & matchOffset) != 0);
            ENCODE_PACK_BITS(LZSS_LENGTH_BITS, (bitfieldMask & (matchLength - LZSS_BREAKEVEN)) != 0);

            bytesToCopyToDict = matchLength;
        }

        // Copy data to dictionary
        for (i = 0; i < bytesToCopyToDict; i++)
        {
            DeleteString(LZSS_DICTPOS_MOD(dictHead, LZSS_LOOKAHEAD_MAX));

            // If past the end of the input data
            if (inCursor - in >= inSize)
            {
                // Signal value to mark end of input data
                dictValue = -1;
            }
            else
            {
                dictValue = *inCursor++;
            }

            // If past the end of the input data, decrement maximum match length
            if (dictValue == -1)
            {
                maxMatchLength--;
            }
            // Else, copy previous byte in input data to the dictionary
            else
            {
                m_Dict[LZSS_DICTPOS_MOD(dictHead, LZSS_LOOKAHEAD_MAX)] = dictValue;
            }

            dictHead = LZSS_DICTPOS_MOD(dictHead, 1);

            // If input data not fully encoded
            if (maxMatchLength != 0)
            {
                matchLength = AddString(dictHead, &matchOffset);
            }
        }
    }

    ENCODE_PACK_BIT(0);
    ENCODE_PACK_BITS(LZSS_OFFSET_BITS, false);

    *outSize = outCursor - out;
    return out;

#undef ENCODE_ADVANCE_WRITE_HEAD
#undef ENCODE_PACK_BIT
#undef ENCODE_PACK_BITS
}

/**
 * \brief Advance the read head forward by one bit.
 */
#define DECODE_ADVANCE_READ_HEAD                                                                                       \
    inBitMask >>= 1;                                                                                                   \
    if (inBitMask == 0)                                                                                                \
    {                                                                                                                  \
        inBitMask = 0x80;                                                                                              \
    }

/**
 * \brief Write a byte to the output buffer.
 */
#define DECODE_WRITE_BYTE(data)                                                                                        \
    *outCursor++ = data;                                                                                               \
    m_Dict[dictHead] = data;                                                                                           \
    dictHead = LZSS_DICTPOS_MOD(dictHead, 1);

/**
 * \brief Fetch a new byte from the input buffer if the current byte has been fully decoded.
 */
#define DECODE_HANDLE_FETCH                                                                                            \
    if (inBitMask == 0x80)                                                                                             \
    {                                                                                                                  \
        currByte = *inCursor;                                                                                          \
        if (inCursor - in >= size)                                                                                     \
        {                                                                                                              \
            currByte = 0;                                                                                              \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            inCursor++;                                                                                                \
        }                                                                                                              \
        checksum += currByte;                                                                                          \
    }

/**
 * \brief Read and unpack a single bit from the input buffer.
 */
#define DECODE_UNPACK_BIT                                                                                              \
    DECODE_HANDLE_FETCH;                                                                                               \
    inBits = currByte & inBitMask;                                                                                     \
    DECODE_ADVANCE_READ_HEAD;

/**
 * \brief Read and unpack a sequence of bits from the input buffer.
 * \param bitCount Number of bits to read
 */
#define DECODE_UNPACK_BITS(bitCount)                                                                                   \
    outBitMask = 0x1 << (bitCount - 1);                                                                                \
    inBits = 0;                                                                                                        \
    while (outBitMask != 0)                                                                                            \
    {                                                                                                                  \
        DECODE_HANDLE_FETCH;                                                                                           \
        if ((currByte & inBitMask) != 0)                                                                               \
            inBits |= outBitMask;                                                                                      \
        outBitMask >>= 1;                                                                                              \
        DECODE_ADVANCE_READ_HEAD;                                                                                      \
    }

#pragma var_order(currByte, outCursor, matchOffset, i, inBitMask, inCursor, inBits, size, matchLength, checksum,       \
                  dictValue, outBitMask, dictHead)
/**
 * \brief Decodes LZSS-compressed data to a buffer.
 * \param in Input buffer
 * \param inSize Size of input data in bytes
 * \param out Output buffer (if NULL, a buffer will allocated automatically)
 * \param outSize Number of bytes to allocate for output buffer
 */
u8 *Lzss::Decode(u8 *in, i32 inSize, u8 *out, i32 outSize)
{
    u8 inBitMask;
    u32 currByte;
    // NOTE: For some reason, this value is discarded after decoding is complete instead of being returned
    u32 checksum;
    i32 size;
    u8 *inCursor, *outCursor;
    u32 dictHead;
    u32 inBits;
    i32 matchOffset;
    i32 matchLength;
    i32 i;
    u32 dictValue;
    u32 outBitMask;

    inBitMask = 0x80;
    currByte = 0;
    checksum = 0;
    size = inSize;

    if (out == NULL)
    {
        out = (u8 *)MemAlloc(outSize);
        if (out == NULL)
        {
            return NULL;
        }
    }

    inCursor = in;
    outCursor = out;
    dictHead = 1;

    for (;;)
    {
        // Read flag bit
        DECODE_UNPACK_BIT;

        // Read literal byte from next 8 bits
        if (inBits != 0)
        {
            DECODE_UNPACK_BITS(8);
            DECODE_WRITE_BYTE(inBits);
        }
        // Copy from dictionary, 13 bit offset, then 4 bit length
        else
        {
            DECODE_UNPACK_BITS(13);

            matchOffset = inBits;
            if (matchOffset == 0)
            {
                break;
            }

            DECODE_UNPACK_BITS(4);

            // Value encoded in 4 bit length is 3 less than the actual length
            matchLength = inBits + 2;
            for (i = 0; i <= matchLength; i++)
            {
                dictValue = m_Dict[LZSS_DICTPOS_MOD(matchOffset, i)];
                DECODE_WRITE_BYTE(dictValue);
            }
        }
    }

    // Read any trailing bits in the data
    while (inBitMask != 0x80)
    {
        DECODE_UNPACK_BIT;
    }

    return out;

#undef DECODE_ADVANCE_READ_HEAD
#undef DECODE_WRITE_BYTE
#undef DECODE_HANDLE_FETCH
#undef DECODE_UNPACK_BIT
#undef DECODE_UNPACK_BITS
}

void Lzss::InitTree(i32 root)
{
    m_Tree[LZSS_DICTSIZE].right = root;
    m_Tree[root].parent = LZSS_DICTSIZE;
    m_Tree[root].right = 0;
    m_Tree[root].left = 0;
}

void Lzss::InitEncoderState()
{
    i32 i;

    for (i = 0; i < LZSS_DICTSIZE; i++)
    {
        m_Dict[i] = 0;
    }
    for (i = 0; i < LZSS_DICTSIZE + 1; i++)
    {
        m_Tree[i].parent = 0;
        m_Tree[i].left = 0;
        m_Tree[i].right = 0;
    }
}

#pragma var_order(i, child, testNode, matchLength, delta)

i32 Lzss::AddString(i32 newNode, i32 *matchPosition)
{
    i32 i;
    i32 *child;
    i32 delta;

    if (newNode == 0)
        return 0;

    i32 testNode = m_Tree[LZSS_DICTSIZE].right;
    i32 matchLength = 0;

    for (;;)
    {
        for (i = 0; i < LZSS_LOOKAHEAD_MAX; i++)
        {
            delta = m_Dict[LZSS_DICTPOS_MOD(newNode, i)] - m_Dict[LZSS_DICTPOS_MOD(testNode, i)];

            if (delta != 0)
            {
                break;
            }
        }

        if (i >= matchLength)
        {
            matchLength = i;
            *matchPosition = testNode;

            if (matchLength >= LZSS_LOOKAHEAD_MAX)
            {
                ReplaceNode(testNode, newNode);
                return matchLength;
            }
        }

        if (delta >= 0)
        {
            child = &m_Tree[testNode].right;
        }
        else
        {
            child = &m_Tree[testNode].left;
        }

        if (*child == 0)
        {
            *child = newNode;
            m_Tree[newNode].parent = testNode;
            m_Tree[newNode].right = 0;
            m_Tree[newNode].left = 0;
            return matchLength;
        }

        testNode = *child;
    }
}

void Lzss::DeleteString(i32 p)
{
    if (m_Tree[p].parent == 0)
    {
        return;
    }

    if (m_Tree[p].right == 0)
    {
        ContractNode(p, m_Tree[p].left);
    }
    else if (m_Tree[p].left == 0)
    {
        ContractNode(p, m_Tree[p].right);
    }
    else
    {
        i32 replacement = FindNextNode(p);
        DeleteString(replacement);
        ReplaceNode(p, replacement);
    }
}

void Lzss::ContractNode(i32 oldNode, i32 newNode)
{
    m_Tree[newNode].parent = m_Tree[oldNode].parent;

    if (m_Tree[m_Tree[oldNode].parent].right == oldNode)
    {
        m_Tree[m_Tree[oldNode].parent].right = newNode;
    }
    else
    {
        m_Tree[m_Tree[oldNode].parent].left = newNode;
    }
    m_Tree[oldNode].parent = 0;
}

void Lzss::ReplaceNode(i32 oldNode, i32 newNode)
{
    i32 parent = m_Tree[oldNode].parent;

    if (m_Tree[parent].left == oldNode)
    {
        m_Tree[parent].left = newNode;
    }
    else
    {
        m_Tree[parent].right = newNode;
    }
    m_Tree[newNode] = m_Tree[oldNode];
    m_Tree[m_Tree[newNode].left].parent = newNode;
    m_Tree[m_Tree[newNode].right].parent = newNode;
    m_Tree[oldNode].parent = 0;
}

i32 Lzss::FindNextNode(i32 node)
{
    i32 next = m_Tree[node].left;

    while (m_Tree[next].right != 0)
    {
        next = m_Tree[next].right;
    }
    return next;
}
}; // namespace th08
