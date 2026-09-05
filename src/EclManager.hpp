#pragma once

#include "Global.hpp"
#include "AnmManager.hpp"
#include "Supervisor.hpp"
#include "ZunResult.hpp"

#include <stddef.h>

namespace th08
{

struct Enemy;
struct AnmVm;
struct Effect;
struct EclTimelineInstruction;

typedef i32 (__fastcall *EffectUpdateCallback)(Effect *effect);
typedef i32 (__fastcall *EffectInitializeCallback)(Effect *effect);
typedef i32 (__fastcall *EffectDrawCallback)(Effect *effect);

// Stable IDs whose template callbacks or production call sites establish a
// reusable role.  The table has 66 entries; values without sufficient
// authored evidence intentionally remain unnamed rather than receiving a
// speculative visual description.
enum EffectId
{
    EFFECT_ANM_SCRIPT_28 = 0,
    EFFECT_SHOT_HIT_LARGE = 3,
    EFFECT_SHOT_HIT = 5,
    EFFECT_DEATH_OR_BOMB_PARTICLE = 6,
    EFFECT_GRAZE = 8,
    EFFECT_PLAYER_DEATH_OR_BOMB_RING = 12,
    EFFECT_SPELLCARD_ORBIT = 13,
    EFFECT_BOMB_PARTICLE = 20,
    EFFECT_FOCUS_AURA = 22,
    EFFECT_BOMB_STATE = 23,
    EFFECT_EXTREME_GAUGE = 25,
    EFFECT_FOCUS_RELEASE = 28,
    EFFECT_FOCUS_ENTER = 29,
    EFFECT_FAMILIAR_UNHIDE = 30,
    EFFECT_FAMILIAR_HIDE = 31,
    EFFECT_ALIGNMENT_BASE = 32,
    EFFECT_BARRIER_RADIAL_TRAIL = 35,
    EFFECT_QUADRUPLE_BARRIER = 36,
    EFFECT_QUADRUPLE_BARRIER_ROTATING = 37,
    EFFECT_FAMILIAR_FORM = 38,
    EFFECT_BOMB_RADIAL_TRAIL = 40,
    EFFECT_EXPANDING_WAVY_RADIAL_TRAIL = 42,
    EFFECT_EXPANDING_POSITIVE_DIAGONAL_RADIAL_TRAIL = 43,
    EFFECT_EXPANDING_NEGATIVE_DIAGONAL_RADIAL_TRAIL = 44,
    EFFECT_EXPANDING_OCTAGONAL_RADIAL_TRAIL = 45,
    EFFECT_EXPANDING_TWELVE_SEGMENT_RADIAL_TRAIL = 46,
    EFFECT_PLAYER_OPTION = 47,
    EFFECT_EXPANDING_ORTHOGONAL_RADIAL_TRAIL = 48,
    EFFECT_DREAM_SEAL_RADIAL_TRAIL = 49,
    EFFECT_DISSOLVE_RADIAL_TRAIL = 50,
    EFFECT_TINTED_BOSS_TRACKING_PARTICLE = 51,
    EFFECT_FADING_RADIAL_TRAIL = 53,
    EFFECT_DREAM_SEAL_FLASH = 55,
    EFFECT_NARROW_BULLET_WARP_BARRIER = 56,
    EFFECT_WIDE_BULLET_WARP_BARRIER = 58,
    EFFECT_PLAYER_DEATH_FIXED_SLOT = 59,
    EFFECT_BACKGROUND_SPECIAL = 62,
    EFFECT_RISING_BOSS_TRACKING_PARTICLE = 63,
    EFFECT_STAGE_ANM_HOST = 64,
    EFFECT_MEDIUM_BULLET_WARP_BARRIER = 65,
};

struct Effect
{
    AnmVm vm;
    Float3 position;
    Float3 vector1;
    Float3 vector2;
    Float3 vector3;
    Float3 vector4;
    Float3 vector5;
    Float3 vector6;
    Float3 vector7;
    Float3 orientationAxis;
    f32 orientationW;
    f32 radius;
    f32 angle;
    u32 unconsumedDword31C;
    f32 shapeThickness;
    i32 vertexSegmentCount;
    i32 slotIndex;
    f32 secondaryRadius;
    f32 secondaryAngle;
    f32 radialWaveCount;
    ZunTimer timer;
    u32 unconsumedDword344;
    EffectUpdateCallback updateCallback;
    EffectDrawCallback drawCallback;
    i8 active;
    i8 effectId;
    i8 releaseRequested;
    i8 releaseTimer;
    i8 drawGroup;
    i8 alternateDrawGroup;
    i8 verticesDirty;
    i8 updateDuringFreeze;
    VertexTex1DiffuseXyzrhw *vertices;
    Effect *nextInDrawGroup;

