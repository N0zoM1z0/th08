#include "th_pch.h"
#include "GameManager.hpp"
#include "ResultScreen.hpp"
#include "ScoreDat.hpp"
#include "Supervisor.hpp"
#include "EclManager.hpp"
#include "Global.hpp"

namespace th08
{

// FUNCTION: th08 0x43bbe1
#pragma var_order(catk, i, scoreDat, j)
i32 InitializeScoreData()
{
    Catk *catk;
    i32 i;
    ScoreDat *scoreDat;
    i32 j;

    catk = g_GameManager.catkData;
    ResultScreen::RegisterChain(2);
    memset(g_GameManager.catkData, 0, sizeof(g_GameManager.catkData));

    for (i = 0; i < SPELLCARD_COUNT_SPELLCARDS; ++i, ++catk)
    {
        catk->base.magic = CATK_MAGIC;
        catk->base.unkLen = sizeof(Catk);
        catk->base.th8kLen = sizeof(Catk);
        catk->base.version = 3;
        catk->spellcardNumber = static_cast<u16>(i);
        for (j = 0; j < 7; ++j)
        {
            catk->inGameHistory.attempts[j] = 0;
            catk->inGameHistory.captures[j] = 0;
            catk->inGameHistory.maxBonus[j] = 0;
        }
    }

    scoreDat = ScoreDat::OpenScore("score.dat");
    if (scoreDat == NULL)
    {
        g_GameErrorContext.Log("error : スコアファイルの作成に失敗しました\r\n");
        return ZUN_ERROR;
    }

    g_GameManager.globals->displayedHighScore =
        ScoreDat::GetHighScore(scoreDat, NULL, g_GameManager.shotType, g_GameManager.difficulty,
                               &g_GameManager.globals->continuesUsedInHighScore);
    ScoreDat::ParseCATK(scoreDat, g_GameManager.catkData);
    ScoreDat::ParseCLRD(scoreDat, g_GameManager.clrdData);
    ScoreDat::ParsePSCR(scoreDat, g_GameManager.pscrData);

    if ((*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3dbac) & 1U) != 0)
    {
        g_GameManager.globals->displayedHighScore =
            g_GameManager.pscrData[g_GameManager.shotType]
                .highScores[g_GameManager.currentStage][g_GameManager.difficulty];
        g_GameManager.pscrData[g_GameManager.shotType]
            .attempts[g_GameManager.currentStage][g_GameManager.difficulty]++;
        g_GameManager.pscrData[g_GameManager.shotType].shotNumber = 1;
    }

    ScoreDat::ReleaseScore(scoreDat);
    memcpy(g_GameManager.catkData2, g_GameManager.catkData, sizeof(g_GameManager.catkData));
    memset(&g_GameManager.hscr, 0, sizeof(g_GameManager.hscr));
    g_GameManager.hscr.character = g_GameManager.shotType;
    g_GameManager.hscr.difficulty = static_cast<u8>(g_GameManager.difficulty);
    g_GameManager.hscr.cfg = g_Supervisor.cfg;
    g_GameManager.playtimeFrames = 0;
    return ZUN_SUCCESS;
}

} // namespace th08
