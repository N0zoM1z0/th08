#pragma once
#include "Global.hpp"
#include "AnmManager.hpp"
#include "BulletManager.hpp"
#include "EclManager.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"

namespace th08
{

struct EnemyEclContext;
struct Spellcard;

struct EnemyTrailSample
{
    Float3 position;
    Float3 velocity;
    f32 angle;

    EnemyTrailSample();
};
C_ASSERT(sizeof(EnemyTrailSample) == 0x1c);
C_ASSERT(offsetof(EnemyTrailSample, position) == 0x0);
C_ASSERT(offsetof(EnemyTrailSample, velocity) == 0xc);
C_ASSERT(offsetof(EnemyTrailSample, angle) == 0x18);

enum EnemyTrailFlagMask
{
    ENEMY_TRAIL_ENABLED = 1U << 0,
    ENEMY_TRAIL_TAPER = 1U << 1,
    ENEMY_TRAIL_FADE = 1U << 2,
    ENEMY_TRAIL_RENDER_AS_STRIP = 1U << 3,
    ENEMY_TRAIL_HIDE_HEAD_ANM = 1U << 4,
};

struct EnemyBulletRankInfluence
{
    f32 speedLow;
    f32 speedHigh;
    i16 count1Low;
    i16 count1High;
    i16 count2Low;
    i16 count2High;
};
C_ASSERT(sizeof(EnemyBulletRankInfluence) == 0x10);

enum EnemyMovementMode
{
    ENEMY_MOVEMENT_MODE_NONE = 0,
    ENEMY_MOVEMENT_MODE_POLAR = 1,
    ENEMY_MOVEMENT_MODE_INTERPOLATED = 2,
    ENEMY_MOVEMENT_MODE_ORBIT = 3,
};

enum EnemyFlag1Mask
{
    ENEMY_FLAG_ACTIVE = 1U << 0,
    ENEMY_FLAG_BOSS = 1U << 1,
    ENEMY_FLAG_COLLISION = 1U << 2,
    ENEMY_FLAG_DAMAGEABLE = 1U << 3,
    ENEMY_FLAG_NO_SPRITE = 1U << 4,
    ENEMY_FLAG_HIDE_PRIMARY_ANM = 1U << 5,
    ENEMY_FLAG_ACCEPTS_DAMAGE = 1U << 6,
    ENEMY_FLAG_SPECIAL_INTERACTION = 1U << 7,
    ENEMY_FLAG_LINKED_CHILD = 1U << 8,
    ENEMY_FLAG_INHERIT_PARENT_POSITION = 1U << 9,
    ENEMY_FLAG_SUPPRESS_DEATH_EFFECTS = 1U << 10,
    ENEMY_FLAG_YOUKAI_ALIGNED = 1U << 11,
    ENEMY_FLAG_MOVEMENT_MODE_MASK = 3U << 12,
    ENEMY_FLAG_MOVEMENT_MODE_POLAR = ENEMY_MOVEMENT_MODE_POLAR << 12,
    ENEMY_FLAG_MOVEMENT_MODE_INTERPOLATED =
        ENEMY_MOVEMENT_MODE_INTERPOLATED << 12,
    ENEMY_FLAG_MOVEMENT_MODE_ORBIT = ENEMY_MOVEMENT_MODE_ORBIT << 12,
    ENEMY_FLAG_MOVEMENT_EASING_MASK = 7U << 14,
    ENEMY_FLAG_DEFER_BULLET_PATTERN = 1U << 17,
    ENEMY_FLAG_MIRROR_MOVEMENT_X = 1U << 18,
    ENEMY_FLAG_CLAMP_POSITION = 1U << 19,
    ENEMY_FLAG_DEATH_MODE_MASK = 7U << 20,
    ENEMY_FLAG_PERSIST_AFTER_DEATH = 1U << 23,
    ENEMY_FLAG_HAS_BEEN_IN_BOUNDS = 1U << 24,
    ENEMY_FLAG_ROTATE_ANM_WITH_MOVEMENT = 1U << 25,
    ENEMY_FLAG_DISABLE_ECL_CALL_STACK = 1U << 26,
    ENEMY_FLAG_TIMEOUT_SPELL = 1U << 27,
    ENEMY_FLAG_ALLOW_OFFSCREEN = 1U << 28,
    ENEMY_FLAG_SKIP_MOVEMENT = 1U << 29,
    ENEMY_FLAG_PAUSE_TIMER = 1U << 30,
    ENEMY_FLAG_NO_DAMAGE_DURING_STOP = 1U << 31
};

enum EnemyFlag1Shift
{
    ENEMY_FLAG_ACTIVE_SHIFT = 0,
    ENEMY_FLAG_BOSS_SHIFT = 1,
    ENEMY_FLAG_COLLISION_SHIFT = 2,
    ENEMY_FLAG_DAMAGEABLE_SHIFT = 3,
    ENEMY_FLAG_NO_SPRITE_SHIFT = 4,
    ENEMY_FLAG_HIDE_PRIMARY_ANM_SHIFT = 5,
    ENEMY_FLAG_ACCEPTS_DAMAGE_SHIFT = 6,
    ENEMY_FLAG_SPECIAL_INTERACTION_SHIFT = 7,
    ENEMY_FLAG_LINKED_CHILD_SHIFT = 8,
    ENEMY_FLAG_INHERIT_PARENT_POSITION_SHIFT = 9,
    ENEMY_FLAG_SUPPRESS_DEATH_EFFECTS_SHIFT = 10,
    ENEMY_FLAG_YOUKAI_ALIGNED_SHIFT = 11,
    ENEMY_FLAG_MOVEMENT_MODE_SHIFT = 12,
    ENEMY_FLAG_MOVEMENT_EASING_SHIFT = 14,
    ENEMY_FLAG_DEFER_BULLET_PATTERN_SHIFT = 17,
    ENEMY_FLAG_MIRROR_MOVEMENT_X_SHIFT = 18,
    ENEMY_FLAG_CLAMP_POSITION_SHIFT = 19,
    ENEMY_FLAG_DEATH_MODE_SHIFT = 20,
    ENEMY_FLAG_HAS_BEEN_IN_BOUNDS_SHIFT = 24,
    ENEMY_FLAG_ROTATE_ANM_WITH_MOVEMENT_SHIFT = 25,
    ENEMY_FLAG_DISABLE_ECL_CALL_STACK_SHIFT = 26,
    ENEMY_FLAG_TIMEOUT_SPELL_SHIFT = 27,
    ENEMY_FLAG_ALLOW_OFFSCREEN_SHIFT = 28,
    ENEMY_FLAG_SKIP_MOVEMENT_SHIFT = 29,
    ENEMY_FLAG_PAUSE_TIMER_SHIFT = 30,
    ENEMY_FLAG_NO_DAMAGE_DURING_STOP_SHIFT = 31
};

struct EnemyFlag1Bits
{
    u32 active : 1;
    u32 boss : 1;
    u32 collision : 1;
    u32 damageable : 1;
    u32 noSprite : 1;
    u32 hidePrimaryAnm : 1;
    u32 acceptsDamage : 1;
    u32 specialInteraction : 1;
    u32 linkedChild : 1;
    u32 inheritParentPosition : 1;
    u32 suppressDeathEffects : 1;
    u32 youkaiAligned : 1;
    u32 movementMode : 2;
    u32 movementEasing : 3;
    u32 deferBulletPattern : 1;
    u32 mirrorMovementX : 1;
    u32 clampPosition : 1;
    u32 deathMode : 3;
    u32 persistAfterDeath : 1;
    u32 hasBeenInBounds : 1;
    u32 rotateAnmWithMovement : 1;
    u32 disableEclCallStack : 1;
    u32 timeoutSpell : 1;
    u32 allowOffscreen : 1;
    u32 skipMovement : 1;
    u32 pauseTimer : 1;
    u32 noDamageDuringStop : 1;
};
C_ASSERT(sizeof(EnemyFlag1Bits) == 4);

enum EnemyFlag2Mask
{
    ENEMY_FLAG2_FORM_EFFECT = 1U << 1,
    ENEMY_FLAG2_ALTERNATE_ANM_BANK = 1U << 2,
    ENEMY_FLAG2_DEATH_LATCH = 1U << 3,
    ENEMY_FLAG2_DAMAGE_FEEDBACK_MASK = 3U << 4,
    ENEMY_FLAG2_NO_DEATH = 1U << 6,
    ENEMY_FLAG2_FORCE_PAUSE = 1U << 7,
    ENEMY_FLAG2_EXTRA_VM_FIXED_OFFSET = 1U << 8
};

enum EnemyFlag2Shift
{
    ENEMY_FLAG2_FORM_EFFECT_SHIFT = 1,
    ENEMY_FLAG2_ALTERNATE_ANM_BANK_SHIFT = 2,
    ENEMY_FLAG2_DEATH_LATCH_SHIFT = 3,
    ENEMY_FLAG2_DAMAGE_FEEDBACK_SHIFT = 4,
    ENEMY_FLAG2_NO_DEATH_SHIFT = 6,
    ENEMY_FLAG2_FORCE_PAUSE_SHIFT = 7,
    ENEMY_FLAG2_EXTRA_VM_FIXED_OFFSET_SHIFT = 8
};

struct EnemyFlag2Bits
{
    u32 reserved00 : 1;
    u32 formEffect : 1;
    u32 alternateAnmBank : 1;
    u32 deathLatch : 1;
    u32 damageFeedbackLevel : 2;
    u32 noDeath : 1;
    u32 forcePause : 1;
    u32 extraVmFixedOffset : 1;
    u32 reserved09_31 : 23;
};
C_ASSERT(sizeof(EnemyFlag2Bits) == 4);

struct EnemyAnmScripts
{
    i16 idleInitial;
    i16 idleFromLeft;
    i16 idleFromRight;
    i16 moveLeft;
    i16 moveRight;
    i16 special;
};
C_ASSERT(sizeof(EnemyAnmScripts) == 0xc);

struct EnemyMovementBounds
{
    Float2 lower;
    Float2 upper;
};
C_ASSERT(sizeof(EnemyMovementBounds) == 0x10);

void __fastcall PrepareSpellcardForTimerCallback(Spellcard *spellcard);

struct Enemy
{
    Enemy();
    f32 ResolveFloat(f32 operand);
    void DetachEnemyChain(i32 awardRewards);
    void ApplyDamageToParent(i32 amount);
    void Despawn();
    void UpdateEffects();