    Effect();
};
C_ASSERT(sizeof(Effect) == 0x360);
C_ASSERT(offsetof(Effect, position) == 0x2a4);
C_ASSERT(offsetof(Effect, vector5) == 0x2e0);
C_ASSERT(offsetof(Effect, vector6) == 0x2ec);
C_ASSERT(offsetof(Effect, orientationAxis) == 0x304);
C_ASSERT(offsetof(Effect, orientationW) == 0x310);
C_ASSERT(offsetof(Effect, shapeThickness) == 0x320);
C_ASSERT(offsetof(Effect, vertexSegmentCount) == 0x324);
C_ASSERT(offsetof(Effect, slotIndex) == 0x328);
C_ASSERT(offsetof(Effect, secondaryRadius) == 0x32c);
C_ASSERT(offsetof(Effect, secondaryAngle) == 0x330);
C_ASSERT(offsetof(Effect, radialWaveCount) == 0x334);
C_ASSERT(offsetof(Effect, radius) == 0x314);
C_ASSERT(offsetof(Effect, angle) == 0x318);
C_ASSERT(offsetof(Effect, unconsumedDword31C) == 0x31c);
C_ASSERT(offsetof(Effect, timer) == 0x338);
C_ASSERT(offsetof(Effect, unconsumedDword344) == 0x344);
C_ASSERT(offsetof(Effect, updateCallback) == 0x348);
C_ASSERT(offsetof(Effect, drawCallback) == 0x34c);
C_ASSERT(offsetof(Effect, active) == 0x350);
C_ASSERT(offsetof(Effect, effectId) == 0x351);
C_ASSERT(offsetof(Effect, releaseRequested) == 0x352);
C_ASSERT(offsetof(Effect, releaseTimer) == 0x353);
C_ASSERT(offsetof(Effect, drawGroup) == 0x354);
C_ASSERT(offsetof(Effect, alternateDrawGroup) == 0x355);
C_ASSERT(offsetof(Effect, verticesDirty) == 0x356);
C_ASSERT(offsetof(Effect, updateDuringFreeze) == 0x357);
C_ASSERT(offsetof(Effect, vertices) == 0x358);
C_ASSERT(offsetof(Effect, nextInDrawGroup) == 0x35c);

struct EffectManager
{
    EffectManager();

    i32 nextEffectIndex;
    i32 unconsumedDword04;
    i32 activeCount;
    f32 scaleX;
    f32 scaleY;
    f32 scaleZ;
    f32 scaleW;
    Effect effects[654];
    Effect drawGroupSentinel0;
    Effect drawGroupSentinel1;
    Effect drawGroupSentinel2;
    Effect drawGroupSentinel3;
    Effect drawGroupSentinel4;
    Effect *drawGroupTails[5];
    i32 tamperCheckCounter;
    AnmLoaded *effectAnm;
    AnmLoaded *stageEffectAnm;

