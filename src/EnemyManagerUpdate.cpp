#include "inttypes.hpp"

#include <d3dx8math.h>
#include <math.h>
#include <string.h>

#pragma intrinsic(fabs)

namespace th08
{

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

    D3DXVECTOR3 *VelocityA() { return reinterpret_cast<D3DXVECTOR3 *>(raw + 0x2D34); }
    D3DXVECTOR3 *VelocityB() { return reinterpret_cast<D3DXVECTOR3 *>(raw + 0x2D40); }
    D3DXVECTOR3 *Position() { return reinterpret_cast<D3DXVECTOR3 *>(raw + 0x2D88); }
    D3DXVECTOR3 *Hitbox() { return reinterpret_cast<D3DXVECTOR3 *>(raw + 0x2D70); }
    D3DXVECTOR3 *SecondaryHitbox() { return reinterpret_cast<D3DXVECTOR3 *>(raw + 0x2D7C); }
    void *DamageData() { return raw + 0x2E10; }
    void *EclContext() { return raw + 0x7F8; }

    i16 &DeathCallbackSub() { return *reinterpret_cast<i16 *>(raw + 0x2CEE); }
    i16 &Unknown2CEA() { return *reinterpret_cast<i16 *>(raw + 0x2CEA); }
    i32 &Life() { return *reinterpret_cast<i32 *>(raw + 0x2DFC); }
    i32 &MaxLife() { return *reinterpret_cast<i32 *>(raw + 0x2E00); }
    i32 &Score() { return *reinterpret_cast<i32 *>(raw + 0x2E08); }
    EnemyManagerUpdateTimer *BossTimer() { return reinterpret_cast<EnemyManagerUpdateTimer *>(raw + 0x2E14); }
    u32 &DisplayColor() { return *reinterpret_cast<u32 *>(raw + 0x2E20); }
    u32 &Flags1() { return *reinterpret_cast<u32 *>(raw + 0x3324); }
    u32 &Flags2() { return *reinterpret_cast<u32 *>(raw + 0x3328); }
    i8 &DeathAnm1() { return *reinterpret_cast<i8 *>(raw + 0x3310); }
    u8 &DeathAnm2() { return *reinterpret_cast<u8 *>(raw + 0x3311); }
    u8 &BossSlot() { return *reinterpret_cast<u8 *>(raw + 0x3313); }
    u8 &DamageFlashTimer() { return *reinterpret_cast<u8 *>(raw + 0x3314); }
    u8 &DrawGroup() { return *reinterpret_cast<u8 *>(raw + 0x332F); }
    i32 &LastDamage() { return *reinterpret_cast<i32 *>(raw + 0x3354); }
    i32 *LifeCallbacks() { return reinterpret_cast<i32 *>(raw + 0x3358); }
    i32 &TimerCallbackThreshold() { return *reinterpret_cast<i32 *>(raw + 0x3378); }
    void **Effects() { return reinterpret_cast<void **>(raw + 0x3384); }
    u8 &TrailFlags() { return *reinterpret_cast<u8 *>(raw + 0x534C); }
    i16 &TrailHistoryCount() { return *reinterpret_cast<i16 *>(raw + 0x534E); }
    i16 &TrailSampleCount() { return *reinterpret_cast<i16 *>(raw + 0x5350); }
    EnemyManagerUpdateTimer *FreezeTimer() { return reinterpret_cast<EnemyManagerUpdateTimer *>(raw + 0x5354); }
    EnemyManagerUpdateAnmVm *&DeathVm()
    {
        return *reinterpret_cast<EnemyManagerUpdateAnmVm **>(raw + 0x53C8);
    }
    i32 &DeathTimeSeconds() { return *reinterpret_cast<i32 *>(raw + 0x53CC); }
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
    u8 playerType;
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

extern EnemyManagerUpdateEclManager g_EnemyManagerUpdateEclManager;
extern EnemyManagerUpdateGui g_EnemyManagerUpdateGui;
extern EnemyManagerUpdateGameManager g_EnemyManagerUpdateGameManager;
extern EnemyManagerUpdatePlayer g_EnemyManagerUpdatePlayer;
extern EnemyManagerUpdateAnmManager *g_EnemyManagerUpdateAnmManager;
extern EnemyManagerUpdateSpellcard g_EnemyManagerUpdateSpellcard;
extern EnemyManagerUpdateEffectManager g_EnemyManagerUpdateEffectManager;
extern EnemyManagerUpdateBulletManager g_EnemyManagerUpdateBulletManager;
extern EnemyManagerUpdateItemManager g_EnemyManagerUpdateItemManager;
extern EnemyManagerUpdateSoundPlayer g_EnemyManagerUpdateSoundPlayer;
extern EnemyManagerUpdateAsciiManager g_EnemyManagerUpdateAsciiManager;
extern EnemyManagerUpdateReplayManager *g_EnemyManagerUpdateReplayManager;
extern EnemyManagerUpdateMemory g_EnemyManagerUpdateMemory;

extern u32 g_EnemyManagerUpdateManagerFlags;
extern i32 g_EnemyManagerUpdateFrameCounter;
extern i32 g_EnemyManagerUpdateHumanCounter;
extern i32 g_EnemyManagerUpdateYoukaiCounter;
extern i32 g_EnemyManagerUpdateFrameStop;
extern i8 g_EnemyManagerUpdateGamePaused;
extern EnemyManagerUpdateTimer g_EnemyManagerUpdatePlayerTimer;
extern D3DXVECTOR3 g_EnemyManagerUpdateReferencePosition;
extern D3DXVECTOR3 g_EnemyManagerUpdateTrackedPosition;
extern i32 g_EnemyManagerUpdateTrackedPositionValid;
extern EnemyManagerUpdateEnemy *g_EnemyManagerUpdateTrackedEnemy;
extern u8 g_EnemyManagerUpdateCombatTemplate[0x210];

struct EnemyManagerUpdateOverlay
{
    u8 raw[1];

