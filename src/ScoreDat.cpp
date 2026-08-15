#include "th_pch.h"

#include "GameManager.hpp"
#include "ScoreDat.hpp"
#include "SpellCard.hpp"
#include "pbg/Lzss.hpp"

namespace th08
{

// FUNCTION: th08 0x45a4a0
ZunBool Catk::WasAttemptedWithShot(i32 shotType)
{
    ZunBool result;

    if (this->inGameHistory.attempts[shotType] > 0)
    {
        result = TRUE;
    }
    else if (this->spellPracticeHistory.attempts[shotType] != 0)
    {
        result = TRUE;
    }
    else
    {
        result = FALSE;
    }

    return result;
}

i32 ScoreDat::LinkScore(ScoreListNode *node, Hscr *newScore)
{
    i32 i;
    ScoreListNode *next;

    i = 0;

    while (node->next != NULL)
    {
        if (node->next->data != NULL)
        {
            if (node->next->data->score <= newScore->score)
            {
                break;
            }
        }
        node = node->next;
        i++;
    }

    next = node->next;
    node->next = (ScoreListNode *)g_ZunMemory.Alloc(sizeof(ScoreListNode), "result");
    node->next->prev = node;

    node = node->next;
    node->data = newScore;
    node->next = next;

    return i;
}

void ScoreDat::FreeAllScores(ScoreListNode *scores)
{
    ScoreListNode *next;

    scores = scores->next;
    while (scores != NULL)
    {
        next = scores->next;
        g_ZunMemory.Free(scores);
        scores = next;
    }
}

#pragma var_order(scoreDat2, fileSize, scoreDat, scoreDecrypted, bytesShifted, xorValue, checksum, bytes,              \
                  bytesToShift, chapter, bytesToRead, th8kChapter, hasFoundTH8K, vrsm)