    Enemy *nextInDrawGroup;
    Enemy *previousInAttachmentChain;
    Enemy *nextInAttachmentChain;
    AnmVm vm;
    AnmVm secondaryVms[2];
    EnemyEclContext mainEclContextStorage;
    EnemyEclContext mainEclCallStackStorage[16];
    EnemyEclContext *activeEclContext;
    EnemyEclContext *activeEclCallStack;
    i32 eclIntVariables[8];
    f32 eclFloatVariables[8];
    i16 mainEclCallStackDepth;
    i16 activeEclCallStackDepth;
    u16 unconsumedWord2CEC;
    i16 deathCallbackSubId;
    i16 eclSubroutineIds[32];
    i16 pendingEclSubroutineIndex;
    u8 positionAlignment2D32[2];
    Float3 position;
    Float3 positionOffset;
    Float3 velocity;
    Float3 previousPosition;
    Float3 lastFrameDisplacement;
    Float3 hitboxDimensions;
    Float3 secondaryHitboxDimensions;
    Float3 worldPosition;
    f32 movementAngle;
    f32 angularVelocity;
    f32 orbitAngle;
    f32 orbitAngularVelocity;
    Enemy *parentEnemy;
    f32 speed;
    f32 acceleration;
    f32 orbitRadius;
    f32 radialVelocity;
    Float3 shootOffset;
    Float3 movementInterpolationDelta;
    Float3 movementInterpolationOrigin;
    ZunTimer movementTimer;
    i32 movementDuration;
    EnemyBulletRankInfluence bulletRankInfluence;
    i32 life;
    i32 maxLife;
    i32 phaseStartingLife;
    i32 score;
    i32 enemyIndex;
    i32 playerShotHitAccumulator;
    ZunTimer bossTimer;
    D3DCOLOR displayColor;
    BulletSpawnDescriptor bulletSpawnDescriptor;
    u8 pendingShotInstruction[0x2c];
    i32 shootIntervalFrames;
    ZunTimer shootIntervalTimer;
    BulletSpawnDescriptor laserSpawnDescriptor;
    Laser *laserSlots[32];
    i32 selectedLaserSlot;
    i32 itemDropType;
    i32 pointItemDropCount;
    i32 powerOrPointItemDropCount;
    i8 deathAnm1;
    u8 deathAnm2;
    u8 deathAnm3;
    u8 bossSlot;
    u8 damageFlashTimer;
    u8 timerAlignment3315[3];
    ZunTimer timer3318;
    u32 flags1;
    u32 flags2;
    u16 unconsumedWord332C;
    u8 anmDirection;
    u8 drawGroup;
    u8 eclDifficultyMaskOverride;
    u8 anmScriptsAlignment3331;
    EnemyAnmScripts anmScripts;
    u8 movementBoundsAlignment333E[2];
    EnemyMovementBounds movementBounds;
    f32 minimumPlayerDistanceSquared;
    i32 lastDamage;
    i32 lifeCallbackThresholds[4];
    i32 lifeCallbackSubIds[4];
    i32 timerCallbackThresholdFrames;
    i32 timerCallbackSubId;
    i32 linkedChildCount;
    EnemyChildEclBlock *childEclBlocks[4];
    EnemyTrailSample trailSamples[96];
    VertexTex1DiffuseXyzrhw trailVertices[194];
    u8 trailFlags;
    u8 trailLengthAlignment534D;
    i16 trailHistoryLength;
    i16 trailCollisionLength;
    i16 trailSampleStride;
    ZunTimer damageReductionTimer;
    Effect *attachedEffects[24];
    i32 attachedEffectCount;
    f32 attachedEffectDistance;
    Effect *alignmentEffect;
    i32 phaseEndTimeRemainingSeconds;