    EnemyManagerUpdateEnemy *Enemies()
    {
        return reinterpret_cast<EnemyManagerUpdateEnemy *>(raw + 0x53D0);
    }

    EnemyManagerUpdateEnemy *&SpecialEnemy()
    {
        return *reinterpret_cast<EnemyManagerUpdateEnemy **>(raw + 0x9DCDA0);
    }

    i32 &EnemyCount() { return *reinterpret_cast<i32 *>(raw + 0x9DCDC4); }
    EnemyManagerUpdateTimelineLane *TimelineLanes()
    {
        return reinterpret_cast<EnemyManagerUpdateTimelineLane *>(raw + 0x9DCDD0);
    }
    EnemyManagerUpdateTimer *Timer()
    {
        return reinterpret_cast<EnemyManagerUpdateTimer *>(raw + 0x9DCED0);
    }
    EnemyManagerUpdateEnemy **DrawHeads()
    {
        return reinterpret_cast<EnemyManagerUpdateEnemy **>(raw + 0x9DCEDC);
    }

    void PrepareFrame();
    i32 ConvertBulletBonus(i32 base, i32 bullets);
    i32 OnUpdate();
};

enum EnemyManagerUpdateFlag1
{
    EMUF1_ACTIVE = 1U << 0,
    EMUF1_BOSS = 1U << 1,
    EMUF1_COLLISION = 1U << 2,
    EMUF1_DAMAGEABLE = 1U << 3,
    EMUF1_NO_SPRITE = 1U << 4,
    EMUF1_SKIP_COMBAT_A = 1U << 5,
    EMUF1_ACCEPTS_DAMAGE = 1U << 6,
    EMUF1_PRE_ECL_UPDATE = 1U << 8,
    EMUF1_COPY_FOLLOW_VELOCITY = 1U << 9,
    EMUF1_SUPPRESS_DEATH_EFFECTS = 1U << 10,
    EMUF1_SKIP_COMBAT_B = 1U << 11,
    EMUF1_DEATH_MODE_MASK = 7U << 20,
    EMUF1_PERSIST_AFTER_DEATH = 1U << 23,
    EMUF1_HAS_BEEN_IN_BOUNDS = 1U << 24,
    EMUF1_ALLOW_OFFSCREEN = 1U << 28,
    EMUF1_SKIP_MOVEMENT = 1U << 29,
    EMUF1_PAUSE_TIMER = 1U << 30,
    EMUF1_NO_DAMAGE_DURING_STOP = 1U << 31
};

enum EnemyManagerUpdateFlag2
{
    EMUF2_DEATH_LATCH = 1U << 3,
    EMUF2_BOSS_MARKER_MASK = 3U << 4,
    EMUF2_NO_DEATH = 1U << 6,
    EMUF2_FORCE_PAUSE = 1U << 7
};

static __forceinline D3DXVECTOR3 *TrailPosition(EnemyManagerUpdateEnemy *enemy, i32 index)
{
    return reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x3394 + 0x1C * index);
}

