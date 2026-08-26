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
extern i8 g_EclScriptedGlobalUpdateFreeze;

/*
 * Private target-pinned overlays for EnemyManager::OnUpdate at 0x0042C660.
 *
 * The shared Enemy/EnemyManager layouts are intentionally not widened here.
 * Every offset used below is observed in TH08 1.00d.  Names for still-unmapped
 * callees and globals are provisional and local to this translation unit.
 */

struct EnemyManagerUpdateSprite
{
    u8 unknown00[0x30];
    f32 height;
    f32 width;
};

struct EnemyManagerUpdateAnmVm
{
    u8 unknown000[0x1F0];
    u32 color;
    u32 unknown1F4;
    u32 flags;
    u8 unknown1FC[0x1E];
    i16 scriptIndex;
    u8 unknown21C[8];
    EnemyManagerUpdateSprite *sprite;
    u8 unknown228[0x7C];

    void SetInterrupt(i32 interrupt);
};
typedef char EnemyManagerUpdateAnmVmSizeCheck[sizeof(EnemyManagerUpdateAnmVm) == 0x2A4 ? 1 : -1];

struct EnemyManagerUpdateTimer
{
    i32 previous;
    f32 subFrame;
    i32 current;

    operator i32();
    void operator++(int);
    void operator--(int);
    i32 operator>(i32 value);
    i32 operator%(i32 value);
    void operator=(i32 value);
};

struct EnemyManagerUpdateEnemy
{
    u8 raw[0x53D0];

    EnemyManagerUpdateAnmVm *PrimaryVm()
    {
        return reinterpret_cast<EnemyManagerUpdateAnmVm *>(raw + 0xC);
    }

    EnemyManagerUpdateAnmVm *ExtraVm(i32 index)
    {
        return reinterpret_cast<EnemyManagerUpdateAnmVm *>(raw + 0x2B0 + 0x2A4 * index);
    }

    void *DamageData() { return &reinterpret_cast<Enemy *>(this)->playerShotHitAccumulator; }
    i32 &Life() { return reinterpret_cast<Enemy *>(this)->life; }
    i32 &MaxLife() { return reinterpret_cast<Enemy *>(this)->maxLife; }
    i32 &Score() { return reinterpret_cast<Enemy *>(this)->score; }
    EnemyManagerUpdateTimer *BossTimer() { return reinterpret_cast<EnemyManagerUpdateTimer *>(&reinterpret_cast<Enemy *>(this)->bossTimer); }
    D3DCOLOR &DisplayColor() { return reinterpret_cast<Enemy *>(this)->displayColor; }
    u32 &Flags1() { return reinterpret_cast<Enemy *>(this)->flags1; }
    u32 &Flags2() { return reinterpret_cast<Enemy *>(this)->flags2; }
    i8 &DeathAnm1() { return reinterpret_cast<Enemy *>(this)->deathAnm1; }
    u8 &DeathAnm2() { return reinterpret_cast<Enemy *>(this)->deathAnm2; }
    u8 &BossSlot() { return reinterpret_cast<Enemy *>(this)->bossSlot; }
    u8 &DamageFlashTimer() { return reinterpret_cast<Enemy *>(this)->damageFlashTimer; }
    u8 &DrawGroup() { return reinterpret_cast<Enemy *>(this)->drawGroup; }
    EnemyManagerUpdateEnemy *&DrawNext()
    {
        return *reinterpret_cast<EnemyManagerUpdateEnemy **>(raw);
    }

    i32 IsActive() { return (Flags1() & 1) != 0; }
    void ClearActive() { Flags1() &= ~1U; }

    void PauseUpdate();
    void ClampPosition();
    void Move();
    void Despawn();
    i32 HandleLifeCallback();
    i32 HandleTimerCallback();
    void CheckPlayerCollision(D3DXVECTOR3 *position, D3DXVECTOR3 *hitbox);
    void ApplyDamage(i32 damage);
    i32 IsBossPart();
    void ReleaseEffects(i32 mode);
    void PrepareDeathCallback();
    void ResetForDeathCallback();
    void FinalizeDeath(i32 bombHit);
    void UpdateEffects();
    void SpecialBossDeath();
};
typedef char EnemyManagerUpdateEnemySizeCheck[sizeof(EnemyManagerUpdateEnemy) == 0x53D0 ? 1 : -1];