    void ResetEffects();
    static ChainCallbackResult OnUpdate(EffectManager *effectManager);
    static ChainCallbackResult OnDraw(EffectManager *effectManager);
    static ZunResult LoadEffectResources(EffectManager *effectManager);
    static ZunResult ReleaseEffectResources(EffectManager *effectManager);
    static ZunResult RegisterChain();
    static void CutChain();
    Effect *SpawnEffectInSecondaryPool(i32 id, D3DXVECTOR3 *position, i32 count, i32 color);
    Effect *SpawnEffect(i32 id, D3DXVECTOR3 *position, i32 count, i32 color);
    Effect *SpawnEffectWithVelocity(i32 id, D3DXVECTOR3 *position, D3DXVECTOR3 *velocity, i32 count, i32 color);
    Effect *GetFixedSlotEffect(i32 index);
    Effect *SpawnEffectInFixedSlot(i32 id, D3DXVECTOR3 *position, i32 slotIndex, i32 unused, i32 color);
    Effect *SpawnEffectInFixedSlotWithVelocity(i32 id, D3DXVECTOR3 *position, D3DXVECTOR3 *velocity, i32 slotIndex, i32 unused, i32 color);
    i32 DrawBulletLayerEffects();
    i32 DrawBackgroundEffects();
};
C_ASSERT(sizeof(EffectManager) == 0x8b05c);
C_ASSERT(offsetof(EffectManager, unconsumedDword04) == 0x4);
C_ASSERT(offsetof(EffectManager, effects) == 0x1c);
C_ASSERT(offsetof(EffectManager, drawGroupSentinel0) == 0x89f5c);
C_ASSERT(offsetof(EffectManager, drawGroupSentinel1) == 0x8a2bc);
C_ASSERT(offsetof(EffectManager, drawGroupSentinel2) == 0x8a61c);
C_ASSERT(offsetof(EffectManager, drawGroupSentinel3) == 0x8a97c);
C_ASSERT(offsetof(EffectManager, drawGroupSentinel4) == 0x8acdc);
C_ASSERT(offsetof(EffectManager, drawGroupTails) == 0x8b03c);
C_ASSERT(offsetof(EffectManager, tamperCheckCounter) == 0x8b050);
C_ASSERT(offsetof(EffectManager, effectAnm) == 0x8b054);
C_ASSERT(offsetof(EffectManager, stageEffectAnm) == 0x8b058);
extern EffectManager g_EffectManager;
void __fastcall ShiftStageEffectOrigins(Float3 *delta);
DIFFABLE_EXTERN(ChainElem, g_EffectManagerCalcChain);
DIFFABLE_EXTERN(ChainElem, g_EffectManagerDrawChain);

DIFFABLE_EXTERN(u32, g_EclGameTimeScaleFlags);

// TH08 1.00d ECL instruction protocol.  Every value is explicit because the
// serialized opcode number is part of the on-disk format; names describe the
// target-observed handler behavior without changing EclRawInstruction's i16
// wire field.  Opcode 163 deliberately retains a neutral protocol name: the
// authored corpus proves its manager storage, but not a narrower game role.
enum EclOpcode
{
    ECL_OPCODE_TERMINATE = 1,
    ECL_OPCODE_SET_SECONDARY_TIME = 2,
    ECL_OPCODE_NOP = 3,
    ECL_OPCODE_JUMP = 4,
    ECL_OPCODE_JUMP_DEC = 5,
    ECL_OPCODE_SET_INT = 6,
    ECL_OPCODE_SET_FLOAT = 7,
    ECL_OPCODE_SET_INT_RANDOM_SIGN = 8,
    ECL_OPCODE_SET_FLOAT_RANDOM_SIGN = 9,
    ECL_OPCODE_INT_ADD_ASSIGN = 10,
    ECL_OPCODE_INT_SUBTRACT_ASSIGN = 11,
    ECL_OPCODE_INT_MULTIPLY_ASSIGN = 12,
    ECL_OPCODE_INT_DIVIDE_ASSIGN = 13,
    ECL_OPCODE_INT_MODULO_ASSIGN = 14,
    ECL_OPCODE_FLOAT_ADD_ASSIGN = 15,
    ECL_OPCODE_FLOAT_SUBTRACT_ASSIGN = 16,
    ECL_OPCODE_FLOAT_MULTIPLY_ASSIGN = 17,
    ECL_OPCODE_FLOAT_DIVIDE_ASSIGN = 18,
    ECL_OPCODE_FLOAT_MODULO_ASSIGN = 19,
    ECL_OPCODE_INT_ADD = 20,
    ECL_OPCODE_INT_SUBTRACT = 21,
    ECL_OPCODE_INT_MULTIPLY = 22,
    ECL_OPCODE_INT_DIVIDE = 23,
    ECL_OPCODE_INT_MODULO = 24,
    ECL_OPCODE_FLOAT_ADD = 25,
    ECL_OPCODE_FLOAT_SUBTRACT = 26,
    ECL_OPCODE_FLOAT_MULTIPLY = 27,
    ECL_OPCODE_FLOAT_DIVIDE = 28,
    ECL_OPCODE_FLOAT_MODULO = 29,
    ECL_OPCODE_INT_INCREMENT = 30,
    ECL_OPCODE_INT_DECREMENT = 31,
    ECL_OPCODE_FLOAT_SINE = 32,
    ECL_OPCODE_FLOAT_COSINE = 33,
    ECL_OPCODE_POINT_ANGLE = 34,
    ECL_OPCODE_INTERPOLATE_VALUE = 35,
    ECL_OPCODE_INSTALL_INTERPOLATION = 36,
    ECL_OPCODE_NORMALIZE_ANGLE = 37,
    ECL_OPCODE_POLAR_TO_CARTESIAN = 38,
    ECL_OPCODE_POINT_DISTANCE = 39,
    ECL_OPCODE_JUMP_IF_INT_EQUAL = 40,
    ECL_OPCODE_JUMP_IF_FLOAT_EQUAL = 41,
    ECL_OPCODE_JUMP_IF_INT_NOT_EQUAL = 42,
    ECL_OPCODE_JUMP_IF_FLOAT_NOT_EQUAL = 43,
    ECL_OPCODE_JUMP_IF_INT_LESS = 44,
    ECL_OPCODE_JUMP_IF_FLOAT_LESS = 45,
    ECL_OPCODE_JUMP_IF_INT_LESS_EQUAL = 46,
    ECL_OPCODE_JUMP_IF_FLOAT_LESS_EQUAL = 47,
    ECL_OPCODE_JUMP_IF_INT_GREATER = 48,
    ECL_OPCODE_JUMP_IF_FLOAT_GREATER = 49,
    ECL_OPCODE_JUMP_IF_INT_GREATER_EQUAL = 50,
    ECL_OPCODE_JUMP_IF_FLOAT_GREATER_EQUAL = 51,
    ECL_OPCODE_CALL = 52,
    ECL_OPCODE_RETURN = 53,
    ECL_OPCODE_SET_MAIN_ANM = 54,
    ECL_OPCODE_SET_PRIMARY_ANM_SEQUENCE = 55,
    ECL_OPCODE_SET_PRIMARY_ANM_SCRIPTS = 56,
    ECL_OPCODE_SET_EXTRA_ANM_SCRIPT = 57,
    ECL_OPCODE_SET_MAIN_ANM_ALTERNATE = 58,
    ECL_OPCODE_SET_PRIMARY_ANM_SEQUENCE_ALTERNATE = 59,
    ECL_OPCODE_SET_PRIMARY_ANM_SCRIPTS_ALTERNATE = 60,
    ECL_OPCODE_SET_EXTRA_ANM_SCRIPT_ALTERNATE = 61,
    ECL_OPCODE_PLAY_SPECIAL_ANM = 62,
    ECL_OPCODE_SET_POSITION = 63,
    ECL_OPCODE_MOVE_TO = 64,
    ECL_OPCODE_SET_DIRECTION_AND_SPEED = 65,
    ECL_OPCODE_MOVE_IN_DIRECTION = 66,
    ECL_OPCODE_MOVE_RANDOM_IN_BOUNDS = 67,
    ECL_OPCODE_SET_AIMED_DIRECTION_AND_SPEED = 68,
    ECL_OPCODE_MOVE_IN_AIMED_DIRECTION = 69,
    ECL_OPCODE_SET_ANGULAR_VELOCITY = 70,
    ECL_OPCODE_SET_ACCELERATION = 71,
    ECL_OPCODE_ORBIT_AROUND_POINT = 72,
    ECL_OPCODE_ORBIT_AROUND_CURRENT_POSITION = 73,
    ECL_OPCODE_SET_ORBIT_VELOCITIES = 74,
    ECL_OPCODE_SET_MOVEMENT_BOUNDS = 75,
    ECL_OPCODE_DISABLE_MOVEMENT_BOUNDS = 76,
    ECL_OPCODE_SET_HITBOX = 77,
    ECL_OPCODE_SET_SECONDARY_HITBOX = 78,
    ECL_OPCODE_SET_INTERACTION_FLAGS = 79,
    ECL_OPCODE_DISABLE_INTERACTION_FLAGS = 80,
    ECL_OPCODE_ENABLE_INTERACTION_FLAGS = 81,
    ECL_OPCODE_SET_MINIMUM_PLAYER_DISTANCE = 82,
    ECL_OPCODE_SET_FORM_EFFECT_ENABLED = 83,
    ECL_OPCODE_NOP_84 = 84,
    ECL_OPCODE_NOP_85 = 85,
    ECL_OPCODE_SET_REMOTE_INT = 86,
    ECL_OPCODE_SET_REMOTE_FLOAT = 87,
    ECL_OPCODE_CALL_REMOTE = 88,
    ECL_OPCODE_SCHEDULE_REMOTE_SUBROUTINE = 89,
    ECL_OPCODE_SPAWN_FAMILIAR_AT_POSITION = 90,
    ECL_OPCODE_SPAWN_FAMILIAR_AT_OFFSET = 91,
    ECL_OPCODE_SPAWN_FAMILIAR_INHERITING_POSITION = 92,
    ECL_OPCODE_SPAWN_ENEMY_AT_POSITION = 93,
    ECL_OPCODE_SPAWN_ENEMY_RELATIVE = 94,
    ECL_OPCODE_KILL_ALL_NON_BOSS_ENEMIES = 95,
    ECL_OPCODE_SHOOT_FAN_AIMED = 96,
    ECL_OPCODE_SHOOT_FAN = 97,
    ECL_OPCODE_SHOOT_CIRCLE_AIMED = 98,
    ECL_OPCODE_SHOOT_CIRCLE = 99,
    ECL_OPCODE_SHOOT_OFFSET_CIRCLE_AIMED = 100,
    ECL_OPCODE_SHOOT_OFFSET_CIRCLE = 101,
    ECL_OPCODE_SHOOT_RANDOM_ANGLE = 102,
    ECL_OPCODE_SHOOT_RANDOM_SPEED = 103,
    ECL_OPCODE_SHOOT_RANDOM = 104,
    ECL_OPCODE_SET_SHOOT_INTERVAL = 105,
    ECL_OPCODE_SET_SHOOT_INTERVAL_DELAYED = 106,
    ECL_OPCODE_ENABLE_DEFERRED_SHOOTING = 107,
    ECL_OPCODE_DISABLE_DEFERRED_SHOOTING = 108,
    ECL_OPCODE_SHOOT_NOW = 109,
    ECL_OPCODE_SET_SHOOT_OFFSET = 110,
    ECL_OPCODE_SET_BULLET_TRANSFORM = 111,
    ECL_OPCODE_CLEAR_BULLETS_FOR_TRANSITION = 112,
    ECL_OPCODE_SET_BULLET_SOUNDS = 113,
    ECL_OPCODE_CREATE_LASER = 114,
    ECL_OPCODE_CREATE_LASER_AIMED = 115,
    ECL_OPCODE_SELECT_LASER_SLOT = 116,
    ECL_OPCODE_ROTATE_LASER = 117,
    ECL_OPCODE_AIM_LASER_AT_PLAYER = 118,
    ECL_OPCODE_SET_LASER_POSITION = 119,
    ECL_OPCODE_TEST_LASER_ACTIVE = 120,
    ECL_OPCODE_CANCEL_LASER = 121,
    ECL_OPCODE_START_SPELL = 122,
    ECL_OPCODE_END_SPELL = 123,
    ECL_OPCODE_PLAY_POSITIONED_SOUND = 124,
    ECL_OPCODE_CALL_SUBROUTINE_SLOT = 125,
    ECL_OPCODE_SET_SUBROUTINE_SLOT = 126,
    ECL_OPCODE_SET_BOSS = 127,
    ECL_OPCODE_ATTACH_SPELL_EFFECT = 128,
    ECL_OPCODE_SET_DEATH_MODE = 129,
    ECL_OPCODE_SET_DEATH_CALLBACK = 130,
    ECL_OPCODE_SET_LIFE = 131,
    ECL_OPCODE_SET_BOSS_TIMER = 132,
    ECL_OPCODE_SET_LIFE_CALLBACK = 133,
    ECL_OPCODE_SET_TIMER_CALLBACK = 134,
    ECL_OPCODE_SET_CHILD_ECL = 135,
    ECL_OPCODE_CALL_EX_INSTRUCTION = 136,
    ECL_OPCODE_SET_REPEATING_EX_INSTRUCTION = 137,
    ECL_OPCODE_SET_DEATH_ANM_SCRIPTS = 138,
    ECL_OPCODE_SPAWN_EFFECT = 139,
    ECL_OPCODE_SPAWN_EFFECT_WITH_VELOCITY = 140,
    ECL_OPCODE_SPAWN_ITEM = 141,
    ECL_OPCODE_DROP_POWER_OR_POINT_ITEMS = 142,
    ECL_OPCODE_SET_ITEM_DROP_TYPE = 143,
    ECL_OPCODE_SET_ITEM_DROP_COUNTS = 144,
    ECL_OPCODE_SET_ANM_ROTATION_ENABLED = 145,
    ECL_OPCODE_ADD_TIME = 146,
    ECL_OPCODE_SET_BACKGROUND_SCRIPT_LABEL = 147,
    ECL_OPCODE_SET_BOSS_LIFE_MARKER_COUNT = 148,
    ECL_OPCODE_INTERRUPT_MAIN_ANM = 149,
    ECL_OPCODE_INTERRUPT_SECONDARY_ANM = 150,
    ECL_OPCODE_SET_CALL_STACK_DISABLED = 151,
    ECL_OPCODE_SET_BULLET_RANK_INFLUENCE = 152,
    ECL_OPCODE_RESET_BOSS_TIMER_CALLBACK = 153,
    ECL_OPCODE_CLEAR_LASER_SLOTS = 154,
    ECL_OPCODE_SET_TIMEOUT_SPELL = 155,
    ECL_OPCODE_SET_SPECIAL_INTERACTION = 156,
    ECL_OPCODE_SET_TRAIL = 157,
    ECL_OPCODE_SET_BOSS_GAUGE_SLOT = 158,
    ECL_OPCODE_SET_DRAW_GROUP = 159,
    ECL_OPCODE_SET_DAMAGE_REDUCTION_TIMER = 160,
    ECL_OPCODE_REMOVE_BULLETS_IN_RADIUS = 161,
    ECL_OPCODE_REMOVE_ALL_BULLETS = 162,
    ECL_OPCODE_SET_MANAGER_PROTOCOL_VALUE = 163,
    ECL_OPCODE_SET_SPELL_EFFECT_TRACKING_DISABLED = 164,
    ECL_OPCODE_SET_MAIN_ANM_ROTATION = 165,
    ECL_OPCODE_POLAR_TO_CARTESIAN_ALT = 166,
    ECL_OPCODE_SET_LASER_ANGLE = 167,
    ECL_OPCODE_DROP_POINT_ITEMS = 168,
    ECL_OPCODE_RANDOM_HORIZONTAL_ANGLE = 169,
    ECL_OPCODE_SET_LASER_START_CAP_HIDDEN = 170,
    ECL_OPCODE_SET_LASER_START_LENGTH = 171,
    ECL_OPCODE_SET_LASER_OFFSETS = 172,
    ECL_OPCODE_SET_TIMER_PAUSED = 173,
    ECL_OPCODE_REPLACE_ALIGNMENT_EFFECT = 174,
    ECL_OPCODE_SET_TIMELINE_SPAWNS_SUPPRESSED = 175,
    ECL_OPCODE_PREPARE_STAGE_TIME_STOP = 176,
    ECL_OPCODE_SET_PHASE_STARTING_LIFE = 177,
    ECL_OPCODE_MOVE_RANDOM_BIASED = 178,
    ECL_OPCODE_START_STAGE_BACKGROUND_SEQUENCE = 179,
    ECL_OPCODE_HIDE_CLOCK_TIME = 180,
    ECL_OPCODE_INCREMENT_CLOCK_TIME = 181,
    ECL_OPCODE_SET_EXTRA_ANM_FIXED_OFFSET = 182,
    ECL_OPCODE_SET_NO_DAMAGE_DURING_STOP = 183,
    ECL_OPCODE_SET_BONUS_UPDATES_DISABLED = 184
};

// Selector bits consumed by opcodes 79..81.  They are ECL operand protocol
// bits, not the physical bit positions in Enemy::flags1/flags2.
enum EclInteractionFlag
{
    ECL_INTERACTION_ACCEPTS_DAMAGE = 1U << 0,
    ECL_INTERACTION_COLLISION = 1U << 1,
    ECL_INTERACTION_DAMAGEABLE = 1U << 2,
    ECL_INTERACTION_NO_SPRITE = 1U << 3,
    ECL_INTERACTION_ALLOW_OFFSCREEN = 1U << 4,
    ECL_INTERACTION_NO_DEATH = 1U << 5,
};

struct EclRawInstruction
{
    i32 time;
    i16 opcode;
    i16 nextOffset;
    u8 serializedReserved08;
    u8 difficultyMask;
    u16 operandFlags;
    u8 operands[1];
};

struct EclExInstruction
{
    i32 time;
    i16 opcode;
    i16 nextOffset;
    u8 serializedReserved08;
    u8 difficultyMask;
    u16 operandFlags;
    u8 serializedReserved0C[4];
    union
    {
        i32 value;
        i8 byteValue;
    };
};
C_ASSERT(offsetof(EclExInstruction, value) == 0x10);

typedef void (__fastcall *EclExInstructionCallback)(
    Enemy *enemy, EclExInstruction *instruction);
extern EclExInstructionCallback g_EclExInsn[32];

// Only the target-observed fixed header is named here. Both pointer tables are
// stored as file-relative offsets and are rebased in place by Load.
struct EclRawHeader
{
    u32 version;
    i16 subCount;
    i16 timelineCount;
    u32 timelineOffsets[16];
    u32 subOffsets[1];
};
typedef char EclRawHeaderTimelineCountOffsetCheck[offsetof(EclRawHeader, timelineCount) == 0x6 ? 1 : -1];
typedef char EclRawHeaderSubTableOffsetCheck[offsetof(EclRawHeader, subOffsets) == 0x48 ? 1 : -1];

struct EnemyEclInterpolationSlot;
typedef void (__fastcall *EnemyEclInterpolatorCallback)(
    Enemy *enemy, EnemyEclInterpolationSlot *slot, f32 progress);

struct EnemyEclInterpolationSlot
{
    EnemyEclInterpolationSlot();