    // Target-observed RunEcl post-dispatch calls.  Both receive the current
    // enemy in ECX and take no explicit arguments.
    void ReleaseAttachedEffects();
    void UpdateMovement();
    void UpdateShotAndAnm();
    void ReleaseChildEclBlocks();
    void ResetBulletRankInfluence();
    i32 HasParentChain();
    i32 HasAttachedEnemy();
    i32 CountParentChain();
    void DetachFromParentChain();
    void DropItems(i32 mode);
    i32 HandleLifeCallback();
    i32 HandleTimerCallback();
    void ClampPosition();
    void CheckPlayerCollision(Float3 *position, Float3 *size);
    void UpdateYoukaiAlignment();
    void IntegrateVelocity();
};
C_ASSERT(sizeof(Enemy) == 0x53d0);
C_ASSERT(offsetof(Enemy, nextInDrawGroup) == 0x0);
C_ASSERT(offsetof(Enemy, previousInAttachmentChain) == 0x4);
C_ASSERT(offsetof(Enemy, nextInAttachmentChain) == 0x8);
C_ASSERT(offsetof(Enemy, vm) == 0xc);
C_ASSERT(offsetof(Enemy, mainEclContextStorage) == 0x7f8);
C_ASSERT(offsetof(Enemy, mainEclCallStackStorage) == 0xa20);
C_ASSERT(offsetof(Enemy, activeEclContext) == 0x2ca0);
C_ASSERT(offsetof(Enemy, activeEclCallStack) == 0x2ca4);
C_ASSERT(offsetof(Enemy, eclIntVariables) == 0x2ca8);
C_ASSERT(offsetof(Enemy, eclFloatVariables) == 0x2cc8);
C_ASSERT(offsetof(Enemy, mainEclCallStackDepth) == 0x2ce8);
C_ASSERT(offsetof(Enemy, activeEclCallStackDepth) == 0x2cea);
C_ASSERT(offsetof(Enemy, unconsumedWord2CEC) == 0x2cec);
C_ASSERT(offsetof(Enemy, deathCallbackSubId) == 0x2cee);
C_ASSERT(offsetof(Enemy, eclSubroutineIds) == 0x2cf0);
C_ASSERT(offsetof(Enemy, pendingEclSubroutineIndex) == 0x2d30);
C_ASSERT(offsetof(Enemy, positionAlignment2D32) == 0x2d32);
C_ASSERT(offsetof(Enemy, position) == 0x2d34);
C_ASSERT(offsetof(Enemy, positionOffset) == 0x2d40);
C_ASSERT(offsetof(Enemy, velocity) == 0x2d4c);
C_ASSERT(offsetof(Enemy, previousPosition) == 0x2d58);
C_ASSERT(offsetof(Enemy, lastFrameDisplacement) == 0x2d64);
C_ASSERT(offsetof(Enemy, hitboxDimensions) == 0x2d70);
C_ASSERT(offsetof(Enemy, secondaryHitboxDimensions) == 0x2d7c);
C_ASSERT(offsetof(Enemy, worldPosition) == 0x2d88);
C_ASSERT(offsetof(Enemy, movementAngle) == 0x2d94);
C_ASSERT(offsetof(Enemy, angularVelocity) == 0x2d98);
C_ASSERT(offsetof(Enemy, orbitAngle) == 0x2d9c);
C_ASSERT(offsetof(Enemy, orbitAngularVelocity) == 0x2da0);
C_ASSERT(offsetof(Enemy, parentEnemy) == 0x2da4);
C_ASSERT(offsetof(Enemy, speed) == 0x2da8);
C_ASSERT(offsetof(Enemy, acceleration) == 0x2dac);
C_ASSERT(offsetof(Enemy, orbitRadius) == 0x2db0);
C_ASSERT(offsetof(Enemy, radialVelocity) == 0x2db4);
C_ASSERT(offsetof(Enemy, shootOffset) == 0x2db8);
C_ASSERT(offsetof(Enemy, movementInterpolationDelta) == 0x2dc4);
C_ASSERT(offsetof(Enemy, movementInterpolationOrigin) == 0x2dd0);
C_ASSERT(offsetof(Enemy, movementTimer) == 0x2ddc);
C_ASSERT(offsetof(Enemy, movementDuration) == 0x2de8);
C_ASSERT(offsetof(Enemy, bulletRankInfluence) == 0x2dec);
C_ASSERT(offsetof(Enemy, life) == 0x2dfc);
C_ASSERT(offsetof(Enemy, maxLife) == 0x2e00);
C_ASSERT(offsetof(Enemy, phaseStartingLife) == 0x2e04);
C_ASSERT(offsetof(Enemy, score) == 0x2e08);
C_ASSERT(offsetof(Enemy, enemyIndex) == 0x2e0c);
C_ASSERT(offsetof(Enemy, playerShotHitAccumulator) == 0x2e10);
C_ASSERT(offsetof(Enemy, bossTimer) == 0x2e14);
C_ASSERT(offsetof(Enemy, displayColor) == 0x2e20);
C_ASSERT(offsetof(Enemy, bulletSpawnDescriptor) == 0x2e24);
C_ASSERT(offsetof(Enemy, pendingShotInstruction) == 0x3034);
C_ASSERT(offsetof(Enemy, shootIntervalFrames) == 0x3060);
C_ASSERT(offsetof(Enemy, shootIntervalTimer) == 0x3064);
C_ASSERT(offsetof(Enemy, laserSpawnDescriptor) == 0x3070);
C_ASSERT(offsetof(Enemy, laserSlots) == 0x3280);
C_ASSERT(offsetof(Enemy, selectedLaserSlot) == 0x3300);
C_ASSERT(offsetof(Enemy, itemDropType) == 0x3304);
C_ASSERT(offsetof(Enemy, pointItemDropCount) == 0x3308);
C_ASSERT(offsetof(Enemy, powerOrPointItemDropCount) == 0x330c);
C_ASSERT(offsetof(Enemy, deathAnm1) == 0x3310);
C_ASSERT(offsetof(Enemy, bossSlot) == 0x3313);
C_ASSERT(offsetof(Enemy, damageFlashTimer) == 0x3314);
C_ASSERT(offsetof(Enemy, timerAlignment3315) == 0x3315);
C_ASSERT(offsetof(Enemy, timer3318) == 0x3318);
C_ASSERT(offsetof(Enemy, flags1) == 0x3324);
C_ASSERT(offsetof(Enemy, flags2) == 0x3328);
C_ASSERT(offsetof(Enemy, unconsumedWord332C) == 0x332c);
C_ASSERT(offsetof(Enemy, anmDirection) == 0x332e);
C_ASSERT(offsetof(Enemy, drawGroup) == 0x332f);
C_ASSERT(offsetof(Enemy, eclDifficultyMaskOverride) == 0x3330);
C_ASSERT(offsetof(Enemy, anmScriptsAlignment3331) == 0x3331);
C_ASSERT(offsetof(Enemy, anmScripts) == 0x3332);
C_ASSERT(offsetof(Enemy, movementBoundsAlignment333E) == 0x333e);
C_ASSERT(offsetof(Enemy, movementBounds) == 0x3340);
C_ASSERT(offsetof(Enemy, minimumPlayerDistanceSquared) == 0x3350);
C_ASSERT(offsetof(Enemy, lastDamage) == 0x3354);
C_ASSERT(offsetof(Enemy, lifeCallbackThresholds) == 0x3358);
C_ASSERT(offsetof(Enemy, lifeCallbackSubIds) == 0x3368);
C_ASSERT(offsetof(Enemy, timerCallbackThresholdFrames) == 0x3378);
C_ASSERT(offsetof(Enemy, timerCallbackSubId) == 0x337c);
C_ASSERT(offsetof(Enemy, linkedChildCount) == 0x3380);
C_ASSERT(offsetof(Enemy, childEclBlocks) == 0x3384);
C_ASSERT(offsetof(Enemy, trailSamples) == 0x3394);
C_ASSERT(offsetof(Enemy, trailVertices) == 0x3e14);
C_ASSERT(offsetof(Enemy, trailFlags) == 0x534c);
C_ASSERT(offsetof(Enemy, trailLengthAlignment534D) == 0x534d);
C_ASSERT(offsetof(Enemy, trailHistoryLength) == 0x534e);
C_ASSERT(offsetof(Enemy, trailCollisionLength) == 0x5350);
C_ASSERT(offsetof(Enemy, trailSampleStride) == 0x5352);
C_ASSERT(offsetof(Enemy, damageReductionTimer) == 0x5354);
C_ASSERT(offsetof(Enemy, attachedEffects) == 0x5360);
C_ASSERT(offsetof(Enemy, attachedEffectCount) == 0x53c0);
C_ASSERT(offsetof(Enemy, attachedEffectDistance) == 0x53c4);
C_ASSERT(offsetof(Enemy, alignmentEffect) == 0x53c8);
C_ASSERT(offsetof(Enemy, phaseEndTimeRemainingSeconds) == 0x53cc);

struct EclTimelineInstruction
{
    i32 time;
    i16 opcode;
    u8 size;
    u8 difficultyMask;
    union
    {
        i32 ints[7];
        f32 floats[7];
    } args;
};

struct EclTimeline
{
    ZunTimer timer;
    EclTimelineInstruction *instruction;
    EclTimeline();
    void Run();
};
C_ASSERT(sizeof(EclTimeline) == 0x10);

struct EnemyManager
{
    EnemyManager();

