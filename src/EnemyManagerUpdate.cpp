#include "inttypes.hpp"
#include "Supervisor.hpp"
#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "Global.hpp"
#include "GameManager.hpp"
#include "EnemyManager.hpp"
#include "Spellcard.hpp"
#include "Gui.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"
#include "SoundPlayer.hpp"

#include <d3dx8math.h>
#include <math.h>
#include <string.h>

namespace th08
{

struct Gui;
struct Player;
struct GameManager;
struct AnmManager;
struct Spellcard;
struct EffectManager;
struct BulletManager;
struct ItemManager;
class SoundPlayer;
struct AsciiManager;
struct ReplayManager;
class ZunMemory;
struct EnemyManager;
struct Enemy;

extern Gui g_Gui;
extern Player g_Player;
extern GameManager g_GameManager;
extern AnmManager *g_AnmManager;
extern Spellcard g_Spellcard;
extern EffectManager g_EffectManager;
extern BulletManager g_BulletManager;
extern ItemManager g_ItemManager;
extern SoundPlayer g_SoundPlayer;
extern AsciiManager g_AsciiManager;
extern ReplayManager *g_ReplayManager;
extern ZunMemory g_ZunMemory;
extern EnemyManager g_EnemyManager;

extern "C" f32 __stdcall EnemyManagerUpdateFabs(f32 value);

extern u8 g_EnemyManagerUpdateCombatTemplate[0x210];

struct EnemyManagerUpdateOverlay
{
    u8 raw[1];

