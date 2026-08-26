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

    D3DXVECTOR3 *VelocityA() { return reinterpret_cast<D3DXVECTOR3 *>(raw + 0x2D34); }
    D3DXVECTOR3 *VelocityB() { return reinterpret_cast<D3DXVECTOR3 *>(raw + 0x2D40); }
    D3DXVECTOR3 *Position() { return reinterpret_cast<D3DXVECTOR3 *>(raw + 0x2D88); }
    D3DXVECTOR3 *Hitbox() { return reinterpret_cast<D3DXVECTOR3 *>(raw + 0x2D70); }
    D3DXVECTOR3 *SecondaryHitbox() { return reinterpret_cast<D3DXVECTOR3 *>(raw + 0x2D7C); }
    void *DamageData() { return &reinterpret_cast<Enemy *>(this)->playerShotHitAccumulator; }
    void *EclContext() { return raw + 0x7F8; }

    i16 &DeathCallbackSub() { return *reinterpret_cast<i16 *>(raw + 0x2CEE); }
    i16 &Unknown2CEA() { return *reinterpret_cast<i16 *>(raw + 0x2CEA); }
    i32 &Life() { return reinterpret_cast<Enemy *>(this)->life; }
    i32 &MaxLife() { return reinterpret_cast<Enemy *>(this)->maxLife; }
    i32 &Score() { return reinterpret_cast<Enemy *>(this)->score; }
    EnemyManagerUpdateTimer *BossTimer() { return reinterpret_cast<EnemyManagerUpdateTimer *>(&reinterpret_cast<Enemy *>(this)->bossTimer); }
    D3DCOLOR &DisplayColor() { return reinterpret_cast<Enemy *>(this)->displayColor; }
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

struct EnemyManagerUpdateManagerFlagBits
{
    u32 unknown00_09 : 10;
    u32 skipUpdate : 1;
    u32 unknown11_12 : 2;
    u32 damageBoss : 1;
    u32 unknown14_31 : 18;
};

struct EnemyManagerUpdateFlag1Bits
{
    u32 active : 1;
    u32 boss : 1;
    u32 collision : 1;
    u32 damageable : 1;
    u32 noSprite : 1;
    u32 skipCombatA : 1;
    u32 acceptsDamage : 1;
    u32 unknown07 : 1;
    u32 preEclUpdate : 1;
    u32 copyFollowVelocity : 1;
    u32 suppressDeathEffects : 1;
    u32 skipCombatB : 1;
    u32 unknown12_19 : 8;
    u32 deathMode : 3;
    u32 persistAfterDeath : 1;
    u32 hasBeenInBounds : 1;
    u32 unknown25_27 : 3;
    u32 allowOffscreen : 1;
    u32 skipMovement : 1;
    u32 pauseTimer : 1;
    u32 noDamageDuringStop : 1;
};