ScoreDat *ScoreDat::OpenScore(const char *filename)
{
    ScoreDat *scoreDat2;
    ScoreDat *scoreDat;
    u32 fileSize;
    ScoreDat *scoreDecrypted;
    u16 checksum;
    u8 xorValue;
    i32 bytesShifted;
    i32 bytesToShift;
    u8 *bytes;
    i32 bytesToRead;
    Th8k *chapter;
    Th8k *th8kChapter;
    ZunBool hasFoundTH8K;
    Vrsm *vrsm;

    utils::DebugPrint("info : score load\r\n");

    scoreDat = (ScoreDat *)FileSystem::OpenFile(filename, (i32 *)&fileSize, TRUE);

    if (scoreDat == NULL)
    {
    recreate_score_file:
        utils::DebugPrint("info : score recreate\r\n");
        if (scoreDat != NULL)
        {
            g_ZunMemory.Free(scoreDat);
        }
        scoreDat = (ScoreDat *)g_ZunMemory.Alloc(sizeof(ScoreDat), "scorefile");
        scoreDat->headerSize = sizeof(ScoreDat);
        scoreDat->decompressedFileSize = sizeof(ScoreDat);
        goto out;
    }

    if (fileSize < sizeof(ScoreDat))
    {
        utils::DebugPrint("warning : score.dat size is short\r\n");
        g_ZunMemory.Free(scoreDat);
        goto recreate_score_file;
    }

    scoreDecrypted =
        (ScoreDat *)FileSystem::Decrypt((u8 *)scoreDat, fileSize, SCORE_DAT_XOR_VALUE, SCORE_DAT_XOR_VALUE_INCREMENT,
                                        SCORE_DAT_CHUNK_SIZE, SCORE_DAT_MAX_BYTES);
    g_ZunMemory.Free(scoreDat);
    scoreDat = scoreDecrypted;
    bytesToShift = fileSize - 2;

    checksum = 0;
    xorValue = 0;
    bytesShifted = 0;

    bytes = (u8 *)scoreDat + 1;

    while (bytesToShift > 0)
    {
        xorValue += *bytes;
        xorValue = (xorValue & 0xe0) >> 5 | (xorValue & 0x1f) << 3;
        bytes[1] ^= xorValue;

        if (bytesShifted >= 2)
        {
            checksum += bytes[1];
        }

        bytes++;
        bytesToShift--;
        bytesShifted++;
    }

    if (scoreDat->checksum != checksum)
    {
        utils::DebugPrint("warning : score.dat chksum error\r\n");
        goto recreate_score_file;
    }

    if (scoreDat->headerSize != sizeof(ScoreDat))
    {
        utils::DebugPrint("warning : header size is mismatch\r\n");
        goto recreate_score_file;
    }

    if (scoreDat->version != SCORE_DAT_VERSION)
    {
        utils::DebugPrint("warning : score.dat version mismatch\r\n");
        goto recreate_score_file;
    }

    scoreDat2 = (ScoreDat *)g_ZunMemory.Alloc(sizeof(ScoreDat) + 0xa0000, "scorefile2");
    memcpy(scoreDat2, scoreDat, sizeof(ScoreDat));
    Lzss::Decode((u8 *)(scoreDat + 1), scoreDat->compressedFileSize, (u8 *)(scoreDat2 + 1),
                 scoreDat->decompressedFileSizeMinusHeader);
    g_ZunMemory.Free(scoreDat);
    scoreDat = scoreDat2;

    bytesToRead = scoreDat->decompressedFileSize;
    hasFoundTH8K = FALSE;
    chapter = (Th8k *)(((u8 *)scoreDat) + scoreDat->headerSize);
    bytesToRead -= scoreDat->headerSize;

    while (bytesToRead > 0)
    {
        if (chapter->magic == TH8K_MAGIC)
        {
            hasFoundTH8K = TRUE;
            th8kChapter = chapter;
        }
        if (chapter->magic == VRSM_MAGIC && chapter->version == VRSM_VERSION)
        {
            vrsm = (Vrsm *)chapter;
            if (g_Supervisor.CheckVersion(vrsm->version, vrsm->exeSize, vrsm->exeChecksum) != ZUN_SUCCESS)
            {
                utils::DebugPrint("warning : score.dat exesumcheck error\r\n");
                goto recreate_score_file;
            }
        }
        if (chapter->th8kLen == 0)
        {
            utils::DebugPrint("warning : score.dat chapter size is ZERO\r\n");
            goto recreate_score_file;
        }
        bytesToRead -= chapter->th8kLen;
        chapter = (Th8k *)(((u8 *)chapter) + chapter->th8kLen);
    }

    if (!hasFoundTH8K || th8kChapter->version != 1)
    {
        utils::DebugPrint("warning : score.dat version mismatch\r\n");
        goto recreate_score_file;
    }

out:
    scoreDat->scores = (ScoreListNode *)g_ZunMemory.Alloc(sizeof(ScoreListNode), "result");
    scoreDat->scores->next = NULL;
    scoreDat->scores->data = NULL;
    scoreDat->scores->prev = NULL;

    return scoreDat;
}

#pragma var_order(hscr, bytesToRead, scoreDat2)
u32 ScoreDat::GetHighScore(ScoreDat *scoreDat, ScoreListNode *node, u32 character, u32 difficulty, u8 *continuesUsed)
{
    Hscr *hscr;
    i32 bytesToRead;
    ScoreDat *scoreDat2 = scoreDat;

    if (node == NULL)
    {
        ScoreDat::FreeAllScores(scoreDat2->scores);
        scoreDat2->scores->next = NULL;
        scoreDat2->scores->data = NULL;
        scoreDat2->scores->prev = NULL;
    }

    bytesToRead = scoreDat2->decompressedFileSize;
    hscr = (Hscr *)(((u8 *)scoreDat2) + scoreDat2->headerSize);
    bytesToRead -= scoreDat2->headerSize;

    while (bytesToRead > 0)
    {
        if (hscr->base.magic == HSCR_MAGIC && hscr->base.version == HSCR_VERSION && hscr->character == character &&
            hscr->difficulty == difficulty)
        {
            if (node != NULL)
            {
                ScoreDat::LinkScore(node, hscr);
            }
            else
            {
                ScoreDat::LinkScore(scoreDat2->scores, hscr);
            }
        }

        bytesToRead -= hscr->base.th8kLen;
        hscr = (Hscr *)((u8 *)hscr + hscr->base.th8kLen);
    }

    if (continuesUsed != NULL)
    {
        *continuesUsed = scoreDat2->scores->next != NULL ? scoreDat2->scores->next->data->numRetries : 0;
    }

    return scoreDat2->scores->next != NULL ? max(scoreDat2->scores->next->data->score, 100000) : 100000;
}