struct EnemyManagerUpdateTimelineLane
{
    EnemyManagerUpdateTimer timer;
    void *instruction;
    void Run();
};
typedef char EnemyManagerUpdateTimelineLaneSizeCheck[sizeof(EnemyManagerUpdateTimelineLane) == 0x10 ? 1 : -1];

struct EnemyManagerUpdateEclManager
{
    i32 GetTimelineCount();
    void *GetTimeline(i32 index);
    i32 RunEcl(EnemyManagerUpdateEnemy *enemy);
    i32 CallEclSub(void *context, i16 subId);
};

struct EnemyManagerUpdateGui
{
    i32 IsDialogPresent();
    void ResetBossUi(i32 value);
    void SetBossHealth(f32 health);
    void ShowBonus(i32 score);
};

struct EnemyManagerUpdateGameManager
{
    i32 IsWithinPlayfield(f32 x, f32 y, f32 width, f32 height);
    void AddScore(i32 score);
    void AddToYoukaiGauge(i32 amount, i32 unknown);
    i32 GaugeIsExtremelyHuman();
    i32 GaugeIsExtremelyYoukai();
    i32 IsTampered();
};

struct EnemyManagerUpdatePlayer
{
    i8 playerType;
    u8 unknown01[2];
    u8 mode;

    i32 CalcDamageToEnemy(D3DXVECTOR3 *position, D3DXVECTOR3 *hitbox,
                          void *damageData, i32 *bombHit);
};

struct EnemyManagerUpdateAnmManager
{
    i32 ExecuteScript(EnemyManagerUpdateAnmVm *vm);
};

struct EnemyManagerUpdateSpellcard
{
    i32 IsActive();
    i32 AllowsBombDamage();
};

struct EnemyManagerUpdateEffectManager
{
    void SpawnEffect(i32 animation, D3DXVECTOR3 *position, i32 count, i32 color);
};

struct EnemyManagerUpdateBulletManager
{
    i32 DespawnBullets(i32 maxScore, i32 awardScore);
};

struct EnemyManagerUpdateItemManager
{
    void SpawnItem(D3DXVECTOR3 *position, i32 item, i32 amount);
};

struct EnemyManagerUpdateSoundPlayer
{
    void PlaySoundPositionedByIdx(i32 sound, f32 x);
};

struct EnemyManagerUpdateAsciiManager
{
    void SetBossMarkerPosition(i32 slot, D3DXVECTOR3 *position);
    void SetBossMarkerState(i32 slot, i32 state);
};

struct EnemyManagerUpdateReplayManager
{
    u8 unknown000[0xDA];
    u16 flags;
};

struct EnemyManagerUpdateMemory
{
    void Free(void *ptr);
};

extern "C" f32 __stdcall EnemyManagerUpdateFabs(f32 value);

extern u8 g_EnemyManagerUpdateCombatTemplate[0x210];

struct EnemyManagerUpdateOverlay
{
    u8 raw[1];

    void PrepareFrame();
    i32 ConvertBulletBonus(i32 base, i32 bullets);
    i32 OnUpdate();
};


struct EnemyManagerUpdateManagerFlagBits
{
    u32 unknown00_09 : 10;
    u32 skipUpdate : 1;
    u32 unknown11_12 : 2;
    u32 damageBoss : 1;
    u32 unknown14_31 : 18;
};