static __forceinline D3DXVECTOR3 *TrailVelocity(EnemyManagerUpdateEnemy *enemy, i32 index)
{
    return reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x33A0 + 0x1C * index);
}

static __forceinline u32 &TrailValue(EnemyManagerUpdateEnemy *enemy, i32 index)
{
    return *reinterpret_cast<u32 *>(enemy->raw + 0x33AC + 0x1C * index);
}

static __forceinline void PushTrailSample(EnemyManagerUpdateEnemy *enemy)
{
    i32 index;

    if (!enemy->TrailFlags())
        return;

    for (index = enemy->TrailHistoryCount() - 1; index > 0; --index)
    {
        *TrailPosition(enemy, index) = *TrailPosition(enemy, index - 1);
        *TrailVelocity(enemy, index) = *TrailVelocity(enemy, index - 1);
        TrailValue(enemy, index) = TrailValue(enemy, index - 1);
    }

    *TrailPosition(enemy, 0) = *enemy->Position();
    *TrailVelocity(enemy, 0) = *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D4C);
    TrailValue(enemy, 0) = *reinterpret_cast<u32 *>(enemy->raw + 0x2D94);
}

static __forceinline i32 EnemyIsInBounds(EnemyManagerUpdateEnemy *enemy)
{
    EnemyManagerUpdateSprite *sprite = enemy->PrimaryVm()->sprite;
    return g_EnemyManagerUpdateGameManager.IsWithinPlayfield(
        enemy->Position()->x, enemy->Position()->y, sprite->width, sprite->height);
}

static __forceinline i32 EnemyTrailIsInBounds(EnemyManagerUpdateEnemy *enemy)
{
    EnemyManagerUpdateSprite *sprite = enemy->PrimaryVm()->sprite;
    D3DXVECTOR3 *sample = TrailPosition(enemy, enemy->TrailHistoryCount() - 1);
    return g_EnemyManagerUpdateGameManager.IsWithinPlayfield(
        sample->x, sample->y, sprite->width, sprite->height);
}

static __forceinline void ResetEnemyCombatState(EnemyManagerUpdateEnemy *enemy)
{
    i32 index;

    enemy->Unknown2CEA() = 0;
    for (index = 0; index < 4; ++index)
        enemy->LifeCallbacks()[index] = -1;
    enemy->TimerCallbackThreshold() = -1;
    enemy->ResetForDeathCallback();
    memcpy(enemy->raw + 0x2E24, g_EnemyManagerUpdateCombatTemplate, 0x210);
    *reinterpret_cast<i32 *>(enemy->raw + 0x3060) = 0;
}

static __forceinline void TrackBossTarget(EnemyManagerUpdateEnemy *enemy)
{
    D3DXVECTOR3 previousDelta;
    D3DXVECTOR3 currentDelta;

    if (enemy->Flags1() & EMUF1_BOSS)
    {
        previousDelta = g_EnemyManagerUpdateTrackedPosition - g_EnemyManagerUpdateReferencePosition;
        currentDelta = *enemy->Position() - g_EnemyManagerUpdateReferencePosition;
        if (!g_EnemyManagerUpdateTrackedPositionValid ||
            fabs(previousDelta.x) > fabs(currentDelta.x))
        {
            g_EnemyManagerUpdateTrackedPosition = *enemy->Position();
        }
        g_EnemyManagerUpdateTrackedPositionValid = 1;
    }

    if (!g_EnemyManagerUpdateTrackedPositionValid &&
        g_EnemyManagerUpdateTrackedPosition.y < enemy->Position()->y)
    {
        g_EnemyManagerUpdateTrackedPosition = *enemy->Position();
    }

    if (fabs(enemy->Position()->x - g_EnemyManagerUpdateReferencePosition.x) <= 64.0f &&
        !enemy->IsBossPart() &&
        (g_EnemyManagerUpdateTrackedEnemy == 0 ||
         g_EnemyManagerUpdateTrackedEnemy->VelocityA()->y <= enemy->Position()->y))
    {
        g_EnemyManagerUpdateTrackedEnemy = enemy;
    }
}