    Enemy spawnTemplate;
    Enemy enemies[481];
    Enemy *bosses[8];
    u16 enemyDropCounter;
    u16 enemyDropScheduleIndex;
    i32 activeEnemyCount;
    i32 unconsumedDword9DCDC8;
    i32 opcode163Value;
    EclTimeline timelines[16];
    ZunTimer timer;
    Enemy *drawGroupHeads[4];
    AnmLoaded *enemyAnm;
    AnmLoaded *alternateEnemyAnm;
    i32 unconsumedDword9DCEF4;
    i32 lastSpawnFailed;
    i32 timelineEventSlots[4];
    i32 suppressTimelineSpawns;

    void Initialize();
    void UpdateSubrank();
    static ZunResult RegisterChain();
    i32 OnUpdate();
    static ChainCallbackResult OnUpdateCallback(EnemyManager *enemyManager);
    static ChainCallbackResult OnDrawHighPrio(EnemyManager *enemyManager);
    ChainCallbackResult __fastcall OnDrawImpl(i32 drawGroup, i32 chainPriority);
    static ChainCallbackResult OnDrawLowPrio(EnemyManager *enemyManager);
    static ZunResult AddedCallback(EnemyManager *enemyManager);
    static ZunResult DeletedCallback(EnemyManager *enemyManager);
    Enemy *SpawnEnemy1(i32 eclSubroutineId, const D3DXVECTOR3 *position, i32 life, i32 itemDropType, i32 score,
                       i32 mirrorMovementX);
    Enemy *SpawnEnemy2(i32 eclSubroutineId, const D3DXVECTOR3 *position, i32 life, i32 itemDropType, i32 score,
                       i32 *contextInts);
    i32 KillAllNonBossEnemies(i32 maxScore, i32 totalScore);
    i32 HasBoss();
    static void CutChain();
};
C_ASSERT(sizeof(EnemyManager) == 0x9dcf10);
C_ASSERT(offsetof(EnemyManager, spawnTemplate) == 0x0);
C_ASSERT(offsetof(EnemyManager, enemies) == 0x53d0);
C_ASSERT(offsetof(EnemyManager, bosses) == 0x9dcda0);
C_ASSERT(offsetof(EnemyManager, enemyDropCounter) == 0x9dcdc0);
C_ASSERT(offsetof(EnemyManager, enemyDropScheduleIndex) == 0x9dcdc2);
C_ASSERT(offsetof(EnemyManager, activeEnemyCount) == 0x9dcdc4);
C_ASSERT(offsetof(EnemyManager, unconsumedDword9DCDC8) == 0x9dcdc8);
C_ASSERT(offsetof(EnemyManager, opcode163Value) == 0x9dcdcc);
C_ASSERT(offsetof(EnemyManager, timelines) == 0x9dcdd0);
C_ASSERT(offsetof(EnemyManager, timer) == 0x9dced0);
C_ASSERT(offsetof(EnemyManager, drawGroupHeads) == 0x9dcedc);
C_ASSERT(offsetof(EnemyManager, enemyAnm) == 0x9dceec);
C_ASSERT(offsetof(EnemyManager, alternateEnemyAnm) == 0x9dcef0);
C_ASSERT(offsetof(EnemyManager, unconsumedDword9DCEF4) == 0x9dcef4);
C_ASSERT(offsetof(EnemyManager, lastSpawnFailed) == 0x9dcef8);
C_ASSERT(offsetof(EnemyManager, timelineEventSlots) == 0x9dcefc);
C_ASSERT(offsetof(EnemyManager, suppressTimelineSpawns) == 0x9dcf0c);

DIFFABLE_EXTERN(EnemyManager, g_EnemyManager);

} /* namespace th08 */
