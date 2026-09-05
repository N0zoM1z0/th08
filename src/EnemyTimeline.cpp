#include "th_pch.h"

#include "EnemyManager.hpp"
#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "Gui.hpp"
#include "GameManager.hpp"
#include "ReplayManager.hpp"

namespace th08
{


// FUNCTION: th08 0x42a4e0
#pragma var_order(i, enemy, this)
Enemy *EnemyManager::SpawnEnemy1(i32 eclSubroutineId, const D3DXVECTOR3 *position, i32 life, i32 itemDropType,
                                 i32 score, i32 mirrorMovementX)
{
    struct EnemySpawnCopy
    {
        u32 words[sizeof(Enemy) / sizeof(u32)];
    };
    i32 i;
    Enemy *enemy;

    enemy = &this->enemies[0];
    g_ReplayManager->frameEventFlags |= 0x1000;
    for (i = 0; i < 480; i++, enemy++)
    {
        if ((enemy->flags1 & ENEMY_FLAG_ACTIVE) != 0)
            continue;

        *reinterpret_cast<EnemySpawnCopy *>(enemy) =
            *reinterpret_cast<const EnemySpawnCopy *>(&this->spawnTemplate);
        enemy->enemyIndex = i;
        reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->mirrorMovementX = mirrorMovementX;
        if (life >= 0)
            enemy->life = life;
        enemy->position = *FLOAT3_CONST_PTR(position);
        g_EclManager.CallEclSub(
            &enemy->mainEclContextStorage, (i16)eclSubroutineId);
        if (g_EclManager.RunEcl(enemy) == ZUN_ERROR)
        {
            reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->active = 0;
            i = 480;
        }
        else
        {
            enemy->displayColor = enemy->vm.color1.d3dColor;
            enemy->itemDropType = (i8)itemDropType;
            if (score >= 0)
                enemy->score = score;
            enemy->maxLife = enemy->life;
            enemy->phaseStartingLife = enemy->maxLife;
        }
        break;
    }
    this->lastSpawnFailed = (i == 480);
    return enemy;
}

// FUNCTION: th08 0x42a680
#pragma var_order(i, enemy, this)
Enemy *EnemyManager::SpawnEnemy2(i32 eclSubroutineId, const D3DXVECTOR3 *position, i32 life, i32 itemDropType,
                                 i32 score, i32 *contextInts)
{
    struct EnemySpawnCopy
    {
        u32 words[sizeof(Enemy) / sizeof(u32)];
    };
    struct EnemyContextCopy
    {
        u32 words[0x78 / sizeof(u32)];
    };
    i32 i;
    Enemy *enemy;

    enemy = &this->enemies[0];
    g_ReplayManager->frameEventFlags |= 0x1000;
    for (i = 0; i < 480; i++, enemy++)
    {
        if ((enemy->flags1 & ENEMY_FLAG_ACTIVE) != 0)
            continue;

        *reinterpret_cast<EnemySpawnCopy *>(enemy) =
            *reinterpret_cast<const EnemySpawnCopy *>(&this->spawnTemplate);
        enemy->enemyIndex = i;
        if (life >= 0)
            enemy->life = life;
        enemy->position = *FLOAT3_CONST_PTR(position);
        g_EclManager.CallEclSub(
            &enemy->mainEclContextStorage, (i16)eclSubroutineId);
        *reinterpret_cast<EnemyContextCopy *>(
            enemy->mainEclContextStorage.intVariables) =
            *reinterpret_cast<const EnemyContextCopy *>(contextInts);
        if (g_EclManager.RunEcl(enemy) == ZUN_ERROR)
        {
            reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->active = 0;
            i = 480;
        }
        else
        {
            enemy->displayColor = enemy->vm.color1.d3dColor;
            enemy->itemDropType = (i8)itemDropType;
            if (life >= 0)
                enemy->life = life;
            if (score >= 0)
                enemy->score = score;
            enemy->maxLife = enemy->life;
            enemy->phaseStartingLife = enemy->maxLife;
        }
        break;
    }
    this->lastSpawnFailed = (i == 480);
    return enemy;
}


#pragma var_order(variant, args1, position1, args15, position15, args11, spawned11, position11, argsRange, positionRange, argsPlay, positionPlay, i, matchCount, j)
// FUNCTION: th08 0x42a8a0
void EclTimeline::Run()
{
    i32 variant;

    variant = 0;
    while (this->instruction->time >= 0)
    {
        if (this->timer == this->instruction->time)
        {
            variant = 0;
            if ((this->instruction->difficultyMask & g_GameManager.difficultyMask) == 0)
                goto advance_instruction;

            switch (this->instruction->opcode)
            {
            case 16:
                g_GameManager.showRetryMenu = 1;
                break;

            case 1:
                variant = 1;
            case 0:
                if (!g_Gui.IsBossPresent() && g_EnemyManager.suppressTimelineSpawns == 0)
                {
                    i32 *args1 = this->instruction->args.ints;
                    D3DXVECTOR3 position1;
                    position1.x = reinterpret_cast<f32 *>(args1)[1];
                    position1.y = reinterpret_cast<f32 *>(args1)[2];
                    position1.z = 0.0f;
                    g_EnemyManager.SpawnEnemy1(args1[0], &position1, args1[3], args1[4], args1[5], variant);
                }
                break;

            case 15:
            {
                i32 *args15 = this->instruction->args.ints;
                D3DXVECTOR3 position15;
                position15.x = reinterpret_cast<f32 *>(args15)[1];
                position15.y = reinterpret_cast<f32 *>(args15)[2];
                position15.z = 0.0f;
                g_EnemyManager.SpawnEnemy1(args15[0], &position15, args15[3], args15[4], args15[5], variant);
                break;
            }

            case 12:
                variant = 1;
            case 11:
                if (!g_Gui.IsBossPresent() && g_EnemyManager.suppressTimelineSpawns == 0)
                {
                    struct SpecialSpawnLocals
                    {
                        Enemy *spawned11;
                        i32 *args11;
                    } locals;
                    locals.args11 = this->instruction->args.ints;
                    D3DXVECTOR3 position11;
                    position11.x = reinterpret_cast<f32 *>(locals.args11)[1];
                    position11.y = reinterpret_cast<f32 *>(locals.args11)[2];
                    position11.z = 0.0f;
                    locals.spawned11 = g_EnemyManager.SpawnEnemy1(
                        locals.args11[0], &position11, locals.args11[3], -1, locals.args11[6], variant);
                    locals.spawned11->pointItemDropCount = locals.args11[4];
                    locals.spawned11->powerOrPointItemDropCount = locals.args11[5];
                }
                break;

            case 4:
                variant = 1;
            case 2:
                if (!g_Gui.IsBossPresent() && g_EnemyManager.suppressTimelineSpawns == 0)
                {
                    i32 *argsRange = this->instruction->args.ints;
                    D3DXVECTOR3 positionRange;
                    positionRange.x = g_Rng.GetRandomF32InRange(
                        reinterpret_cast<f32 *>(argsRange)[2] - reinterpret_cast<f32 *>(argsRange)[1]) +
                        reinterpret_cast<f32 *>(argsRange)[1];
                    positionRange.y = reinterpret_cast<f32 *>(argsRange)[3];
                    positionRange.z = 0.0f;
                    g_EnemyManager.SpawnEnemy1(
                        argsRange[0], &positionRange, argsRange[4], argsRange[5], argsRange[6], variant);
                }
                break;

            case 5:
                variant = 1;
            case 3:
                if (!g_Gui.IsBossPresent() && g_EnemyManager.suppressTimelineSpawns == 0)
                {
                    i32 *argsPlay = this->instruction->args.ints;
                    {
                        D3DXVECTOR3 positionPlay;
                        positionPlay.x = g_Rng.GetRandomF32InRange(384.0f);
                        positionPlay.y = reinterpret_cast<f32 *>(argsPlay)[1];
                        positionPlay.z = 0.0f;
                        g_EnemyManager.SpawnEnemy1(
                            argsPlay[0], &positionPlay, argsPlay[2], argsPlay[3], argsPlay[4], variant);
                    }
                }
                break;

            case 6:
                g_Gui.MsgRead(this->instruction->args.ints[0]);
                break;

            case 7:
                if (g_Gui.MsgWait())
                {
                    this->timer--;
                    goto done;
                }
                break;

            case 8:
                g_EnemyManager.bosses[
                    this->instruction->args.ints[0]]->pendingEclSubroutineIndex =
                    static_cast<i16>(this->instruction->args.ints[1]);
                break;

            case 9:
                g_GameManager.SetPower(this->instruction->args.ints[0]);
                break;

            case 10:
                if (g_EnemyManager.bosses[this->instruction->args.ints[0]] != NULL &&
                    (g_EnemyManager.bosses[this->instruction->args.ints[0]]->flags1 &
                     ENEMY_FLAG_ACTIVE) != 0)
                {
                    this->timer--;
                    goto done;
                }
                break;

            case 13:
            {
                i32 matchCount = 0;
                for (u32 i = 0; i < 4; i++)
                {
                    if (g_EnemyManager.timelineEventSlots[i] ==
                        this->instruction->args.ints[0])
                    {
                        matchCount++;
                        g_EnemyManager.timelineEventSlots[i] = -1;
                    }
                }
                if (matchCount == 0)
                {
                    this->timer--;
                    goto done;
                }
                break;
            }

            case 14:
            {
                for (u32 j = 0; j < 4; j++)
                {
                    if (g_EnemyManager.timelineEventSlots[j] >= 0)
                        continue;
                    g_EnemyManager.timelineEventSlots[j] = this->instruction->args.ints[0];
                }
                break;
            }
            }
        }
        else if (this->timer < this->instruction->time)
        {
            break;
        }

advance_instruction:
        this->instruction = reinterpret_cast<EclTimelineInstruction *>(
            reinterpret_cast<u8 *>(this->instruction) + this->instruction->size);
    }

done:
    this->timer++;
}


} // namespace th08