    void PrepareFrame();
    i32 ConvertBulletBonus(i32 base, i32 bullets);
    i32 OnUpdate();
};


#pragma var_order(currentTargetDelta, difficultyScale, bombHit, damage, enemyIndex, secondaryHitbox, extraDamage, trailIndex, damageOccurred, enemy, rankAmount, previousTargetDelta)
i32 EnemyManagerUpdateOverlay::OnUpdate()
{
    i32 bombHit = 0;
    Enemy *enemy;
    D3DXVECTOR3 secondaryHitbox;
    D3DXVECTOR3 previousTargetDelta;
    D3DXVECTOR3 currentTargetDelta;
    i32 damageOccurred;
    i32 trailIndex;
    i32 extraDamage;
    i32 damage;
    i32 enemyIndex;
    i32 difficultyScale;
    i32 rankAmount;

    difficultyScale = 10;

    if (!g_Gui.IsDialoguePresent())
    {
        ++g_GameManager.stagePlayTimeAll;
        if ((i32)reinterpret_cast<EnemyManager *>(this)->timer >= 16)
        {
            ++g_GameManager.humanityRateDenominator;
            if (!g_Player.optionModeFlag)
                ++g_GameManager.humanityRateNumerator;
        }
    }

    if (g_GameManager.flags.deathbombFreezeActive)
        return 1;

    if (g_GameManager.flags.suppressPlayerShots &&
        reinterpret_cast<EnemyManager *>(this)->bosses[0] != 0)
    {
        // These target-pinned identifiers preserve VC7's block-local hash
        // order.  The initializer values and argument order carry the actual
        // outer/inner semantics.
        D3DXVECTOR3 lowerBounds(384.0f, 448.0f, 0.0f);
        D3DXVECTOR3 upperBounds(192.0f, 224.0f, 0.0f);
        g_Player.CalcDamageToEnemy(
            reinterpret_cast<Float3 *>(&upperBounds), reinterpret_cast<Float3 *>(&lowerBounds),
            &reinterpret_cast<EnemyManager *>(this)->bosses[0]->playerShotHitAccumulator,
            &bombHit);
    }

    reinterpret_cast<EnemyManager *>(this)->UpdateSubrank();

    reinterpret_cast<EnemyManager *>(this)->drawGroupHeads[3] = 0;
    reinterpret_cast<EnemyManager *>(this)->drawGroupHeads[2] = 0;
    reinterpret_cast<EnemyManager *>(this)->drawGroupHeads[1] = 0;
    reinterpret_cast<EnemyManager *>(this)->drawGroupHeads[0] = 0;

    for (enemyIndex = 0; enemyIndex < g_EclManager.GetTimelineCount(); ++enemyIndex)
    {
        if (reinterpret_cast<EnemyManager *>(this)->timelines[enemyIndex].instruction == 0)
        {
            reinterpret_cast<EnemyManager *>(this)->timelines[enemyIndex].instruction =
                reinterpret_cast<EclTimelineInstruction *>(
                    g_EclManager.GetTimeline(enemyIndex));
        }
        reinterpret_cast<EnemyManager *>(this)->timelines[enemyIndex].Run();
    }

    enemy = &reinterpret_cast<EnemyManager *>(this)->enemies[0];
    reinterpret_cast<EnemyManager *>(this)->activeEnemyCount = 0;
    for (enemyIndex = 0; enemyIndex < 480; ++enemyIndex, ++enemy)
    {
        if ((enemy->flags1 & ENEMY_FLAG_ACTIVE) == 0)
        {
            if (g_Player.optionHomingTarget == enemy)
                g_Player.optionHomingTarget = 0;
            continue;
        }

        damageOccurred = 0;

        if (reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->suppressDeathEffects)
        {
            *reinterpret_cast<D3DXVECTOR3 *>(&enemy->worldPosition) =
                *reinterpret_cast<D3DXVECTOR3 *>(&enemy->position) +
                *reinterpret_cast<D3DXVECTOR3 *>(&enemy->positionOffset);
            enemy->worldPosition.z = 0.0f;
            goto process_enemy_death;
        }

        ++reinterpret_cast<EnemyManager *>(this)->activeEnemyCount;

        if ((reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->pauseTimer &&
             (g_Player.bombState.isInUse || g_Player.playerState)) ||
            reinterpret_cast<EnemyFlag2Bits *>(&enemy->flags2)->forcePause)
        {
            enemy->bossTimer--;
            goto update_damage_flash;
        }

    run_enemy_ecl:
        if (reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->linkedChild)
            enemy->UpdateYoukaiAlignment();

    run_enemy_ecl_after_pause:
        if (g_EclManager.RunEcl(enemy) == -1)
        {
            enemy->flags1 &= ~1U;
            enemy->Despawn();
            continue;
        }

        if (!reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->skipMovement)
        {
            enemy->ClampPosition();
            enemy->IntegrateVelocity();
            enemy->ClampPosition();

            if (enemy->parentEnemy != 0 &&
                reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->inheritParentPosition)
            {
                *reinterpret_cast<D3DXVECTOR3 *>(&enemy->positionOffset) =
                    *reinterpret_cast<D3DXVECTOR3 *>(
                        &enemy->parentEnemy->position);
            }

            *reinterpret_cast<D3DXVECTOR3 *>(&enemy->worldPosition) =
                *reinterpret_cast<D3DXVECTOR3 *>(&enemy->position) +
                *reinterpret_cast<D3DXVECTOR3 *>(&enemy->positionOffset);
            enemy->worldPosition.z = 0.0f;
        }
        else
        {
            *reinterpret_cast<D3DXVECTOR3 *>(&enemy->worldPosition) =
                *reinterpret_cast<D3DXVECTOR3 *>(&enemy->position) +
                *reinterpret_cast<D3DXVECTOR3 *>(&enemy->positionOffset);
            enemy->worldPosition.z = 0.0f;
        }

        if (enemy->alignmentEffect != 0)
        {
            *reinterpret_cast<D3DXVECTOR3 *>(
                &enemy->alignmentEffect->position) =
                *reinterpret_cast<D3DXVECTOR3 *>(&enemy->worldPosition);
        }

        if (enemy->trailFlags)
        {
            for (trailIndex = enemy->trailHistoryLength - 1;
                 trailIndex > 0; --trailIndex)
            {
                *reinterpret_cast<D3DXVECTOR3 *>(
                    &enemy->trailSamples[trailIndex].position) =
                    *reinterpret_cast<D3DXVECTOR3 *>(
                        &enemy->trailSamples[trailIndex - 1].position);
                *reinterpret_cast<D3DXVECTOR3 *>(
                    &enemy->trailSamples[trailIndex].velocity) =
                    *reinterpret_cast<D3DXVECTOR3 *>(
                        &enemy->trailSamples[trailIndex - 1].velocity);
                enemy->trailSamples[trailIndex].angle =
                    enemy->trailSamples[trailIndex - 1].angle;
            }

            *reinterpret_cast<D3DXVECTOR3 *>(
                &enemy->trailSamples[0].position) =
                *reinterpret_cast<D3DXVECTOR3 *>(&enemy->worldPosition);
            *reinterpret_cast<D3DXVECTOR3 *>(
                &enemy->trailSamples[0].velocity) =
                *reinterpret_cast<D3DXVECTOR3 *>(&enemy->velocity);
            enemy->trailSamples[0].angle =
                enemy->movementAngle;
        }

        if (enemy->vm.loadedSprite == 0)
            enemy->flags1 |= ENEMY_FLAG_NO_SPRITE;

        if (!reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->noSprite &&
            !reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->hasBeenInBounds &&
            g_GameManager.IsWithinPlayfield(
                (*reinterpret_cast<D3DXVECTOR3 *>(&enemy->worldPosition))[0],
                (*reinterpret_cast<D3DXVECTOR3 *>(&enemy->worldPosition))[1],
                enemy->vm.loadedSprite->widthPx,
                enemy->vm.loadedSprite->heightPx))
        {
            enemy->flags1 |= ENEMY_FLAG_HAS_BEEN_IN_BOUNDS;
        }

        else if (reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->hasBeenInBounds == 1 &&
            !reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->allowOffscreen)
        {
            if ((!enemy->trailFlags &&
                 !g_GameManager.IsWithinPlayfield(
                     enemy->worldPosition.x,
                     enemy->worldPosition.y,
                     enemy->vm.loadedSprite->widthPx,
                     enemy->vm.loadedSprite->heightPx)) ||
                (enemy->trailFlags &&
                 !g_GameManager.IsWithinPlayfield(
                     enemy->worldPosition.x,
                     enemy->worldPosition.y,
                     enemy->vm.loadedSprite->widthPx,
                     enemy->vm.loadedSprite->heightPx) &&
                 !g_GameManager.IsWithinPlayfield(
                     enemy->trailSamples[
                         enemy->trailHistoryLength - 1].position.x,
                     enemy->trailSamples[
                         enemy->trailHistoryLength - 1].position.y,
                     enemy->vm.loadedSprite->widthPx,
                     enemy->vm.loadedSprite->heightPx)))
            {
                enemy->flags1 &= ~1U;
                enemy->Despawn();
                continue;
            }
        }

        if (enemy->HandleLifeCallback())
            goto run_enemy_ecl_after_pause;
        if (enemy->timerCallbackThresholdFrames >= 0 &&
            enemy->HandleTimerCallback())
            goto run_enemy_ecl_after_pause;

        enemy->vm.color1.d3dColor = enemy->displayColor;
        g_AnmManager->ExecuteScript(&enemy->vm);
        enemy->displayColor = enemy->vm.color1.d3dColor;
        for (trailIndex = 0; trailIndex < 2; ++trailIndex)
        {
            if (enemy->secondaryVms[trailIndex].scriptIndex >= 0 &&
                g_AnmManager->ExecuteScript(&enemy->secondaryVms[trailIndex]))
            {
                enemy->secondaryVms[trailIndex].scriptIndex = -1;
            }
        }

        bombHit = g_Player.bombState.isInUse;
        if (!reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->noSprite &&
            !reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->hidePrimaryAnm &&
            !reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->youkaiAligned &&
            (!reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->noDamageDuringStop || !g_Player.bombState.isInUse))
        {
            if (reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->collision)
            {
                enemy->CheckPlayerCollision(
                    &enemy->worldPosition,
                    &enemy->hitboxDimensions);
                if (enemy->trailFlags)
                {
                    secondaryHitbox = *reinterpret_cast<D3DXVECTOR3 *>(
                        &enemy->hitboxDimensions);
                    for (trailIndex = 1; trailIndex < enemy->trailCollisionLength; trailIndex += 6)
                    {
                        if (enemy->trailFlags & ENEMY_TRAIL_TAPER)
                        {
                            secondaryHitbox = *reinterpret_cast<D3DXVECTOR3 *>(
                                                  &enemy->hitboxDimensions) -
                                (*reinterpret_cast<D3DXVECTOR3 *>(
                                     &enemy->hitboxDimensions) *
                                 (f32)trailIndex / (f32)enemy->trailCollisionLength);
                        }
                        enemy->CheckPlayerCollision(
                            &enemy->trailSamples[trailIndex].position,
                            reinterpret_cast<Float3 *>(&secondaryHitbox));
                    }
                }
            }

            enemy->lastDamage = 0;
            if (reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->acceptsDamage)
            {
                if (!g_Spellcard.IsActive() || !reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(enemy)->HasAttachedEnemy() ||
                    !g_Player.bombState.isInUse)
                {
                    damage = g_Player.CalcDamageToEnemy(
                        &enemy->worldPosition,
                        &enemy->hitboxDimensions,
                        &enemy->playerShotHitAccumulator, &bombHit);
                }
                else
                {
                    damage = 0;
                }

                if (enemy->secondaryHitboxDimensions.x > 0.0f)
                {
                    extraDamage = g_Player.CalcDamageToEnemy(
                        &enemy->worldPosition,
                        &enemy->secondaryHitboxDimensions,
                        &enemy->playerShotHitAccumulator, &bombHit);
                    if (!bombHit)
                    {
                        if (g_GameManager.shotType == 3 ||
                            g_GameManager.shotType == 11)
                            damage = (i32)((f32)damage + (f32)extraDamage / 6.5f);
                        else
                            damage = (i32)((f32)damage + (f32)extraDamage / 1.7f);
                    }
                }

                if (damage > 0)
                {
                    if ((reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->boss || !g_Player.optionModeFlag) &&
                        !g_Player.bombState.isInUse)
                    {
                        if (reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->boss && !g_Player.optionModeFlag)
                            rankAmount = 10 * (damage / (10 - difficultyScale / 3));
                        else
                            rankAmount = 10 * (damage / (30 - difficultyScale));
                        if (rankAmount > 70)
                            rankAmount = 70;
                        if (!rankAmount &&
                            (!g_Player.optionModeFlag || ((i32)enemy->bossTimer & 1)))
                            rankAmount = 10;
                    }

                    if (damage >= 70)
                        damage = 70;
                    g_GameManager.AddScore(10 * (damage / 5));

                    if (reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->damageable)
                    {
                        if (g_Spellcard.IsActive())
                        {
                            if (!bombHit)
                            {
                                if (damage > 7)
                                    damage /= 7;
                                else if (damage != 0)
                                    damage = 1;
                            }
                            else if (g_Spellcard.FUN_0042DFF0() && !reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(enemy)->HasAttachedEnemy())
                            {
                                if (damage > 2)
                                    damage = (i32)((f32)damage / 2.5f);
                                else if (damage != 0)
                                    damage = 1;
                            }
                            else
                                damage = 0;
                        }

                        if (enemy->damageReductionTimer > 0)
                        {
                            if (reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->boss)
                                damage /= 9;
                            else
                                damage = 0;
                        }

                        enemy->life -= damage;
                        enemy->lastDamage = damage;
                        enemy->ApplyDamageToParent(damage);
                    }
                    damageOccurred = 1;
                }

                if (reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->boss)
                {
                    previousTargetDelta =
                        *reinterpret_cast<D3DXVECTOR3 *>(&g_Player.tailPosition0) -
                        *reinterpret_cast<D3DXVECTOR3 *>(&g_Player.position);
                    currentTargetDelta = *reinterpret_cast<D3DXVECTOR3 *>(
                                             &enemy->worldPosition) -
                        *reinterpret_cast<D3DXVECTOR3 *>(&g_Player.position);
                    if (!g_Player.enemyTrackedPositionValid ||
                        fabsf(previousTargetDelta.x) >
                            fabsf(currentTargetDelta.x))
                    {
                        g_Player.tailPosition0 = enemy->worldPosition;
                    }
                    g_Player.enemyTrackedPositionValid = 1;
                }

                if (!g_Player.enemyTrackedPositionValid &&
                    g_Player.tailPosition0[1] <
                        (*reinterpret_cast<D3DXVECTOR3 *>(
                            &enemy->worldPosition))[1])
                {
                    g_Player.tailPosition0 = enemy->worldPosition;
                }

                if (fabsf(
                        enemy->worldPosition.x -
                        reinterpret_cast<D3DXVECTOR3 *>(&g_Player.position)->x) < 64.0f &&
                    !reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(enemy)->HasAttachedEnemy() &&
                    (g_Player.optionHomingTarget == 0 ||
                     reinterpret_cast<Enemy *>(g_Player.optionHomingTarget)->position.y >
                         enemy->worldPosition.y))
                {
                    g_Player.optionHomingTarget = enemy;
                }
            }
        }

        if (reinterpret_cast<EnemyFlag2Bits *>(&enemy->flags2)->deathLatch &&
            enemy->life > 0)
            enemy->flags2 &= ~ENEMY_FLAG2_DEATH_LATCH;

        if (enemy->life <= 0 &&
            !reinterpret_cast<EnemyFlag2Bits *>(&enemy->flags2)->deathLatch &&
            !reinterpret_cast<EnemyFlag2Bits *>(&enemy->flags2)->noDeath)
        {
        process_enemy_death:
            ;
            // VC7 hashes nested locals by identifier.  These target-pinned
            // spellings carry, respectively, the bonus value, the otherwise
            // unused death-position vector, and the VM loop/gauge value.
            i32 deathVmIndex;

            enemy->flags2 |= ENEMY_FLAG2_DEATH_LATCH;
            D3DXVECTOR3 bonus;
            i32 deathPosition;
            enemy->phaseEndTimeRemainingSeconds =
                (enemy->timerCallbackThresholdFrames -
                 (i32)enemy->bossTimer) / 60;
            enemy->timerCallbackThresholdFrames = -1;
            for (deathPosition = 0; deathPosition < 4; ++deathPosition)
                enemy->lifeCallbackThresholds[deathPosition] = -1;
            for (deathPosition = 0; deathPosition < 4; ++deathPosition)
            {
                if (enemy->childEclBlocks[deathPosition] != 0)
                {
                    g_ZunMemory.Free(enemy->childEclBlocks[deathPosition]);
                    enemy->childEclBlocks[deathPosition] = 0;
                }
            }

            if (reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(enemy)->HasAttachedEnemy())
            {
                --enemy->parentEnemy->linkedChildCount;
            }

            reinterpret_cast<EclOperands::EnemyOverlay *>(enemy)->FUN_0042adb0(1);
            if (!g_Player.optionModeFlag)
                deathPosition = -200;
            else
                deathPosition = 200;
            g_GameManager.AddToYoukaiGauge(deathPosition, 0);

            switch ((enemy->flags1 >> 20) & 7)
            {
            case 3:
                enemy->life = 1;
                enemy->flags1 &= ~ENEMY_FLAG_DAMAGEABLE;
                enemy->flags1 &= ~ENEMY_FLAG_DEATH_MODE_MASK;
                g_Gui.SetBossPresent(false);
                g_ReplayManager->flags |= 0x20;
                if (enemy->deathAnm1 >= 0)
                {
                    g_EffectManager.SpawnEffect(enemy->deathAnm1, reinterpret_cast<D3DXVECTOR3 *>(&enemy->worldPosition), 1, -1);
                    g_EffectManager.SpawnEffect(enemy->deathAnm1, reinterpret_cast<D3DXVECTOR3 *>(&enemy->worldPosition), 1, -1);
                    g_EffectManager.SpawnEffect(enemy->deathAnm1, reinterpret_cast<D3DXVECTOR3 *>(&enemy->worldPosition), 1, -1);
                }
                if (enemy->alignmentEffect != 0)
                {
                    enemy->alignmentEffect->vm.SetInterrupt(3);
                    enemy->alignmentEffect = 0;
                }
                if (!g_Player.playerState)
                {
                    g_Player.timer = 90;
                    g_Player.playerState = PLAYER_STATE_INVULNERABLE;
                }
                enemy->flags1 &= ~ENEMY_FLAG_PAUSE_TIMER;
                enemy->flags1 &= ~ENEMY_FLAG_NO_DAMAGE_DURING_STOP;
                goto death_audio_and_callback;

            case 1:
                g_GameManager.AddScore(enemy->score);
                enemy->flags1 |= ENEMY_FLAG_PERSIST_AFTER_DEATH;
                enemy->flags1 &= ~ENEMY_FLAG_COLLISION;
                enemy->flags1 &= ~ENEMY_FLAG_DAMAGEABLE;
                enemy->flags1 &= ~ENEMY_FLAG_ACCEPTS_DAMAGE;
                goto common_death_mode;

            case 0:
                g_GameManager.AddScore(enemy->score);
                enemy->flags1 &= ~1U;
                if (enemy->alignmentEffect != 0)
                {
                    enemy->alignmentEffect->vm.SetInterrupt(3);
                    enemy->alignmentEffect = 0;
                }
                goto common_death_mode;

            case 2:
            common_death_mode:
                if (reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->boss)
                {
                    g_Gui.SetBossPresent(false);
                    enemy->ReleaseAttachedEffects();
                }
                enemy->DropItems(bombHit);
                if (reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->boss &&
                    !g_Spellcard.IsActive())
                {
                    deathVmIndex = g_BulletManager.DespawnBullets(8000, 1);
                    deathVmIndex = g_EnemyManager.KillAllNonBossEnemies(8000, deathVmIndex);
                    if (deathVmIndex)
                    {
                        g_GameManager.AddScore(deathVmIndex);
                        g_Gui.ShowBonusScore(deathVmIndex);
                    }
                }
                enemy->life = 0;
                g_ReplayManager->flags |= 0x20;
                break;
            }

        death_audio_and_callback:
            if (!reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->suppressDeathEffects)
            {
                g_SoundPlayer.PlaySoundPositionedByIdx(
                    static_cast<SoundIdx>(enemyIndex % 2 + 2), enemy->worldPosition.x);
                if (enemy->deathAnm1 >= 0)
                {
                    g_EffectManager.SpawnEffect(enemy->deathAnm1, reinterpret_cast<D3DXVECTOR3 *>(&enemy->worldPosition), 1, -1);
                    g_EffectManager.SpawnEffect(enemy->deathAnm2 + 4, reinterpret_cast<D3DXVECTOR3 *>(&enemy->worldPosition), 4, -1);
                }
                if (g_GameManager.GaugeIsExtremelyHuman() ||
                    g_GameManager.GaugeIsExtremelyYoukai())
                {
                    g_ItemManager.SpawnItem(&enemy->worldPosition, static_cast<ItemType>(7), 1);
                }
            }

            if (enemy->deathCallbackSubId >= 0)
            {
                i32 callbackVmIndex;

                enemy->enemy_fun_00415c80();
                enemy->activeEclCallStackDepth = 0;
                for (callbackVmIndex = 0; callbackVmIndex < 4; ++callbackVmIndex)
                    enemy->lifeCallbackThresholds[callbackVmIndex] = -1;
                enemy->timerCallbackThresholdFrames = -1;
                enemy->ReleaseChildEclBlocks();
                memcpy(&enemy->bulletSpawnDescriptor,
                       &g_EnemyManager.spawnTemplate.bulletSpawnDescriptor,
                       sizeof(BulletSpawnDescriptor));
                enemy->shootIntervalFrames = 0;
                g_EclManager.CallEclSub(
                    reinterpret_cast<EnemyEclContext *>(
                        &enemy->mainEclContextStorage),
                    enemy->deathCallbackSubId);
                enemy->deathCallbackSubId = -1;
            }
        }

    update_damage_flash:
        if (!reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->youkaiAligned)
        {
            if (enemy->damageFlashTimer)
            {
                --enemy->damageFlashTimer;
                enemy->vm.flagsWord &= ~0x20000U;
            }
            else if (damageOccurred)
            {
                if (reinterpret_cast<EnemyFlag2Bits *>(&enemy->flags2)->damageFeedbackLevel < 2)
                    g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(20), enemy->worldPosition.x);
                else
                    g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(37), enemy->worldPosition.x);

                enemy->vm.color2.r = 0xFF;
                enemy->vm.color2.g = 0x60;
                enemy->vm.color2.b = 0x80;
                enemy->vm.color2.a = enemy->vm.color1.a;
                enemy->vm.flagsWord |= 0x20000U;
                enemy->damageFlashTimer = 1;
            }
            else
            {
                enemy->vm.flagsWord &= ~0x20000U;
            }
        }
        else
        {
            enemy->vm.color2.r = 0x20;
            enemy->vm.color2.g = 0x20;
            enemy->vm.color2.b = 0xC0;
            enemy->vm.color2.a = enemy->vm.color1.a / 2;
            enemy->vm.flagsWord |= 0x20000U;
        }