#pragma var_order(catk, bytesToRead, scoreDat2)
i32 ScoreDat::ParseCATK(ScoreDat *scoreDat, Catk *outCatk)
{
    Catk *catk;
    i32 bytesToRead;
    /* ??? maybe the parameter is `void *`? */
    ScoreDat *scoreDat2 = scoreDat;

    if (outCatk == NULL)
    {
        return ZUN_ERROR;
    }

    catk = (Catk *)((u8 *)scoreDat2 + scoreDat2->headerSize);
    bytesToRead = scoreDat2->decompressedFileSize - scoreDat2->headerSize;

    while (bytesToRead > 0)
    {
        if (catk->base.magic == CATK_MAGIC && catk->base.version == CATK_VERSION)
        {
            if (catk->spellcardNumber >= SPELLCARD_COUNT_SPELLCARDS)
            {
                break;
            }

            outCatk[catk->spellcardNumber] = *catk;
        }

        bytesToRead -= catk->base.th8kLen;
        catk = (Catk *)((u8 *)catk + catk->base.th8kLen);
    }

    return ZUN_SUCCESS;
}

#pragma var_order(lsnm, bytesToRead, scoreDat2)
i32 ScoreDat::ParseLSNM(ScoreDat *scoreDat, Lsnm *outLsnm)
{
    Lsnm *lsnm;
    i32 bytesToRead;
    ScoreDat *scoreDat2 = scoreDat;

    lsnm = (Lsnm *)((u8 *)scoreDat2 + scoreDat2->headerSize);
    bytesToRead = scoreDat2->decompressedFileSize - scoreDat2->headerSize;

    while (bytesToRead > 0)
    {
        if (lsnm->base.magic == LSNM_MAGIC && lsnm->base.version == LSNM_VERSION)
        {
            *outLsnm = *lsnm;

            return TRUE;
        }

        bytesToRead -= lsnm->base.th8kLen;
        lsnm = (Lsnm *)((u8 *)lsnm + lsnm->base.th8kLen);
    }

    return FALSE;
}
#pragma var_order(flsp, bytesToRead, scoreDat2)
i32 ScoreDat::ParseFLSP(ScoreDat *scoreDat, Flsp *outFlsp)
{
    Flsp *flsp;
    i32 bytesToRead;
    ScoreDat *scoreDat2 = scoreDat;

    flsp = (Flsp *)((u8 *)scoreDat2 + scoreDat2->headerSize);
    bytesToRead = scoreDat2->decompressedFileSize - scoreDat2->headerSize;

    while (bytesToRead > 0)
    {
        if (flsp->base.magic == FLSP_MAGIC && flsp->base.version == FLSP_VERSION)
        {
            *outFlsp = *flsp;

            return TRUE;
        }

        bytesToRead -= flsp->base.th8kLen;
        flsp = (Flsp *)((u8 *)flsp + flsp->base.th8kLen);
    }

    return FALSE;
}