    EnemyEclInterpolatorCallback callback;
    ZunTimer timer;
    i32 duration;
    i32 callbackIndex;
    i32 easing;
    f32 parameters[4];
    f32 affectedVariable;
};
C_ASSERT(sizeof(EnemyEclInterpolationSlot) == 0x30);
C_ASSERT(offsetof(EnemyEclInterpolationSlot, duration) == 0x10);
C_ASSERT(offsetof(EnemyEclInterpolationSlot, callbackIndex) == 0x14);
C_ASSERT(offsetof(EnemyEclInterpolationSlot, parameters) == 0x1c);
C_ASSERT(offsetof(EnemyEclInterpolationSlot, affectedVariable) == 0x2c);

// Target-observed TH08 per-enemy ECL interpreter context. The constructor-
// bearing Enemy storage type retains its target symbol separately, while this
// ABI view names the state consumed by EclManager and the exact interpreter.
struct EnemyEclContext
{
    EnemyEclContext();

    EclRawInstruction *currentInstr;
    ZunTimer time;
    EclExInstructionCallback perFrameCallback;
    EclExInstruction *perFrameInstruction;
    i32 intVariables[8];
    f32 floatVariables[8];
    i32 extraIntVariables[4];
    f32 extraFloatVariables[2];
    i32 callParameterInts[4];
    f32 callParameterFloats[4];
    ZunTimer secondaryTime;
    EnemyEclInterpolationSlot interpolationSlots[8];
    i32 unconsumedDword21C;
    i32 childContextSlot;
    i16 subId;
    u8 contextTailAlignment226[2];
};
typedef char EnemyEclContextCallbackOffsetCheck[offsetof(EnemyEclContext, perFrameCallback) == 0x10 ? 1 : -1];
typedef char EnemyEclContextIntVariablesOffsetCheck[offsetof(EnemyEclContext, intVariables) == 0x18 ? 1 : -1];
typedef char EnemyEclContextFloatVariablesOffsetCheck[offsetof(EnemyEclContext, floatVariables) == 0x38 ? 1 : -1];
typedef char EnemyEclContextExtraIntVariablesOffsetCheck[offsetof(EnemyEclContext, extraIntVariables) == 0x58 ? 1 : -1];
typedef char EnemyEclContextExtraFloatVariablesOffsetCheck[offsetof(EnemyEclContext, extraFloatVariables) == 0x68 ? 1 : -1];
typedef char EnemyEclContextCallParametersOffsetCheck[offsetof(EnemyEclContext, callParameterInts) == 0x70 ? 1 : -1];
typedef char EnemyEclContextSecondaryTimerOffsetCheck[offsetof(EnemyEclContext, secondaryTime) == 0x90 ? 1 : -1];
typedef char EnemyEclContextInterpolationSlotsOffsetCheck[offsetof(EnemyEclContext, interpolationSlots) == 0x9c ? 1 : -1];
typedef char EnemyEclContextUnconsumedOffsetCheck[offsetof(EnemyEclContext, unconsumedDword21C) == 0x21c ? 1 : -1];
typedef char EnemyEclContextChildSlotOffsetCheck[offsetof(EnemyEclContext, childContextSlot) == 0x220 ? 1 : -1];
typedef char EnemyEclContextSubIdOffsetCheck[offsetof(EnemyEclContext, subId) == 0x224 ? 1 : -1];
typedef char EnemyEclContextTailAlignmentOffsetCheck[offsetof(EnemyEclContext, contextTailAlignment226) == 0x226 ? 1 : -1];
typedef char EnemyEclContextSizeCheck[sizeof(EnemyEclContext) == 0x228 ? 1 : -1];

// Raw-allocated by ECL opcode 135. The target clears the complete block,
// installs one child context at +0x8, and uses the 16 following contexts as
// that child's call stack.
struct EnemyChildEclBlock
{
    i32 subId;
    u16 unconsumedWord04;
    i16 callStackDepth;
    EnemyEclContext eclContext;
    EnemyEclContext callStack[16];
};
C_ASSERT(sizeof(EnemyChildEclBlock) == 0x24b0);
C_ASSERT(offsetof(EnemyChildEclBlock, subId) == 0x0);
C_ASSERT(offsetof(EnemyChildEclBlock, unconsumedWord04) == 0x4);
C_ASSERT(offsetof(EnemyChildEclBlock, callStackDepth) == 0x6);
C_ASSERT(offsetof(EnemyChildEclBlock, eclContext) == 0x8);
C_ASSERT(offsetof(EnemyChildEclBlock, callStack) == 0x230);

struct EclTimelineState
{
    EclTimelineState();

    u8 unconsumedBytes000[0x100];
    D3DXVECTOR3 vectors[8];
};
C_ASSERT(sizeof(EclTimelineState) == 0x160);
C_ASSERT(offsetof(EclTimelineState, unconsumedBytes000) == 0x0);
C_ASSERT(offsetof(EclTimelineState, vectors) == 0x100);

struct EclManager
{
    EclManager();
    ZunResult Load(char *path);
    void Unload();
    ZunResult CallEclSub(EnemyEclContext *context, i16 subId);
    ZunResult RunEcl(Enemy *enemy);
    i32 GetTimelineCount();
    EclTimelineInstruction *GetTimeline(i32 index);

    EclRawHeader *eclFile;             // +0x000
    u32 *subTable;                     // +0x004
    EclTimelineState timelineState;    // +0x008
};
C_ASSERT(sizeof(EclManager) == 0x168);
C_ASSERT(offsetof(EclManager, timelineState) == 0x8);

DIFFABLE_EXTERN(EclManager, g_EclManager);

} // namespace th08