#pragma var_order(currentTargetDelta, difficultyScale, bombHit, damage, enemyIndex, secondaryHitbox, extraDamage, trailIndex, damageOccurred, enemy, rankAmount, previousTargetDelta)
i32 EnemyManagerUpdateOverlay::OnUpdate()
{
    i32 bombHit = 0;
    EnemyManagerUpdateEnemy *enemy;
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

    if (!g_Gui.IsDialogPresent())
    {
        ++(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE04));
        if ((i32)reinterpret_cast<EnemyManager *>(this)->timer >= 16)
        {
            ++(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBA4));
            if (!reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->mode)
                ++(*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBA0));
        }
    }

    if (reinterpret_cast<EnemyManagerUpdateManagerFlagBits *>(
            reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBAC)->skipUpdate)
        return 1;

    if (reinterpret_cast<EnemyManagerUpdateManagerFlagBits *>(
            reinterpret_cast<u8 *>(&g_GameManager) + 0x3DBAC)->damageBoss &&
        reinterpret_cast<EnemyManager *>(this)->bosses[0] != 0)
    {
        // These target-pinned identifiers preserve VC7's block-local hash
        // order.  The initializer values and argument order carry the actual
        // outer/inner semantics.
        D3DXVECTOR3 lowerBounds(384.0f, 448.0f, 0.0f);
        D3DXVECTOR3 upperBounds(192.0f, 224.0f, 0.0f);
        g_Player.FUN_00451670(
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

    enemy = reinterpret_cast<EnemyManagerUpdateEnemy *>(
        &reinterpret_cast<EnemyManager *>(this)->enemies[0]);
    reinterpret_cast<EnemyManager *>(this)->activeEnemyCount = 0;
    for (enemyIndex = 0; enemyIndex < 480; ++enemyIndex, ++enemy)
    {
        if ((reinterpret_cast<Enemy *>(enemy)->flags1 & ENEMY_FLAG_ACTIVE) == 0)
        {
            if (g_Player.optionHomingTarget == reinterpret_cast<Enemy *>(enemy))
                g_Player.optionHomingTarget = 0;
            continue;
        }

        damageOccurred = 0;

        if (reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->suppressDeathEffects)
        {
            *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->worldPosition) =
                *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->position) +
                *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->positionOffset);
            reinterpret_cast<Enemy *>(enemy)->worldPosition.z = 0.0f;
            goto process_enemy_death;
        }

        ++reinterpret_cast<EnemyManager *>(this)->activeEnemyCount;

        if ((reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->pauseTimer &&
             (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_Player) + 0xFDC) || reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->playerType)) ||
            reinterpret_cast<EnemyFlag2Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags2)->forcePause)
        {
            reinterpret_cast<Enemy *>(enemy)->bossTimer--;
            goto update_damage_flash;
        }

    run_enemy_ecl:
        if (reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->linkedChild)
            reinterpret_cast<Enemy *>(enemy)->UpdateYoukaiAlignment();

    run_enemy_ecl_after_pause:
        if (g_EclManager.RunEcl(reinterpret_cast<Enemy *>(enemy)) == -1)
        {
            reinterpret_cast<Enemy *>(enemy)->flags1 &= ~1U;
            reinterpret_cast<Enemy *>(enemy)->Despawn();
            continue;
        }

        if (!reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->skipMovement)
        {
            reinterpret_cast<Enemy *>(enemy)->ClampPosition();
            reinterpret_cast<Enemy *>(enemy)->IntegrateVelocity();
            reinterpret_cast<Enemy *>(enemy)->ClampPosition();

            if (reinterpret_cast<Enemy *>(enemy)->parentEnemy != 0 &&
                reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->inheritParentPosition)
            {
                *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->positionOffset) =
                    *reinterpret_cast<D3DXVECTOR3 *>(
                        &reinterpret_cast<Enemy *>(enemy)->parentEnemy->position);
            }

            *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->worldPosition) =
                *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->position) +
                *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->positionOffset);
            reinterpret_cast<Enemy *>(enemy)->worldPosition.z = 0.0f;
        }
        else
        {
            *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->worldPosition) =
                *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->position) +
                *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->positionOffset);
            reinterpret_cast<Enemy *>(enemy)->worldPosition.z = 0.0f;
        }

        if (reinterpret_cast<Enemy *>(enemy)->alignmentEffect != 0)
        {
            *reinterpret_cast<D3DXVECTOR3 *>(
                &reinterpret_cast<Enemy *>(enemy)->alignmentEffect->vector0) =
                *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->worldPosition);
        }

        if (reinterpret_cast<Enemy *>(enemy)->trailFlags)
        {
            for (trailIndex = reinterpret_cast<Enemy *>(enemy)->trailHistoryLength - 1;
                 trailIndex > 0; --trailIndex)
            {
                *reinterpret_cast<D3DXVECTOR3 *>(
                    &reinterpret_cast<Enemy *>(enemy)->trailSamples[trailIndex].position) =
                    *reinterpret_cast<D3DXVECTOR3 *>(
                        &reinterpret_cast<Enemy *>(enemy)->trailSamples[trailIndex - 1].position);
                *reinterpret_cast<D3DXVECTOR3 *>(
                    &reinterpret_cast<Enemy *>(enemy)->trailSamples[trailIndex].velocity) =
                    *reinterpret_cast<D3DXVECTOR3 *>(
                        &reinterpret_cast<Enemy *>(enemy)->trailSamples[trailIndex - 1].velocity);
                reinterpret_cast<Enemy *>(enemy)->trailSamples[trailIndex].angle =
                    reinterpret_cast<Enemy *>(enemy)->trailSamples[trailIndex - 1].angle;
            }

            *reinterpret_cast<D3DXVECTOR3 *>(
                &reinterpret_cast<Enemy *>(enemy)->trailSamples[0].position) =
                *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->worldPosition);
            *reinterpret_cast<D3DXVECTOR3 *>(
                &reinterpret_cast<Enemy *>(enemy)->trailSamples[0].velocity) =
                *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->velocity);
            reinterpret_cast<Enemy *>(enemy)->trailSamples[0].angle =
                reinterpret_cast<Enemy *>(enemy)->movementAngle;
        }

        if (reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite == 0)
            reinterpret_cast<Enemy *>(enemy)->flags1 |= ENEMY_FLAG_NO_SPRITE;

        if (!reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->noSprite &&
            !reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->hasBeenInBounds &&
            g_GameManager.IsWithinPlayfield(
                (*reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->worldPosition))[0],
                (*reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->worldPosition))[1],
                reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->width,
                reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->height))
        {
            reinterpret_cast<Enemy *>(enemy)->flags1 |= ENEMY_FLAG_HAS_BEEN_IN_BOUNDS;
        }

        else if (reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->hasBeenInBounds == 1 &&
            !reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->allowOffscreen)
        {
            if ((!reinterpret_cast<Enemy *>(enemy)->trailFlags &&
                 !g_GameManager.IsWithinPlayfield(
                     reinterpret_cast<Enemy *>(enemy)->worldPosition.x,
                     reinterpret_cast<Enemy *>(enemy)->worldPosition.y,
                     reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->width,
                     reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->height)) ||
                (reinterpret_cast<Enemy *>(enemy)->trailFlags &&
                 !g_GameManager.IsWithinPlayfield(
                     reinterpret_cast<Enemy *>(enemy)->worldPosition.x,
                     reinterpret_cast<Enemy *>(enemy)->worldPosition.y,
                     reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->width,
                     reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->height) &&
                 !g_GameManager.IsWithinPlayfield(
                     reinterpret_cast<Enemy *>(enemy)->trailSamples[
                         reinterpret_cast<Enemy *>(enemy)->trailHistoryLength - 1].position.x,
                     reinterpret_cast<Enemy *>(enemy)->trailSamples[
                         reinterpret_cast<Enemy *>(enemy)->trailHistoryLength - 1].position.y,
                     reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->width,
                     reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->height)))
            {
                reinterpret_cast<Enemy *>(enemy)->flags1 &= ~1U;
                reinterpret_cast<Enemy *>(enemy)->Despawn();
                continue;
            }
        }

        if (reinterpret_cast<Enemy *>(enemy)->HandleLifeCallback())
            goto run_enemy_ecl_after_pause;
        if (reinterpret_cast<Enemy *>(enemy)->timerCallbackThresholdFrames >= 0 &&
            reinterpret_cast<Enemy *>(enemy)->HandleTimerCallback())
            goto run_enemy_ecl_after_pause;

        reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->color = reinterpret_cast<Enemy *>(enemy)->displayColor;
        g_AnmManager->ExecuteScript(reinterpret_cast<AnmVm *>(enemy->raw + 0xC));
        reinterpret_cast<Enemy *>(enemy)->displayColor = reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->color;
        for (trailIndex = 0; trailIndex < 2; ++trailIndex)
        {
            if (reinterpret_cast<EnemyManagerUpdateAnmVm *>(
                    enemy->raw + 0x2B0 + 0x2A4 * trailIndex)->scriptIndex >= 0 &&
                g_AnmManager->ExecuteScript(
                    reinterpret_cast<AnmVm *>(
                        enemy->raw + 0x2B0 + 0x2A4 * trailIndex)))
            {
                reinterpret_cast<EnemyManagerUpdateAnmVm *>(
                    enemy->raw + 0x2B0 + 0x2A4 * trailIndex)->scriptIndex = -1;
            }
        }

        bombHit = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_Player) + 0xFDC);
        if (!reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->noSprite &&
            !reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->hidePrimaryAnm &&
            !reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->youkaiAligned &&
            (!reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->noDamageDuringStop || !*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_Player) + 0xFDC)))
        {
            if (reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->collision)
            {
                reinterpret_cast<Enemy *>(enemy)->CheckPlayerCollision(
                    &reinterpret_cast<Enemy *>(enemy)->worldPosition,
                    &reinterpret_cast<Enemy *>(enemy)->hitboxDimensions);
                if (reinterpret_cast<Enemy *>(enemy)->trailFlags)
                {
                    secondaryHitbox = *reinterpret_cast<D3DXVECTOR3 *>(
                        &reinterpret_cast<Enemy *>(enemy)->hitboxDimensions);
                    for (trailIndex = 1; trailIndex < reinterpret_cast<Enemy *>(enemy)->trailCollisionLength; trailIndex += 6)
                    {
                        if (reinterpret_cast<Enemy *>(enemy)->trailFlags & ENEMY_TRAIL_TAPER)
                        {
                            secondaryHitbox = *reinterpret_cast<D3DXVECTOR3 *>(
                                                  &reinterpret_cast<Enemy *>(enemy)->hitboxDimensions) -
                                (*reinterpret_cast<D3DXVECTOR3 *>(
                                     &reinterpret_cast<Enemy *>(enemy)->hitboxDimensions) *
                                 (f32)trailIndex / (f32)reinterpret_cast<Enemy *>(enemy)->trailCollisionLength);
                        }
                        reinterpret_cast<Enemy *>(enemy)->CheckPlayerCollision(
                            &reinterpret_cast<Enemy *>(enemy)->trailSamples[trailIndex].position,
                            reinterpret_cast<Float3 *>(&secondaryHitbox));
                    }
                }
            }

            reinterpret_cast<Enemy *>(enemy)->lastDamage = 0;
            if (reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->acceptsDamage)
            {
                if (!g_Spellcard.IsActive() || !reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(enemy)->HasAttachedEnemy() ||
                    !*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_Player) + 0xFDC))
                {
                    damage = g_Player.FUN_00451670(
                        &reinterpret_cast<Enemy *>(enemy)->worldPosition,
                        &reinterpret_cast<Enemy *>(enemy)->hitboxDimensions,
                        &reinterpret_cast<Enemy *>(enemy)->playerShotHitAccumulator, &bombHit);
                }
                else
                {
                    damage = 0;
                }

                if (reinterpret_cast<Enemy *>(enemy)->secondaryHitboxDimensions.x > 0.0f)
                {
                    extraDamage = g_Player.FUN_00451670(
                        &reinterpret_cast<Enemy *>(enemy)->worldPosition,
                        &reinterpret_cast<Enemy *>(enemy)->secondaryHitboxDimensions,
                        &reinterpret_cast<Enemy *>(enemy)->playerShotHitAccumulator, &bombHit);
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
                    if ((reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->boss || !reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->mode) &&
                        !*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_Player) + 0xFDC))
                    {
                        if (reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->boss && !reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->mode)
                            rankAmount = 10 * (damage / (10 - difficultyScale / 3));
                        else
                            rankAmount = 10 * (damage / (30 - difficultyScale));
                        if (rankAmount > 70)
                            rankAmount = 70;
                        if (!rankAmount &&
                            (!reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->mode || ((i32)reinterpret_cast<Enemy *>(enemy)->bossTimer & 1)))
                            rankAmount = 10;
                    }

                    if (damage >= 70)
                        damage = 70;
                    g_GameManager.AddScore(10 * (damage / 5));

                    if (reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->damageable)
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

                        if (reinterpret_cast<Enemy *>(enemy)->damageReductionTimer > 0)
                        {
                            if (reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->boss)
                                damage /= 9;
                            else
                                damage = 0;
                        }

                        reinterpret_cast<Enemy *>(enemy)->life -= damage;
                        reinterpret_cast<Enemy *>(enemy)->lastDamage = damage;
                        reinterpret_cast<Enemy *>(enemy)->ApplyDamageToParent(damage);
                    }
                    damageOccurred = 1;
                }

                if (reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->boss)
                {
                    previousTargetDelta =
                        *reinterpret_cast<D3DXVECTOR3 *>(&g_Player.tailPosition0) -
                        (*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(&g_Player) + 0x2B4));
                    currentTargetDelta = *reinterpret_cast<D3DXVECTOR3 *>(
                                             &reinterpret_cast<Enemy *>(enemy)->worldPosition) -
                        (*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(&g_Player) + 0x2B4));
                    if (!g_Player.enemyTrackedPositionValid ||
                        fabsf(previousTargetDelta.x) >
                            fabsf(currentTargetDelta.x))
                    {
                        g_Player.tailPosition0 = reinterpret_cast<Enemy *>(enemy)->worldPosition;
                    }
                    g_Player.enemyTrackedPositionValid = 1;
                }

                if (!g_Player.enemyTrackedPositionValid &&
                    g_Player.tailPosition0[1] <
                        (*reinterpret_cast<D3DXVECTOR3 *>(
                            &reinterpret_cast<Enemy *>(enemy)->worldPosition))[1])
                {
                    g_Player.tailPosition0 = reinterpret_cast<Enemy *>(enemy)->worldPosition;
                }

                if (fabsf(
                        reinterpret_cast<Enemy *>(enemy)->worldPosition.x -
                        (*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(&g_Player) + 0x2B4)).x) < 64.0f &&
                    !reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(enemy)->HasAttachedEnemy() &&
                    (g_Player.optionHomingTarget == 0 ||
                     reinterpret_cast<Enemy *>(g_Player.optionHomingTarget)->position.y >
                         reinterpret_cast<Enemy *>(enemy)->worldPosition.y))
                {
                    g_Player.optionHomingTarget = reinterpret_cast<Enemy *>(enemy);
                }
            }
        }

        if (reinterpret_cast<EnemyFlag2Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags2)->deathLatch &&
            reinterpret_cast<Enemy *>(enemy)->life > 0)
            reinterpret_cast<Enemy *>(enemy)->flags2 &= ~ENEMY_FLAG2_DEATH_LATCH;

        if (reinterpret_cast<Enemy *>(enemy)->life <= 0 &&
            !reinterpret_cast<EnemyFlag2Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags2)->deathLatch &&
            !reinterpret_cast<EnemyFlag2Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags2)->noDeath)
        {
        process_enemy_death:
            ;
            // VC7 hashes nested locals by identifier.  These target-pinned
            // spellings carry, respectively, the bonus value, the otherwise
            // unused death-position vector, and the VM loop/gauge value.
            i32 deathVmIndex;

            reinterpret_cast<Enemy *>(enemy)->flags2 |= ENEMY_FLAG2_DEATH_LATCH;
            D3DXVECTOR3 bonus;
            i32 deathPosition;
            reinterpret_cast<Enemy *>(enemy)->phaseEndTimeRemainingSeconds =
                (reinterpret_cast<Enemy *>(enemy)->timerCallbackThresholdFrames -
                 (i32)reinterpret_cast<Enemy *>(enemy)->bossTimer) / 60;
            reinterpret_cast<Enemy *>(enemy)->timerCallbackThresholdFrames = -1;
            for (deathPosition = 0; deathPosition < 4; ++deathPosition)
                reinterpret_cast<Enemy *>(enemy)->lifeCallbackThresholds[deathPosition] = -1;
            for (deathPosition = 0; deathPosition < 4; ++deathPosition)
            {
                if (reinterpret_cast<Enemy *>(enemy)->childEclBlocks[deathPosition] != 0)
                {
                    g_ZunMemory.Free(reinterpret_cast<Enemy *>(enemy)->childEclBlocks[deathPosition]);
                    reinterpret_cast<Enemy *>(enemy)->childEclBlocks[deathPosition] = 0;
                }
            }

            if (reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(enemy)->HasAttachedEnemy())
            {
                --reinterpret_cast<Enemy *>(enemy)->parentEnemy->linkedChildCount;
            }

            reinterpret_cast<EclOperands::EnemyOverlay *>(enemy)->FUN_0042adb0(1);
            if (!reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->mode)
                deathPosition = -200;
            else
                deathPosition = 200;
            g_GameManager.AddToYoukaiGauge(deathPosition, 0);

            switch ((reinterpret_cast<Enemy *>(enemy)->flags1 >> 20) & 7)
            {
            case 3:
                reinterpret_cast<Enemy *>(enemy)->life = 1;
                reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_DAMAGEABLE;
                reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_DEATH_MODE_MASK;
                g_Gui.SetBossPresent(false);
                reinterpret_cast<EnemyManagerUpdateReplayManager *>(g_ReplayManager)->flags |= 0x20;
                if (reinterpret_cast<Enemy *>(enemy)->deathAnm1 >= 0)
                {
                    g_EffectManager.SpawnEffect(reinterpret_cast<Enemy *>(enemy)->deathAnm1, reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->worldPosition), 1, -1);
                    g_EffectManager.SpawnEffect(reinterpret_cast<Enemy *>(enemy)->deathAnm1, reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->worldPosition), 1, -1);
                    g_EffectManager.SpawnEffect(reinterpret_cast<Enemy *>(enemy)->deathAnm1, reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->worldPosition), 1, -1);
                }
                if (reinterpret_cast<Enemy *>(enemy)->alignmentEffect != 0)
                {
                    reinterpret_cast<Enemy *>(enemy)->alignmentEffect->vm.SetInterrupt(3);
                    reinterpret_cast<Enemy *>(enemy)->alignmentEffect = 0;
                }
                if (!reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->playerType)
                {
                    g_Player.timer = 90;
                    reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->playerType = 3;
                }
                reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_PAUSE_TIMER;
                reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_NO_DAMAGE_DURING_STOP;
                goto death_audio_and_callback;

            case 1:
                g_GameManager.AddScore(reinterpret_cast<Enemy *>(enemy)->score);
                reinterpret_cast<Enemy *>(enemy)->flags1 |= ENEMY_FLAG_PERSIST_AFTER_DEATH;
                reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_COLLISION;
                reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_DAMAGEABLE;
                reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_ACCEPTS_DAMAGE;
                goto common_death_mode;

            case 0:
                g_GameManager.AddScore(reinterpret_cast<Enemy *>(enemy)->score);
                reinterpret_cast<Enemy *>(enemy)->flags1 &= ~1U;
                if (reinterpret_cast<Enemy *>(enemy)->alignmentEffect != 0)
                {
                    reinterpret_cast<Enemy *>(enemy)->alignmentEffect->vm.SetInterrupt(3);
                    reinterpret_cast<Enemy *>(enemy)->alignmentEffect = 0;
                }
                goto common_death_mode;

            case 2:
            common_death_mode:
                if (reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->boss)
                {
                    g_Gui.SetBossPresent(false);
                    reinterpret_cast<Enemy *>(enemy)->ReleaseAttachedEffects();
                }
                reinterpret_cast<Enemy *>(enemy)->DropItems(bombHit);
                if (reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->boss &&
                    !g_Spellcard.IsActive())
                {
                    deathVmIndex = g_BulletManager.DespawnBullets(8000, 1);
                    deathVmIndex = g_EnemyManager.KillAllNonBossEnemies(8000, deathVmIndex);
                    if (deathVmIndex)
                    {
                        g_GameManager.AddScore(deathVmIndex);
                        g_Gui.FUN_00437ddd(deathVmIndex);
                    }
                }
                reinterpret_cast<Enemy *>(enemy)->life = 0;
                reinterpret_cast<EnemyManagerUpdateReplayManager *>(g_ReplayManager)->flags |= 0x20;
                break;
            }

        death_audio_and_callback:
            if (!reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->suppressDeathEffects)
            {
                g_SoundPlayer.PlaySoundPositionedByIdx(
                    static_cast<SoundIdx>(enemyIndex % 2 + 2), reinterpret_cast<Enemy *>(enemy)->worldPosition.x);
                if (reinterpret_cast<Enemy *>(enemy)->deathAnm1 >= 0)
                {
                    g_EffectManager.SpawnEffect(reinterpret_cast<Enemy *>(enemy)->deathAnm1, reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->worldPosition), 1, -1);
                    g_EffectManager.SpawnEffect(reinterpret_cast<Enemy *>(enemy)->deathAnm2 + 4, reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->worldPosition), 4, -1);
                }
                if (g_GameManager.GaugeIsExtremelyHuman() ||
                    g_GameManager.GaugeIsExtremelyYoukai())
                {
                    g_ItemManager.SpawnItem(&reinterpret_cast<Enemy *>(enemy)->worldPosition, static_cast<ItemType>(7), 1);
                }
            }

            if (reinterpret_cast<Enemy *>(enemy)->deathCallbackSubId >= 0)
            {
                i32 callbackVmIndex;

                reinterpret_cast<Enemy *>(enemy)->enemy_fun_00415c80();
                reinterpret_cast<Enemy *>(enemy)->activeEclCallStackDepth = 0;
                for (callbackVmIndex = 0; callbackVmIndex < 4; ++callbackVmIndex)
                    reinterpret_cast<Enemy *>(enemy)->lifeCallbackThresholds[callbackVmIndex] = -1;
                reinterpret_cast<Enemy *>(enemy)->timerCallbackThresholdFrames = -1;
                reinterpret_cast<Enemy *>(enemy)->ReleaseChildEclBlocks();
                memcpy(&reinterpret_cast<Enemy *>(enemy)->bulletSpawnDescriptor,
                       &g_EnemyManager.spawnTemplate.bulletSpawnDescriptor,
                       sizeof(BulletSpawnDescriptor));
                reinterpret_cast<Enemy *>(enemy)->shootIntervalFrames = 0;
                g_EclManager.CallEclSub(
                    reinterpret_cast<EnemyEclContext *>(
                        &reinterpret_cast<Enemy *>(enemy)->mainEclContextStorage),
                    reinterpret_cast<Enemy *>(enemy)->deathCallbackSubId);
                reinterpret_cast<Enemy *>(enemy)->deathCallbackSubId = -1;
            }
        }

    update_damage_flash:
        if (!reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->youkaiAligned)
        {
            if (reinterpret_cast<Enemy *>(enemy)->damageFlashTimer)
            {
                --reinterpret_cast<Enemy *>(enemy)->damageFlashTimer;
                reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->flags &= ~0x20000U;
            }
            else if (damageOccurred)
            {
                if (reinterpret_cast<EnemyFlag2Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags2)->damageFeedbackLevel < 2)
                    g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(20), reinterpret_cast<Enemy *>(enemy)->worldPosition.x);
                else
                    g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(37), reinterpret_cast<Enemy *>(enemy)->worldPosition.x);

                reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->unknown1F4)[2] = 0xFF;
                reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->unknown1F4)[1] = 0x60;
                reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->unknown1F4)[0] = 0x80;
                reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->unknown1F4)[3] =
                    reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->color)[3];
                reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->flags |= 0x20000U;
                reinterpret_cast<Enemy *>(enemy)->damageFlashTimer = 1;
            }
            else
            {
                reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->flags &= ~0x20000U;
            }
        }
        else
        {
            reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->unknown1F4)[2] = 0x20;
            reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->unknown1F4)[1] = 0x20;
            reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->unknown1F4)[0] = 0xC0;
            reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->unknown1F4)[3] =
                reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->color)[3] / 2;
            reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->flags |= 0x20000U;
        }

        if (reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->boss)
        {
            D3DXVECTOR3 markerPosition;

            if (!g_Gui.IsDialogPresent() && !reinterpret_cast<Enemy *>(enemy)->bossSlot)
                g_Gui.FUN_004230c0((f32)reinterpret_cast<Enemy *>(enemy)->life / (f32)reinterpret_cast<Enemy *>(enemy)->maxLife);

            if (reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->boss < 4)
            {
                if (!reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->noSprite)
                    markerPosition.x = reinterpret_cast<Enemy *>(enemy)->worldPosition.x + 32.0f;
                else
                    markerPosition.x = -999.0f;
                markerPosition.y = 472.0f;
                markerPosition.z = 0.0f;
                g_AsciiManager.SetBossMarkerPosition(reinterpret_cast<Enemy *>(enemy)->bossSlot, &markerPosition);

                if (reinterpret_cast<EnemyFlag2Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags2)->damageFeedbackLevel == 0)
                    g_AsciiManager.FUN_0042f2d0(
                        reinterpret_cast<Enemy *>(enemy)->bossSlot,
                        ((reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->flags >> 17) & 1) != 0);
                else
                    g_AsciiManager.FUN_0042f2d0(
                        reinterpret_cast<Enemy *>(enemy)->bossSlot,
                        reinterpret_cast<EnemyFlag2Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags2)->damageFeedbackLevel + 1);
            }
        }

        reinterpret_cast<Enemy *>(enemy)->UpdateEffects();
        if (!g_EclScriptedGlobalUpdateFreeze)
            reinterpret_cast<Enemy *>(enemy)->bossTimer++;
        if (reinterpret_cast<Enemy *>(enemy)->damageReductionTimer > 0)
            reinterpret_cast<Enemy *>(enemy)->damageReductionTimer--;

        if (!reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->noSprite &&
            reinterpret_cast<EnemyFlag1Bits *>(&reinterpret_cast<Enemy *>(enemy)->flags1)->active)
        {
            *reinterpret_cast<EnemyManagerUpdateEnemy **>(enemy->raw) =
                reinterpret_cast<EnemyManagerUpdateEnemy *>(
                    reinterpret_cast<EnemyManager *>(this)->drawGroupHeads[
                        reinterpret_cast<Enemy *>(enemy)->drawGroup]);
            reinterpret_cast<EnemyManager *>(this)->drawGroupHeads[
                reinterpret_cast<Enemy *>(enemy)->drawGroup] = reinterpret_cast<Enemy *>(enemy);
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