#pragma var_order(clrd, clrdIdx, bytesToRead, difficultyIdx, scoreDat2)
i32 ScoreDat::ParseCLRD(ScoreDat *scoreDat, Clrd *outClrd)
{
    Clrd *clrd;
    i32 bytesToRead;
    i32 clrdIdx;
    i32 difficultyIdx;
    ScoreDat *scoreDat2 = scoreDat;

    if (outClrd == NULL)
    {
        return ZUN_ERROR;
    }

    for (clrdIdx = 0; clrdIdx < SHOT_ALL + 1; clrdIdx++)
    {
        memset(&outClrd[clrdIdx], 0, sizeof(Clrd));

        outClrd[clrdIdx].base.magic = CLRD_MAGIC;
        outClrd[clrdIdx].base.unkLen = sizeof(Clrd);
        outClrd[clrdIdx].base.th8kLen = sizeof(Clrd);
        outClrd[clrdIdx].base.version = CLRD_VERSION;
        outClrd[clrdIdx].shotNumber = clrdIdx;

        for (difficultyIdx = 0; difficultyIdx < MAX_DIFFICULTIES; difficultyIdx++)
        {
            /* Make Stage 1 available in Practice Start. */
            outClrd[clrdIdx].difficultiesClearedWithoutRetries[difficultyIdx] = ZUN_BIT(STAGE1);
            outClrd[clrdIdx].difficultiesClearedWithRetries[difficultyIdx] = ZUN_BIT(STAGE1);
        }
    }

    clrd = (Clrd *)((u8 *)scoreDat2 + scoreDat2->headerSize);
    bytesToRead = scoreDat2->decompressedFileSize - scoreDat2->headerSize;

    while (bytesToRead > 0)
    {
        if (clrd->base.magic == CLRD_MAGIC && clrd->base.version == CLRD_VERSION)
        {
            if (clrd->shotNumber >= SHOT_ALL + 1)
            {
                break;
            }

            outClrd[clrd->shotNumber] = *clrd;
        }

        bytesToRead -= clrd->base.th8kLen;
        clrd = (Clrd *)((u8 *)clrd + clrd->base.th8kLen);
    }

    return ZUN_SUCCESS;
}

#pragma var_order(pscr2, pscr, pscrIdx, bytesToRead, scoreDat2)
i32 ScoreDat::ParsePSCR(ScoreDat *scoreDat, Pscr *outPscr)
{
    Pscr *pscr;
    Pscr *pscr2;
    i32 bytesToRead;
    i32 pscrIdx;
    ScoreDat *scoreDat2 = scoreDat;

    if (outPscr == NULL)
    {
        return ZUN_ERROR;
    }

    pscr2 = outPscr;

    for (pscrIdx = 0; pscrIdx < SHOT_ALL; pscrIdx++, pscr2++)
    {
        memset(pscr2, 0, sizeof(Pscr));

        pscr2->base.magic = PSCR_MAGIC;
        pscr2->base.unkLen = sizeof(Pscr);
        pscr2->base.th8kLen = sizeof(Pscr);
        pscr2->base.version = PSCR_VERSION;
        pscr2->shotNumber = pscrIdx;
        pscr2->unk0x175 = 0;
    }

    pscr = (Pscr *)((u8 *)scoreDat2 + scoreDat2->headerSize);
    bytesToRead = scoreDat2->decompressedFileSize - scoreDat2->headerSize;

    while (bytesToRead > 0)
    {
        if (pscr->base.magic == PSCR_MAGIC && pscr->base.version == PSCR_VERSION)
        {
            /* ??? */
            pscr2 = pscr;

            if (pscr2->shotNumber >= SHOT_ALL)
            {
                break;
            }

            outPscr[pscr2->shotNumber] = *pscr2;
        }

        bytesToRead -= pscr->base.th8kLen;
        pscr = (Pscr *)((u8 *)pscr + pscr->base.th8kLen);
    }

    return ZUN_SUCCESS;
}

#pragma var_order(plst, bytesToRead, scoreDat2)
i32 ScoreDat::ParsePLST(ScoreDat *scoreDat, Plst *outPlst)
{
    Plst *plst;
    i32 bytesToRead;
    ScoreDat *scoreDat2 = scoreDat;

    /* no NULL check here? */

    plst = (Plst *)((u8 *)scoreDat2 + scoreDat2->headerSize);
    bytesToRead = scoreDat2->decompressedFileSize - scoreDat2->headerSize;

    while (bytesToRead > 0)
    {
        if (plst->base.magic == PLST_MAGIC && plst->base.version == PLST_VERSION)
        {
            *outPlst = *plst;
        }

        bytesToRead -= plst->base.th8kLen;
        plst = (Plst *)((u8 *)plst + plst->base.th8kLen);
    }

    return 0;
}

void ScoreDat::ReleaseScore(ScoreDat *score)
{
    ScoreDat::FreeAllScores(score->scores);
    g_ZunMemory.Free(score->scores);
    g_ZunMemory.Free(score);
}

} /* namespace th08 */