struct EnemyManagerUpdateFlag2Bits
{
    u32 unknown00_02 : 3;
    u32 deathLatch : 1;
    u32 bossMarker : 2;
    u32 noDeath : 1;
    u32 forcePause : 1;
    u32 unknown08_31 : 24;
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
        if ((i32)*reinterpret_cast<ZunTimer *>(raw + 0x9DCED0) >= 16)
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
        *reinterpret_cast<EnemyManagerUpdateEnemy **>(raw + 0x9DCDA0) != 0)
    {
        // These target-pinned identifiers preserve VC7's block-local hash
        // order.  The initializer values and argument order carry the actual
        // outer/inner semantics.
        D3DXVECTOR3 lowerBounds(384.0f, 448.0f, 0.0f);
        D3DXVECTOR3 upperBounds(192.0f, 224.0f, 0.0f);
        g_Player.FUN_00451670(
            reinterpret_cast<Float3 *>(&upperBounds), reinterpret_cast<Float3 *>(&lowerBounds),
            &reinterpret_cast<Enemy *>(*reinterpret_cast<EnemyManagerUpdateEnemy **>(raw + 0x9DCDA0))->playerShotHitAccumulator,
            &bombHit);
    }

    reinterpret_cast<EnemyManager *>(this)->FUN_0042c3b0();

    reinterpret_cast<EnemyManagerUpdateEnemy **>(raw + 0x9DCEDC)[3] = 0;
    reinterpret_cast<EnemyManagerUpdateEnemy **>(raw + 0x9DCEDC)[2] = 0;
    reinterpret_cast<EnemyManagerUpdateEnemy **>(raw + 0x9DCEDC)[1] = 0;
    reinterpret_cast<EnemyManagerUpdateEnemy **>(raw + 0x9DCEDC)[0] = 0;

    for (enemyIndex = 0; enemyIndex < g_EclManager.GetTimelineCount(); ++enemyIndex)
    {
        if (reinterpret_cast<EnemyManagerUpdateTimelineLane *>(raw + 0x9DCDD0)[enemyIndex].instruction == 0)
        {
            reinterpret_cast<EnemyManagerUpdateTimelineLane *>(raw + 0x9DCDD0)[enemyIndex].instruction =
                reinterpret_cast<void *>(g_EclManager.GetTimeline(enemyIndex));
        }
        reinterpret_cast<EclTimeline *>(raw + 0x9DCDD0)[enemyIndex].Run();
    }

    enemy = reinterpret_cast<EnemyManagerUpdateEnemy *>(raw + 0x53D0);
    *reinterpret_cast<i32 *>(raw + 0x9DCDC4) = 0;
    for (enemyIndex = 0; enemyIndex < 480; ++enemyIndex, ++enemy)
    {
        if ((*reinterpret_cast<u32 *>(enemy->raw + 0x3324) & EMUF1_ACTIVE) == 0)
        {
            if (g_Player.optionHomingTarget == reinterpret_cast<Enemy *>(enemy))
                g_Player.optionHomingTarget = 0;
            continue;
        }

        damageOccurred = 0;

        if (reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->suppressDeathEffects)
        {
            *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88) = *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D34) + *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D40);
            reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88)->z = 0.0f;
            goto process_enemy_death;
        }

        ++*reinterpret_cast<i32 *>(raw + 0x9DCDC4);

        if ((reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->pauseTimer &&
             (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_Player) + 0xFDC) || reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->playerType)) ||
            reinterpret_cast<EnemyManagerUpdateFlag2Bits *>(enemy->raw + 0x3328)->forcePause)
        {
            reinterpret_cast<Enemy *>(enemy)->bossTimer--;
            goto update_damage_flash;
        }

    run_enemy_ecl:
        if (reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->preEclUpdate)
            reinterpret_cast<Enemy *>(enemy)->FUN_0042c420();

    run_enemy_ecl_after_pause:
        if (g_EclManager.RunEcl(reinterpret_cast<Enemy *>(enemy)) == -1)
        {
            *reinterpret_cast<u32 *>(enemy->raw + 0x3324) &= ~1U;
            reinterpret_cast<Enemy *>(enemy)->FUN_0042bcf0();
            continue;
        }

        if (!reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->skipMovement)
        {
            reinterpret_cast<Enemy *>(enemy)->ClampPosition();
            reinterpret_cast<Enemy *>(enemy)->FUN_0042deb0();
            reinterpret_cast<Enemy *>(enemy)->ClampPosition();

            if (*reinterpret_cast<void **>(enemy->raw + 0x2DA4) != 0 &&
                reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(
                    enemy->raw + 0x3324)->copyFollowVelocity)
            {
                *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D40) =
                    *reinterpret_cast<D3DXVECTOR3 *>(
                        (*reinterpret_cast<EnemyManagerUpdateEnemy **>(
                            enemy->raw + 0x2DA4))->raw + 0x2D34);
            }

            *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88) =
                *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D34) +
                *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D40);
            reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88)->z = 0.0f;
        }
        else
        {
            *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88) =
                *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D34) +
                *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D40);
            reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88)->z = 0.0f;
        }

        if (*reinterpret_cast<EnemyManagerUpdateAnmVm **>(enemy->raw + 0x53C8) != 0)
        {
            *reinterpret_cast<D3DXVECTOR3 *>(
                reinterpret_cast<u8 *>(*reinterpret_cast<EnemyManagerUpdateAnmVm **>(
                    enemy->raw + 0x53C8)) + 0x2A4) =
                *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88);
        }

        if (*reinterpret_cast<u8 *>(enemy->raw + 0x534C))
        {
            for (trailIndex = *reinterpret_cast<i16 *>(enemy->raw + 0x534E) - 1;
                 trailIndex > 0; --trailIndex)
            {
                *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x3394 + 0x1C * trailIndex) =
                    *reinterpret_cast<D3DXVECTOR3 *>(
                        enemy->raw + 0x3394 + 0x1C * (trailIndex - 1));
                *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x33A0 + 0x1C * trailIndex) =
                    *reinterpret_cast<D3DXVECTOR3 *>(
                        enemy->raw + 0x33A0 + 0x1C * (trailIndex - 1));
                *reinterpret_cast<u32 *>(enemy->raw + 0x33AC + 0x1C * trailIndex) =
                    *reinterpret_cast<u32 *>(
                        enemy->raw + 0x33AC + 0x1C * (trailIndex - 1));
            }

            *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x3394) =
                *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88);
            *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x33A0) =
                *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D4C);
            *reinterpret_cast<u32 *>(enemy->raw + 0x33AC) =
                *reinterpret_cast<u32 *>(enemy->raw + 0x2D94);
        }

        if (reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite == 0)
            *reinterpret_cast<u32 *>(enemy->raw + 0x3324) |= EMUF1_NO_SPRITE;

        if (!reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->noSprite &&
            !reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->hasBeenInBounds &&
            g_GameManager.IsWithinPlayfield(
                (*reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88))[0],
                (*reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88))[1],
                reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->width,
                reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->height))
        {
            *reinterpret_cast<u32 *>(enemy->raw + 0x3324) |= EMUF1_HAS_BEEN_IN_BOUNDS;
        }

        else if (reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->hasBeenInBounds == 1 &&
            !reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->allowOffscreen)
        {
            if ((!*reinterpret_cast<u8 *>(enemy->raw + 0x534C) &&
                 !g_GameManager.IsWithinPlayfield(
                     reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88)->x,
                     reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88)->y,
                     reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->width,
                     reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->height)) ||
                (*reinterpret_cast<u8 *>(enemy->raw + 0x534C) &&
                 !g_GameManager.IsWithinPlayfield(
                     reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88)->x,
                     reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88)->y,
                     reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->width,
                     reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->height) &&
                 !g_GameManager.IsWithinPlayfield(
                     reinterpret_cast<D3DXVECTOR3 *>(
                         enemy->raw + 0x3394 + 0x1C *
                         (*reinterpret_cast<i16 *>(enemy->raw + 0x534E) - 1))->x,
                     reinterpret_cast<D3DXVECTOR3 *>(
                         enemy->raw + 0x3394 + 0x1C *
                         (*reinterpret_cast<i16 *>(enemy->raw + 0x534E) - 1))->y,
                     reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->width,
                     reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->sprite->height)))
            {
                *reinterpret_cast<u32 *>(enemy->raw + 0x3324) &= ~1U;
                reinterpret_cast<Enemy *>(enemy)->FUN_0042bcf0();
                continue;
            }
        }

        if (reinterpret_cast<Enemy *>(enemy)->FUN_0042b490())
            goto run_enemy_ecl_after_pause;
        if (*reinterpret_cast<i32 *>(enemy->raw + 0x3378) >= 0 && reinterpret_cast<Enemy *>(enemy)->FUN_0042b930())
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
        if (!reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->noSprite &&
            !reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->skipCombatA &&
            !reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->skipCombatB &&
            (!reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->noDamageDuringStop || !*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_Player) + 0xFDC)))
        {
            if (reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->collision)
            {
                reinterpret_cast<Enemy *>(enemy)->FUN_0042c290(reinterpret_cast<Float3 *>(enemy->raw + 0x2D88), reinterpret_cast<Float3 *>(enemy->raw + 0x2D70));
                if (*reinterpret_cast<u8 *>(enemy->raw + 0x534C))
                {
                    secondaryHitbox = *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D70);
                    for (trailIndex = 1; trailIndex < *reinterpret_cast<i16 *>(enemy->raw + 0x5350); trailIndex += 6)
                    {
                        if (*reinterpret_cast<u8 *>(enemy->raw + 0x534C) & 2)
                        {
                            secondaryHitbox = *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D70) -
                                (*reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D70) * (f32)trailIndex / (f32)*reinterpret_cast<i16 *>(enemy->raw + 0x5350));
                        }
                        reinterpret_cast<Enemy *>(enemy)->FUN_0042c290(
                            reinterpret_cast<Float3 *>(
                                enemy->raw + 0x3394 + 0x1C * trailIndex),
                            reinterpret_cast<Float3 *>(&secondaryHitbox));
                    }
                }
            }

            *reinterpret_cast<i32 *>(enemy->raw + 0x3354) = 0;
            if (reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->acceptsDamage)
            {
                if (!g_Spellcard.IsActive() || !reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(enemy)->HasAttachedEnemy() ||
                    !*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_Player) + 0xFDC))
                {
                    damage = g_Player.FUN_00451670(
                        reinterpret_cast<Float3 *>(enemy->raw + 0x2D88), reinterpret_cast<Float3 *>(enemy->raw + 0x2D70), &reinterpret_cast<Enemy *>(enemy)->playerShotHitAccumulator, &bombHit);
                }
                else
                {
                    damage = 0;
                }

                if (reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D7C)->x > 0.0f)
                {
                    extraDamage = g_Player.FUN_00451670(
                        reinterpret_cast<Float3 *>(enemy->raw + 0x2D88), reinterpret_cast<Float3 *>(enemy->raw + 0x2D7C), &reinterpret_cast<Enemy *>(enemy)->playerShotHitAccumulator, &bombHit);
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
                    if ((reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->boss || !reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->mode) &&
                        !*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_Player) + 0xFDC))
                    {
                        if (reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->boss && !reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->mode)
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

                    if (reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->damageable)
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

                        if (*reinterpret_cast<ZunTimer *>(enemy->raw + 0x5354) > 0)
                        {
                            if (reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->boss)
                                damage /= 9;
                            else
                                damage = 0;
                        }

                        reinterpret_cast<Enemy *>(enemy)->life -= damage;
                        *reinterpret_cast<i32 *>(enemy->raw + 0x3354) = damage;
                        reinterpret_cast<Enemy *>(enemy)->FUN_0042b370(damage);
                    }
                    damageOccurred = 1;
                }

                if (reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->boss)
                {
                    previousTargetDelta =
                        *reinterpret_cast<D3DXVECTOR3 *>(&g_Player.tailPosition0) -
                        (*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(&g_Player) + 0x2B4));
                    currentTargetDelta = *reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88) - (*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(&g_Player) + 0x2B4));
                    if (!g_Player.enemyTrackedPositionValid ||
                        fabsf(previousTargetDelta.x) >
                            fabsf(currentTargetDelta.x))
                    {
                        g_Player.tailPosition0 = *reinterpret_cast<Float3 *>(enemy->raw + 0x2D88);
                    }
                    g_Player.enemyTrackedPositionValid = 1;
                }

                if (!g_Player.enemyTrackedPositionValid &&
                    g_Player.tailPosition0[1] <
                        (*reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88))[1])
                {
                    g_Player.tailPosition0 = *reinterpret_cast<Float3 *>(enemy->raw + 0x2D88);
                }

                if (fabsf(
                        reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88)->x -
                        (*reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(&g_Player) + 0x2B4)).x) < 64.0f &&
                    !reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(enemy)->HasAttachedEnemy() &&
                    (g_Player.optionHomingTarget == 0 ||
                     reinterpret_cast<D3DXVECTOR3 *>(
                         reinterpret_cast<EnemyManagerUpdateEnemy *>(g_Player.optionHomingTarget)->raw + 0x2D34)->y >
                         reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88)->y))
                {
                    g_Player.optionHomingTarget = reinterpret_cast<Enemy *>(enemy);
                }
            }
        }

        if (reinterpret_cast<EnemyManagerUpdateFlag2Bits *>(enemy->raw + 0x3328)->deathLatch &&
            reinterpret_cast<Enemy *>(enemy)->life > 0)
            *reinterpret_cast<u32 *>(enemy->raw + 0x3328) &= ~EMUF2_DEATH_LATCH;

        if (reinterpret_cast<Enemy *>(enemy)->life <= 0 &&
            !reinterpret_cast<EnemyManagerUpdateFlag2Bits *>(enemy->raw + 0x3328)->deathLatch &&
            !reinterpret_cast<EnemyManagerUpdateFlag2Bits *>(enemy->raw + 0x3328)->noDeath)
        {
        process_enemy_death:
            ;
            // VC7 hashes nested locals by identifier.  These target-pinned
            // spellings carry, respectively, the bonus value, the otherwise
            // unused death-position vector, and the VM loop/gauge value.
            i32 deathVmIndex;

            *reinterpret_cast<u32 *>(enemy->raw + 0x3328) |= EMUF2_DEATH_LATCH;
            D3DXVECTOR3 bonus;
            i32 deathPosition;
            *reinterpret_cast<i32 *>(enemy->raw + 0x53CC) =
                (*reinterpret_cast<i32 *>(enemy->raw + 0x3378) - (i32)reinterpret_cast<Enemy *>(enemy)->bossTimer) / 60;
            *reinterpret_cast<i32 *>(enemy->raw + 0x3378) = -1;
            for (deathPosition = 0; deathPosition < 4; ++deathPosition)
                reinterpret_cast<i32 *>(enemy->raw + 0x3358)[deathPosition] = -1;
            for (deathPosition = 0; deathPosition < 4; ++deathPosition)
            {
                if (reinterpret_cast<void **>(enemy->raw + 0x3384)[deathPosition] != 0)
                {
                    g_ZunMemory.Free(reinterpret_cast<void **>(enemy->raw + 0x3384)[deathPosition]);
                    reinterpret_cast<void **>(enemy->raw + 0x3384)[deathPosition] = 0;
                }
            }

            if (reinterpret_cast<EclOperands::TargetEnemyHelpersOverlay *>(enemy)->HasAttachedEnemy())
            {
                --*reinterpret_cast<i32 *>(
                    (*reinterpret_cast<EnemyManagerUpdateEnemy **>(
                        enemy->raw + 0x2DA4))->raw + 0x3380);
            }

            reinterpret_cast<EclOperands::EnemyOverlay *>(enemy)->FUN_0042adb0(1);
            if (!reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->mode)
                deathPosition = -200;
            else
                deathPosition = 200;
            g_GameManager.AddToYoukaiGauge(deathPosition, 0);

            switch ((*reinterpret_cast<u32 *>(enemy->raw + 0x3324) >> 20) & 7)
            {
            case 3:
                reinterpret_cast<Enemy *>(enemy)->life = 1;
                *reinterpret_cast<u32 *>(enemy->raw + 0x3324) &= ~EMUF1_DAMAGEABLE;
                *reinterpret_cast<u32 *>(enemy->raw + 0x3324) &= ~EMUF1_DEATH_MODE_MASK;
                g_Gui.SetBossPresent(false);
                reinterpret_cast<EnemyManagerUpdateReplayManager *>(g_ReplayManager)->flags |= 0x20;
                if (*reinterpret_cast<i8 *>(enemy->raw + 0x3310) >= 0)
                {
                    g_EffectManager.SpawnEffect(*reinterpret_cast<i8 *>(enemy->raw + 0x3310), reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88), 1, -1);
                    g_EffectManager.SpawnEffect(*reinterpret_cast<i8 *>(enemy->raw + 0x3310), reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88), 1, -1);
                    g_EffectManager.SpawnEffect(*reinterpret_cast<i8 *>(enemy->raw + 0x3310), reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88), 1, -1);
                }
                if (*reinterpret_cast<EnemyManagerUpdateAnmVm **>(enemy->raw + 0x53C8) != 0)
                {
                    reinterpret_cast<AnmVmBase *>(*reinterpret_cast<EnemyManagerUpdateAnmVm **>(
                        enemy->raw + 0x53C8))->SetInterrupt(3);
                    *reinterpret_cast<EnemyManagerUpdateAnmVm **>(enemy->raw + 0x53C8) = 0;
                }
                if (!reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->playerType)
                {
                    g_Player.timer = 90;
                    reinterpret_cast<EnemyManagerUpdatePlayer *>(&g_Player)->playerType = 3;
                }
                *reinterpret_cast<u32 *>(enemy->raw + 0x3324) &= ~EMUF1_PAUSE_TIMER;
                *reinterpret_cast<u32 *>(enemy->raw + 0x3324) &= ~EMUF1_NO_DAMAGE_DURING_STOP;
                goto death_audio_and_callback;

            case 1:
                g_GameManager.AddScore(reinterpret_cast<Enemy *>(enemy)->score);
                *reinterpret_cast<u32 *>(enemy->raw + 0x3324) |= EMUF1_PERSIST_AFTER_DEATH;
                *reinterpret_cast<u32 *>(enemy->raw + 0x3324) &= ~EMUF1_COLLISION;
                *reinterpret_cast<u32 *>(enemy->raw + 0x3324) &= ~EMUF1_DAMAGEABLE;
                *reinterpret_cast<u32 *>(enemy->raw + 0x3324) &= ~EMUF1_ACCEPTS_DAMAGE;
                goto common_death_mode;

            case 0:
                g_GameManager.AddScore(reinterpret_cast<Enemy *>(enemy)->score);
                *reinterpret_cast<u32 *>(enemy->raw + 0x3324) &= ~1U;
                if (*reinterpret_cast<EnemyManagerUpdateAnmVm **>(enemy->raw + 0x53C8) != 0)
                {
                    reinterpret_cast<AnmVmBase *>(*reinterpret_cast<EnemyManagerUpdateAnmVm **>(
                        enemy->raw + 0x53C8))->SetInterrupt(3);
                    *reinterpret_cast<EnemyManagerUpdateAnmVm **>(enemy->raw + 0x53C8) = 0;
                }
                goto common_death_mode;

            case 2:
            common_death_mode:
                if (reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->boss)
                {
                    g_Gui.SetBossPresent(false);
                    reinterpret_cast<Enemy *>(enemy)->FUN_0042a820();
                }
                reinterpret_cast<Enemy *>(enemy)->FUN_0042bea0(bombHit);
                if (reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->boss &&
                    !g_Spellcard.IsActive())
                {
                    deathVmIndex = g_BulletManager.DespawnBullets(8000, 1);
                    deathVmIndex = g_EnemyManager.FUN_0042efb0(8000, deathVmIndex);
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
            if (!reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->suppressDeathEffects)
            {
                g_SoundPlayer.PlaySoundPositionedByIdx(
                    static_cast<SoundIdx>(enemyIndex % 2 + 2), reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88)->x);
                if (*reinterpret_cast<i8 *>(enemy->raw + 0x3310) >= 0)
                {
                    g_EffectManager.SpawnEffect(*reinterpret_cast<i8 *>(enemy->raw + 0x3310), reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88), 1, -1);
                    g_EffectManager.SpawnEffect(*reinterpret_cast<u8 *>(enemy->raw + 0x3311) + 4, reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88), 4, -1);
                }
                if (g_GameManager.GaugeIsExtremelyHuman() ||
                    g_GameManager.GaugeIsExtremelyYoukai())
                {
                    g_ItemManager.SpawnItem(reinterpret_cast<Float3 *>(enemy->raw + 0x2D88), static_cast<ItemType>(7), 1);
                }
            }

            if (*reinterpret_cast<i16 *>(enemy->raw + 0x2CEE) >= 0)
            {
                i32 callbackVmIndex;

                reinterpret_cast<Enemy *>(enemy)->enemy_fun_00415c80();
                *reinterpret_cast<i16 *>(enemy->raw + 0x2CEA) = 0;
                for (callbackVmIndex = 0; callbackVmIndex < 4; ++callbackVmIndex)
                    reinterpret_cast<i32 *>(enemy->raw + 0x3358)[callbackVmIndex] = -1;
                *reinterpret_cast<i32 *>(enemy->raw + 0x3378) = -1;
                reinterpret_cast<Enemy *>(enemy)->FUN_0042bc90();
                memcpy(&reinterpret_cast<Enemy *>(enemy)->bulletSpawnDescriptor,
                       &g_EnemyManager.firstEnemy.bulletSpawnDescriptor,
                       sizeof(BulletSpawnDescriptor));
                reinterpret_cast<Enemy *>(enemy)->shootIntervalFrames = 0;
                g_EclManager.CallEclSub(reinterpret_cast<EnemyEclContext *>(enemy->raw + 0x7F8), *reinterpret_cast<i16 *>(enemy->raw + 0x2CEE));
                *reinterpret_cast<i16 *>(enemy->raw + 0x2CEE) = -1;
            }
        }

    update_damage_flash:
        if (!reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->skipCombatB)
        {
            if (*reinterpret_cast<u8 *>(enemy->raw + 0x3314))
            {
                --*reinterpret_cast<u8 *>(enemy->raw + 0x3314);
                reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->flags &= ~0x20000U;
            }
            else if (damageOccurred)
            {
                if (reinterpret_cast<EnemyManagerUpdateFlag2Bits *>(enemy->raw + 0x3328)->bossMarker < 2)
                    g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(20), reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88)->x);
                else
                    g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(37), reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88)->x);

                reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->unknown1F4)[2] = 0xFF;
                reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->unknown1F4)[1] = 0x60;
                reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->unknown1F4)[0] = 0x80;
                reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->unknown1F4)[3] =
                    reinterpret_cast<u8 *>(&reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->color)[3];
                reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->flags |= 0x20000U;
                *reinterpret_cast<u8 *>(enemy->raw + 0x3314) = 1;
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

        if (reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->boss)
        {
            D3DXVECTOR3 markerPosition;

            if (!g_Gui.IsDialogPresent() && !*reinterpret_cast<u8 *>(enemy->raw + 0x3313))
                g_Gui.FUN_004230c0((f32)reinterpret_cast<Enemy *>(enemy)->life / (f32)reinterpret_cast<Enemy *>(enemy)->maxLife);

            if (reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->boss < 4)
            {
                if (!reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->noSprite)
                    markerPosition.x = reinterpret_cast<D3DXVECTOR3 *>(enemy->raw + 0x2D88)->x + 32.0f;
                else
                    markerPosition.x = -999.0f;
                markerPosition.y = 472.0f;
                markerPosition.z = 0.0f;
                g_AsciiManager.SetBossMarkerPosition(*reinterpret_cast<u8 *>(enemy->raw + 0x3313), &markerPosition);

                if (reinterpret_cast<EnemyManagerUpdateFlag2Bits *>(enemy->raw + 0x3328)->bossMarker == 0)
                    g_AsciiManager.FUN_0042f2d0(
                        *reinterpret_cast<u8 *>(enemy->raw + 0x3313),
                        ((reinterpret_cast<EnemyManagerUpdateAnmVm *>(enemy->raw + 0xC)->flags >> 17) & 1) != 0);
                else
                    g_AsciiManager.FUN_0042f2d0(
                        *reinterpret_cast<u8 *>(enemy->raw + 0x3313),
                        reinterpret_cast<EnemyManagerUpdateFlag2Bits *>(enemy->raw + 0x3328)->bossMarker + 1);
            }
        }

        reinterpret_cast<Enemy *>(enemy)->FUN_0042e010();
        if (!g_EclScriptedGlobalUpdateFreeze)
            reinterpret_cast<Enemy *>(enemy)->bossTimer++;
        if (*reinterpret_cast<ZunTimer *>(enemy->raw + 0x5354) > 0)
            (*reinterpret_cast<ZunTimer *>(enemy->raw + 0x5354))--;

        if (!reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->noSprite &&
            reinterpret_cast<EnemyManagerUpdateFlag1Bits *>(enemy->raw + 0x3324)->active)
        {
            *reinterpret_cast<EnemyManagerUpdateEnemy **>(enemy->raw) =
                reinterpret_cast<EnemyManagerUpdateEnemy **>(
                    raw + 0x9DCEDC)[*reinterpret_cast<u8 *>(enemy->raw + 0x332F)];
            reinterpret_cast<EnemyManagerUpdateEnemy **>(
                raw + 0x9DCEDC)[*reinterpret_cast<u8 *>(enemy->raw + 0x332F)] = enemy;
        }
    }

    if ((*reinterpret_cast<ZunTimer *>(raw + 0x9DCED0) % 200) == 0 &&
        g_GameManager.IsTampered())
        return 4;

    (*reinterpret_cast<ZunTimer *>(raw + 0x9DCED0))++;
    return 1;
}

// FUNCTION: th08 0x42c660
ChainCallbackResult EnemyManager::OnUpdate(EnemyManager *enemyManager)
{
    return static_cast<ChainCallbackResult>(
        reinterpret_cast<EnemyManagerUpdateOverlay *>(enemyManager)->OnUpdate());
}

} // namespace th08