#pragma var_order(bombHit, difficultyScale, enemyIndex, damage, extraDamage, vmIndex, trailIndex, enemy)
i32 EnemyManagerUpdateOverlay::OnUpdate()
{
    EnemyManagerUpdateEnemy *enemy;
    D3DXVECTOR3 secondaryHitbox;
    D3DXVECTOR3 markerPosition;
    D3DXVECTOR3 lowerBounds;
    D3DXVECTOR3 upperBounds;
    i32 damageOccurred;
    i32 trailIndex;
    i32 vmIndex;
    i32 extraDamage;
    i32 damage;
    i32 enemyIndex;
    i32 difficultyScale;
    i32 bombHit;

    bombHit = 0;
    difficultyScale = 10;

    if (!g_EnemyManagerUpdateGui.IsDialogPresent())
    {
        ++g_EnemyManagerUpdateFrameCounter;
        if ((i32)*Timer() >= 16)
        {
            ++g_EnemyManagerUpdateHumanCounter;
            if (!g_EnemyManagerUpdatePlayer.mode)
                ++g_EnemyManagerUpdateYoukaiCounter;
        }
    }

    if (g_EnemyManagerUpdateManagerFlags & (1U << 10))
        return 1;

    if ((g_EnemyManagerUpdateManagerFlags & (1U << 13)) && SpecialEnemy() != 0)
    {
        lowerBounds = D3DXVECTOR3(192.0f, 224.0f, 0.0f);
        upperBounds = D3DXVECTOR3(384.0f, 448.0f, 0.0f);
        g_EnemyManagerUpdatePlayer.CalcDamageToEnemy(
            &lowerBounds, &upperBounds, SpecialEnemy()->DamageData(), &bombHit);
    }

    PrepareFrame();

    for (enemyIndex = 0; enemyIndex < 4; ++enemyIndex)
        DrawHeads()[enemyIndex] = 0;

    for (enemyIndex = 0; enemyIndex < g_EnemyManagerUpdateEclManager.GetTimelineCount(); ++enemyIndex)
    {
        if (TimelineLanes()[enemyIndex].instruction == 0)
            TimelineLanes()[enemyIndex].instruction = g_EnemyManagerUpdateEclManager.GetTimeline(enemyIndex);
        TimelineLanes()[enemyIndex].Run();
    }

    enemy = Enemies();
    EnemyCount() = 0;
    for (enemyIndex = 0; enemyIndex < 480; ++enemyIndex, ++enemy)
    {
        if (!enemy->IsActive())
        {
            if (g_EnemyManagerUpdateTrackedEnemy == enemy)
                g_EnemyManagerUpdateTrackedEnemy = 0;
            continue;
        }

        damageOccurred = 0;

        if (enemy->Flags1() & EMUF1_SUPPRESS_DEATH_EFFECTS)
        {
            *enemy->Position() = *enemy->VelocityA() + *enemy->VelocityB();
            enemy->Position()->z = 0.0f;
            goto process_enemy_death;
        }

        ++EnemyCount();

        if (((enemy->Flags1() & EMUF1_PAUSE_TIMER) &&
             (g_EnemyManagerUpdateFrameStop || g_EnemyManagerUpdatePlayer.playerType)) ||
            (enemy->Flags2() & EMUF2_FORCE_PAUSE))
        {
            (*enemy->BossTimer())--;
            goto update_damage_flash;
        }

    run_enemy_ecl:
        if (enemy->Flags1() & EMUF1_PRE_ECL_UPDATE)
            enemy->PauseUpdate();

        if (g_EnemyManagerUpdateEclManager.RunEcl(enemy) == -1)
        {
            enemy->ClearActive();
            enemy->Despawn();
            continue;
        }

        if ((enemy->Flags1() & EMUF1_SKIP_MOVEMENT) == 0)
        {
            enemy->ClampPosition();
            enemy->Move();
            enemy->ClampPosition();
        }

        if (*reinterpret_cast<void **>(enemy->raw + 0x2DA4) != 0 &&
            (enemy->Flags1() & EMUF1_COPY_FOLLOW_VELOCITY))
        {
            EnemyManagerUpdateEnemy *follow =
                *reinterpret_cast<EnemyManagerUpdateEnemy **>(enemy->raw + 0x2DA4);
            *follow->VelocityA() = *enemy->VelocityB();
        }

        *enemy->Position() = *enemy->VelocityA() + *enemy->VelocityB();
        enemy->Position()->z = 0.0f;

        if (enemy->DeathVm() != 0)
        {
            u8 *follow = reinterpret_cast<u8 *>(enemy->DeathVm());
            *reinterpret_cast<D3DXVECTOR3 *>(follow + 0x2A4) = *enemy->Position();
        }

        PushTrailSample(enemy);

        if (enemy->PrimaryVm()->sprite == 0)
            enemy->Flags1() |= EMUF1_NO_SPRITE;

        if ((enemy->Flags1() & EMUF1_NO_SPRITE) == 0 &&
            (enemy->Flags1() & EMUF1_HAS_BEEN_IN_BOUNDS) == 0 &&
            EnemyIsInBounds(enemy))
        {
            enemy->Flags1() |= EMUF1_HAS_BEEN_IN_BOUNDS;
        }

        if ((enemy->Flags1() & EMUF1_HAS_BEEN_IN_BOUNDS) &&
            (enemy->Flags1() & EMUF1_ALLOW_OFFSCREEN) == 0)
        {
            if ((!enemy->TrailFlags() && !EnemyIsInBounds(enemy)) ||
                (enemy->TrailFlags() && !EnemyIsInBounds(enemy) && !EnemyTrailIsInBounds(enemy)))
            {
                enemy->ClearActive();
                enemy->Despawn();
                continue;
            }
        }

        if (enemy->HandleLifeCallback())
            goto run_enemy_ecl;
        if (enemy->TimerCallbackThreshold() >= 0 && enemy->HandleTimerCallback())
            goto run_enemy_ecl;

        enemy->PrimaryVm()->color = enemy->DisplayColor();
        g_EnemyManagerUpdateAnmManager->ExecuteScript(enemy->PrimaryVm());
        enemy->DisplayColor() = enemy->PrimaryVm()->color;
        for (vmIndex = 0; vmIndex < 2; ++vmIndex)
        {
            if (enemy->ExtraVm(vmIndex)->scriptIndex >= 0 &&
                g_EnemyManagerUpdateAnmManager->ExecuteScript(enemy->ExtraVm(vmIndex)))
            {
                enemy->ExtraVm(vmIndex)->scriptIndex = -1;
            }
        }

        bombHit = g_EnemyManagerUpdateFrameStop;
        if ((enemy->Flags1() & (EMUF1_NO_SPRITE | EMUF1_SKIP_COMBAT_A | EMUF1_SKIP_COMBAT_B)) == 0 &&
            ((enemy->Flags1() & EMUF1_NO_DAMAGE_DURING_STOP) == 0 || !g_EnemyManagerUpdateFrameStop))
        {
            if (enemy->Flags1() & EMUF1_COLLISION)
            {
                enemy->CheckPlayerCollision(enemy->Position(), enemy->Hitbox());
                if (enemy->TrailFlags())
                {
                    secondaryHitbox = *enemy->Hitbox();
                    for (trailIndex = 1; trailIndex < enemy->TrailSampleCount(); trailIndex += 6)
                    {
                        if (enemy->TrailFlags() & 2)
                        {
                            secondaryHitbox = *enemy->Hitbox() -
                                (*enemy->Hitbox() * (f32)trailIndex / (f32)enemy->TrailSampleCount());
                        }
                        enemy->CheckPlayerCollision(TrailPosition(enemy, trailIndex), &secondaryHitbox);
                    }
                }
            }

            enemy->LastDamage() = 0;
            if (enemy->Flags1() & EMUF1_ACCEPTS_DAMAGE)
            {
                if (!g_EnemyManagerUpdateSpellcard.IsActive() || !enemy->IsBossPart() ||
                    !g_EnemyManagerUpdateFrameStop)
                {
                    damage = g_EnemyManagerUpdatePlayer.CalcDamageToEnemy(
                        enemy->Position(), enemy->Hitbox(), enemy->DamageData(), &bombHit);
                }
                else
                {
                    damage = 0;
                }

                if (enemy->SecondaryHitbox()->x > 0.0f)
                {
                    extraDamage = g_EnemyManagerUpdatePlayer.CalcDamageToEnemy(
                        enemy->Position(), enemy->SecondaryHitbox(), enemy->DamageData(), &bombHit);
                    if (!bombHit)
                    {
                        if (g_EnemyManagerUpdatePlayer.playerType == 3 ||
                            g_EnemyManagerUpdatePlayer.playerType == 11)
                            damage += (i32)((f32)extraDamage / 6.5f);
                        else
                            damage += (i32)((f32)extraDamage / 1.7f);
                    }
                }

                if (damage > 0)
                {
                    i32 rankAmount = 0;
                    if (((enemy->Flags1() & EMUF1_BOSS) || !g_EnemyManagerUpdatePlayer.mode) &&
                        !g_EnemyManagerUpdateFrameStop)
                    {
                        if ((enemy->Flags1() & EMUF1_BOSS) && !g_EnemyManagerUpdatePlayer.mode)
                            rankAmount = 10 * (damage / (10 - difficultyScale / 3));
                        else
                            rankAmount = 10 * (damage / (30 - difficultyScale));
                        if (rankAmount > 70)
                            rankAmount = 70;
                        if (!rankAmount &&
                            (!g_EnemyManagerUpdatePlayer.mode || ((i32)*enemy->BossTimer() & 1)))
                            rankAmount = 10;
                    }

                    if (damage >= 70)
                        damage = 70;
                    g_EnemyManagerUpdateGameManager.AddScore(10 * (damage / 5));

                    if (enemy->Flags1() & EMUF1_DAMAGEABLE)
                    {
                        if (g_EnemyManagerUpdateSpellcard.IsActive())
                        {
                            if (!bombHit)
                                damage = damage > 7 ? damage / 7 : (damage != 0);
                            else if (g_EnemyManagerUpdateSpellcard.AllowsBombDamage() && !enemy->IsBossPart())
                                damage = damage > 2 ? (i32)((f32)damage / 2.5f) : (damage != 0);
                            else
                                damage = 0;
                        }

                        if (*enemy->FreezeTimer() > 0)
                            damage = (enemy->Flags1() & EMUF1_BOSS) ? damage / 9 : 0;

                        enemy->Life() -= damage;
                        enemy->LastDamage() = damage;
                        enemy->ApplyDamage(damage);
                    }
                    damageOccurred = 1;
                }
            }
        }

        TrackBossTarget(enemy);

        if ((enemy->Flags2() & EMUF2_DEATH_LATCH) && enemy->Life() > 0)
            enemy->Flags2() &= ~EMUF2_DEATH_LATCH;

        if (enemy->Life() <= 0 &&
            (enemy->Flags2() & (EMUF2_DEATH_LATCH | EMUF2_NO_DEATH)) == 0)
        {
        process_enemy_death:
            ;
            i32 deathMode;
            i32 bullets;
            i32 bonus;

            enemy->Flags2() |= EMUF2_DEATH_LATCH;
            enemy->DeathTimeSeconds() =
                (enemy->TimerCallbackThreshold() - (i32)*enemy->BossTimer()) / 60;
            enemy->TimerCallbackThreshold() = -1;
            for (vmIndex = 0; vmIndex < 4; ++vmIndex)
                enemy->LifeCallbacks()[vmIndex] = -1;
            for (vmIndex = 0; vmIndex < 4; ++vmIndex)
            {
                if (enemy->Effects()[vmIndex] != 0)
                {
                    g_EnemyManagerUpdateMemory.Free(enemy->Effects()[vmIndex]);
                    enemy->Effects()[vmIndex] = 0;
                }
            }

            if (enemy->IsBossPart())
            {
                EnemyManagerUpdateEnemy *owner =
                    *reinterpret_cast<EnemyManagerUpdateEnemy **>(enemy->raw + 0x2DA4);
                --*reinterpret_cast<i32 *>(owner->raw + 0x3380);
            }

            enemy->ReleaseEffects(1);
            g_EnemyManagerUpdateGameManager.AddToYoukaiGauge(
                g_EnemyManagerUpdatePlayer.mode ? 200 : -200, 0);

            deathMode = (enemy->Flags1() & EMUF1_DEATH_MODE_MASK) >> 20;
            switch (deathMode)
            {
            case 3:
                enemy->Life() = 1;
                enemy->Flags1() &= ~EMUF1_DAMAGEABLE;
                enemy->Flags1() &= ~EMUF1_DEATH_MODE_MASK;
                g_EnemyManagerUpdateGui.ResetBossUi(0);
                g_EnemyManagerUpdateReplayManager->flags |= 0x20;
                if (enemy->DeathAnm1() >= 0)
                {
                    g_EnemyManagerUpdateEffectManager.SpawnEffect(enemy->DeathAnm1(), enemy->Position(), 1, -1);
                    g_EnemyManagerUpdateEffectManager.SpawnEffect(enemy->DeathAnm1(), enemy->Position(), 1, -1);
                    g_EnemyManagerUpdateEffectManager.SpawnEffect(enemy->DeathAnm1(), enemy->Position(), 1, -1);
                }
                if (enemy->DeathVm() != 0)
                {
                    enemy->DeathVm()->SetInterrupt(3);
                    enemy->DeathVm() = 0;
                }
                if (!g_EnemyManagerUpdatePlayer.playerType)
                {
                    g_EnemyManagerUpdatePlayerTimer = 90;
                    g_EnemyManagerUpdatePlayer.playerType = 3;
                }
                enemy->Flags1() &= ~EMUF1_PAUSE_TIMER;
                enemy->Flags1() &= ~EMUF1_NO_DAMAGE_DURING_STOP;
                goto death_audio_and_callback;

            case 1:
                g_EnemyManagerUpdateGameManager.AddScore(enemy->Score());
                enemy->Flags1() |= EMUF1_PERSIST_AFTER_DEATH;
                enemy->Flags1() &= ~(EMUF1_COLLISION | EMUF1_DAMAGEABLE | EMUF1_ACCEPTS_DAMAGE);
                goto common_death_mode;

            case 0:
                g_EnemyManagerUpdateGameManager.AddScore(enemy->Score());
                enemy->ClearActive();
                if (enemy->DeathVm() != 0)
                {
                    enemy->DeathVm()->SetInterrupt(3);
                    enemy->DeathVm() = 0;
                }
                goto common_death_mode;

            case 2:
            common_death_mode:
                if (enemy->Flags1() & EMUF1_BOSS)
                {
                    g_EnemyManagerUpdateGui.ResetBossUi(0);
                    enemy->SpecialBossDeath();
                }
                enemy->FinalizeDeath(bombHit);
                if ((enemy->Flags1() & EMUF1_BOSS) && !g_EnemyManagerUpdateSpellcard.IsActive())
                {
                    bullets = g_EnemyManagerUpdateBulletManager.DespawnBullets(8000, 1);
                    bonus = ConvertBulletBonus(8000, bullets);
                    if (bonus)
                    {
                        g_EnemyManagerUpdateGameManager.AddScore(bonus);
                        g_EnemyManagerUpdateGui.ShowBonus(bonus);
                    }
                }
                enemy->Life() = 0;
                g_EnemyManagerUpdateReplayManager->flags |= 0x20;
                break;
            }

        death_audio_and_callback:
            if ((enemy->Flags1() & EMUF1_SUPPRESS_DEATH_EFFECTS) == 0)
            {
                g_EnemyManagerUpdateSoundPlayer.PlaySoundPositionedByIdx(
                    enemyIndex % 2 + 2, enemy->Position()->x);
                if (enemy->DeathAnm1() >= 0)
                {
                    g_EnemyManagerUpdateEffectManager.SpawnEffect(enemy->DeathAnm1(), enemy->Position(), 1, -1);
                    g_EnemyManagerUpdateEffectManager.SpawnEffect(enemy->DeathAnm2() + 4, enemy->Position(), 4, -1);
                }
                if (g_EnemyManagerUpdateGameManager.GaugeIsExtremelyHuman() ||
                    g_EnemyManagerUpdateGameManager.GaugeIsExtremelyYoukai())
                {
                    g_EnemyManagerUpdateItemManager.SpawnItem(enemy->Position(), 7, 1);
                }
            }

            if (enemy->DeathCallbackSub() >= 0)
            {
                enemy->PrepareDeathCallback();
                ResetEnemyCombatState(enemy);
                g_EnemyManagerUpdateEclManager.CallEclSub(enemy->EclContext(), enemy->DeathCallbackSub());
                enemy->DeathCallbackSub() = -1;
            }
        }

    update_damage_flash:
        if ((enemy->Flags1() & EMUF1_SKIP_COMBAT_B) == 0)
        {
            if (enemy->DamageFlashTimer())
            {
                --enemy->DamageFlashTimer();
                enemy->PrimaryVm()->flags &= ~0x20000U;
            }
            else if (damageOccurred)
            {
                if (((enemy->Flags2() & EMUF2_BOSS_MARKER_MASK) >> 4) < 2)
                    g_EnemyManagerUpdateSoundPlayer.PlaySoundPositionedByIdx(20, enemy->Position()->x);
                else
                    g_EnemyManagerUpdateSoundPlayer.PlaySoundPositionedByIdx(37, enemy->Position()->x);

                reinterpret_cast<u8 *>(&enemy->PrimaryVm()->color)[2] = 0xFF;
                reinterpret_cast<u8 *>(&enemy->PrimaryVm()->color)[1] = 0x60;
                reinterpret_cast<u8 *>(&enemy->PrimaryVm()->color)[0] = 0x80;
                reinterpret_cast<u8 *>(&enemy->PrimaryVm()->color)[3] =
                    reinterpret_cast<u8 *>(&enemy->DisplayColor())[3];
                enemy->PrimaryVm()->flags |= 0x20000U;
                enemy->DamageFlashTimer() = 1;
            }
            else
            {
                enemy->PrimaryVm()->flags &= ~0x20000U;
            }
        }
        else
        {
            reinterpret_cast<u8 *>(&enemy->PrimaryVm()->color)[2] = 0x20;
            reinterpret_cast<u8 *>(&enemy->PrimaryVm()->color)[1] = 0x20;
            reinterpret_cast<u8 *>(&enemy->PrimaryVm()->color)[0] = 0xC0;
            reinterpret_cast<u8 *>(&enemy->PrimaryVm()->color)[3] =
                reinterpret_cast<u8 *>(&enemy->DisplayColor())[3] / 2;
            enemy->PrimaryVm()->flags |= 0x20000U;
        }

        if (enemy->Flags1() & EMUF1_BOSS)
        {
            if (!g_EnemyManagerUpdateGui.IsDialogPresent() && !enemy->BossSlot())
                g_EnemyManagerUpdateGui.SetBossHealth((f32)enemy->Life() / (f32)enemy->MaxLife());

            markerPosition.x = (enemy->Flags1() & EMUF1_NO_SPRITE) ? -999.0f : enemy->Position()->x + 32.0f;
            markerPosition.y = 472.0f;
            markerPosition.z = 0.0f;
            g_EnemyManagerUpdateAsciiManager.SetBossMarkerPosition(enemy->BossSlot(), &markerPosition);

            if ((enemy->Flags2() & EMUF2_BOSS_MARKER_MASK) == 0)
                g_EnemyManagerUpdateAsciiManager.SetBossMarkerState(
                    enemy->BossSlot(), (enemy->PrimaryVm()->flags & 0x20000U) != 0);
            else
                g_EnemyManagerUpdateAsciiManager.SetBossMarkerState(
                    enemy->BossSlot(), ((enemy->Flags2() & EMUF2_BOSS_MARKER_MASK) >> 4) + 1);
        }

        enemy->UpdateEffects();
        if (!g_EnemyManagerUpdateGamePaused)
            (*enemy->BossTimer())++;
        if (*enemy->FreezeTimer() > 0)
            (*enemy->FreezeTimer())--;

        if ((enemy->Flags1() & EMUF1_NO_SPRITE) == 0 && enemy->IsActive())
        {
            enemy->DrawNext() = DrawHeads()[enemy->DrawGroup()];
            DrawHeads()[enemy->DrawGroup()] = enemy;
        }
    }

    if ((*Timer() % 200) == 0 && g_EnemyManagerUpdateGameManager.IsTampered())
        return 4;

    (*Timer())++;
    return 1;
}

} // namespace th08