        if (reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->boss)
        {
            D3DXVECTOR3 markerPosition;

            if (!g_Gui.IsDialoguePresent() && !enemy->bossSlot)
                g_Gui.SetBossLifeBarTarget((f32)enemy->life / (f32)enemy->maxLife);

            if (reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->boss < 4)
            {
                if (!reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->noSprite)
                    markerPosition.x = enemy->worldPosition.x + 32.0f;
                else
                    markerPosition.x = -999.0f;
                markerPosition.y = 472.0f;
                markerPosition.z = 0.0f;
                g_AsciiManager.SetBossMarkerPosition(enemy->bossSlot, &markerPosition);

                if (reinterpret_cast<EnemyFlag2Bits *>(&enemy->flags2)->damageFeedbackLevel == 0)
                    g_AsciiManager.SetBossMarkerState(
                        enemy->bossSlot,
                        enemy->vm.flag17 != 0);
                else
                    g_AsciiManager.SetBossMarkerState(
                        enemy->bossSlot,
                        reinterpret_cast<EnemyFlag2Bits *>(&enemy->flags2)->damageFeedbackLevel + 1);
            }
        }

        enemy->UpdateEffects();
        if (!g_GameManager.scriptedUpdateFreeze)
            enemy->bossTimer++;
        if (enemy->damageReductionTimer > 0)
            enemy->damageReductionTimer--;

        if (!reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->noSprite &&
            reinterpret_cast<EnemyFlag1Bits *>(&enemy->flags1)->active)
        {
            enemy->nextInDrawGroup = reinterpret_cast<EnemyManager *>(this)->drawGroupHeads[
                enemy->drawGroup];
            reinterpret_cast<EnemyManager *>(this)->drawGroupHeads[
                enemy->drawGroup] = enemy;
        }
    }

    if ((reinterpret_cast<EnemyManager *>(this)->timer % 200) == 0 &&
        g_GameManager.IsTampered())
        return 4;

    reinterpret_cast<EnemyManager *>(this)->timer++;
    return 1;
}

// FUNCTION: th08 0x42c660
ChainCallbackResult EnemyManager::OnUpdate(EnemyManager *enemyManager)
{
    return static_cast<ChainCallbackResult>(
        reinterpret_cast<EnemyManagerUpdateOverlay *>(enemyManager)->OnUpdate());
}

} // namespace th08
