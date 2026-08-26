#include "th_pch.h"

#include "AsciiManager.hpp"
#include "Background.hpp"
#include "BulletManager.hpp"
#include "Gui.hpp"
#include "ItemManager.hpp"
#include "AnmManager.hpp"
#include "Player.hpp"
#include "SoundPlayer.hpp"
#include "ReplayManager.hpp"
#include "EnemyManager.hpp"
#include "Spellcard.hpp"
#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "ScreenEffect.hpp"
#include "utils.hpp"

namespace th08
{

// The target places the bomb/shot callback family at 0x0040BC20..0x004142C0,
// far before the main Player implementation that begins at 0x00449CA0. Its
// production definitions live in PlayerBomb.cpp.
DIFFABLE_STATIC(Player, g_Player);
DIFFABLE_STATIC(i32, g_PlayerNormalBombCount);
DIFFABLE_STATIC(i32, g_PlayerDeathbombCount);
DIFFABLE_STATIC_ARRAY(i16, 6, g_PlayerGaugeBounds);

DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 12, g_PlayerAnmFilenames) = {
    "player00.anm", "player01.anm", "player02.anm", "player03.anm",
    "player00.anm", "player00.anm", "player01.anm", "player01.anm",
    "player02.anm", "player02.anm", "player03.anm", "player03.anm",
};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 12, g_Player1ShtFiles) = {
    "ply00a.sht", "ply01a.sht", "ply02a.sht", "ply03a.sht",
    "ply00a.sht", "ply00as.sht", "ply01a.sht", "ply01as.sht",
    "ply02a.sht", "ply02as.sht", "ply03a.sht", "ply03as.sht",
};
DIFFABLE_STATIC_ARRAY_ASSIGN(const char *, 12, g_Player2ShtFile) = {
    "ply00as.sht", "ply01as.sht", "ply02as.sht", "ply03as.sht",
    "ply00a.sht", "ply00as.sht", "ply01a.sht", "ply01as.sht",
    "ply02a.sht", "ply02as.sht", "ply03a.sht", "ply03as.sht",
};

void __fastcall FUN_0040c010(Player *player);
void __fastcall FUN_0040c820(Player *player);
void __fastcall FUN_0040c910(Player *player);
void __fastcall FUN_0040d010(Player *player);
void __fastcall FUN_0040d100(Player *player);
void __fastcall FUN_0040d310(Player *player);
void __fastcall FUN_0040d430(Player *player);
void __fastcall FUN_0040d950(Player *player);
void __fastcall FUN_0040d970(Player *player);
void __fastcall FUN_0040dee0(Player *player);
void __fastcall FUN_0040e3b0(Player *player);
void __fastcall FUN_0040e610(Player *player);
void __fastcall FUN_0040e780(Player *player);
void __fastcall FUN_0040ee10(Player *player);
void __fastcall FUN_0040f550(Player *player);
void __fastcall FUN_0040f570(Player *player);
void __fastcall FUN_0040fcb0(Player *player);
void __fastcall FUN_0040fcd0(Player *player);
void __fastcall FUN_00410300(Player *player);
void __fastcall FUN_004103f0(Player *player);
void __fastcall FUN_00410ac0(Player *player);
void __fastcall FUN_00410c40(Player *player);
void __fastcall FUN_00410fe0(Player *player);
void __fastcall FUN_004113a0(Player *player);
void __fastcall FUN_00411b10(Player *player);
void __fastcall FUN_00412300(Player *player);
void __fastcall FUN_004123d0(Player *player);
void __fastcall FUN_00412fa0(Player *player);
void __fastcall FUN_00413140(Player *player);
void __fastcall FUN_00413890(Player *player);
void __fastcall FUN_00413990(Player *player);
void __fastcall FUN_004142c0(Player *player);

DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerBombCallbacks, 24, g_PlayerBombCallbackTable) = {
    {{FUN_0040c010, FUN_00410c40, FUN_0040c910, FUN_00410fe0, FUN_0040d100}},
    {{FUN_0040c820, FUN_0040d950, FUN_0040d010, FUN_004113a0, FUN_0040d310}},
    {{FUN_0040e3b0, FUN_0040d430, FUN_0040e780, FUN_0040d970, FUN_0040d100}},
    {{FUN_0040e610, FUN_0040d950, FUN_0040e610, FUN_0040dee0, FUN_0040d310}},
    {{FUN_0040fcd0, FUN_0040ee10, FUN_004103f0, FUN_0040f570, FUN_0040d100}},
    {{FUN_00410300, FUN_0040f550, FUN_00410ac0, FUN_0040fcb0, FUN_0040d310}},
    {{FUN_00411b10, FUN_00413140, FUN_004123d0, FUN_00413990, FUN_0040d100}},
    {{FUN_00412300, FUN_00413890, FUN_00412fa0, FUN_004142c0, FUN_0040d310}},
    {{FUN_0040c010, FUN_0040c010, FUN_0040c910, FUN_0040c910, FUN_0040d100}},
    {{FUN_0040c820, FUN_0040c820, FUN_0040d010, FUN_0040d010, FUN_0040d310}},
    {{FUN_00410c40, FUN_00410c40, FUN_00410fe0, FUN_00410fe0, FUN_0040d100}},
    {{FUN_0040d950, FUN_0040d950, FUN_004113a0, FUN_004113a0, FUN_0040d310}},
    {{FUN_0040e3b0, FUN_0040e3b0, FUN_0040e780, FUN_0040e780, FUN_0040d100}},
    {{FUN_0040e610, FUN_0040e610, FUN_0040e610, FUN_0040e610, FUN_0040d310}},
    {{FUN_0040d430, FUN_0040d430, FUN_0040d970, FUN_0040d970, FUN_0040d100}},
    {{FUN_0040d950, FUN_0040d950, FUN_0040dee0, FUN_0040dee0, FUN_0040d310}},
    {{FUN_0040fcd0, FUN_0040fcd0, FUN_004103f0, FUN_004103f0, FUN_0040d100}},
    {{FUN_00410300, FUN_00410300, FUN_00410ac0, FUN_00410ac0, FUN_0040d310}},
    {{FUN_0040ee10, FUN_0040ee10, FUN_0040f570, FUN_0040f570, FUN_0040d100}},
    {{FUN_0040f550, FUN_0040f550, FUN_0040fcb0, FUN_0040fcb0, FUN_0040d310}},
    {{FUN_00411b10, FUN_00411b10, FUN_004123d0, FUN_004123d0, FUN_0040d100}},
    {{FUN_00412300, FUN_00412300, FUN_00412fa0, FUN_00412fa0, FUN_0040d310}},
    {{FUN_00413140, FUN_00413140, FUN_00413990, FUN_00413990, FUN_0040d100}},
    {{FUN_00413890, FUN_00413890, FUN_004142c0, FUN_004142c0, FUN_0040d310}},
};
typedef i32 (__fastcall *PlayerOptionCallback)(Player *, PlayerOptionState *);
typedef i32 (__fastcall *PlayerShotUpdateCallback)(Player *, PlayerShot *, i32, u8 *);
typedef i32 (__fastcall *PlayerShotRenderCallback)(Player *, PlayerShot *);

i32 __fastcall FUN_0044e3a0(Player *player, PlayerOptionState *option);
i32 __fastcall FUN_0044ea40(Player *player, PlayerOptionState *option);
i32 __fastcall FUN_0044eb70(Player *player, PlayerOptionState *option);
i32 __fastcall FUN_0044ee70(Player *player, PlayerOptionState *option);
i32 __fastcall FUN_0044f2d0(Player *player, PlayerOptionState *option);
i32 __fastcall FUN_0044f5e0(Player *player, PlayerOptionState *option);
i32 __fastcall FUN_0044f930(Player *player, PlayerOptionState *option);
i32 __fastcall PlayerRoute2OptionRender(Player *player, u8 *option);
i32 __fastcall FUN_00450080(Player *player, PlayerShot *slot, i32 value, u8 *entry);
i32 __fastcall FUN_00450110(Player *player, PlayerShot *slot, i32 value, u8 *entry);
i32 __fastcall FUN_004501b0(Player *player, PlayerShot *slot, i32 value, u8 *entry);
i32 __fastcall FUN_00450240(Player *player, PlayerShot *slot, i32 value, u8 *entry);
i32 __fastcall FUN_00450320(Player *player, PlayerShot *slot);
i32 __fastcall FUN_00450580(Player *player, PlayerShot *slot);
i32 __fastcall FUN_004505d0(Player *player, PlayerShot *slot);
i32 __fastcall FUN_00450840(Player *player, PlayerShot *slot);
i32 __fastcall FUN_00450ad0(Player *player, PlayerShot *slot);
i32 __fastcall FUN_00450c50(Player *player, PlayerShot *slot, Float3 *effectPosition);
i32 __fastcall FUN_00450ee0(Player *player, PlayerShot *slot, Float3 *effectPosition);

static i32 __fastcall PlayerShotUpdateFdd0(Player *player, PlayerShot *slot, i32 value, u8 *entry)
{
    return player->FUN_0044fdd0(reinterpret_cast<u8 *>(slot), value, entry);
}

static i32 __fastcall PlayerShotUpdateFe20(Player *player, PlayerShot *slot, i32 value, u8 *entry)
{
    return player->FUN_0044fe20(reinterpret_cast<u8 *>(slot), value, entry);
}

static i32 __fastcall PlayerShotUpdateFfa0(Player *player, PlayerShot *slot, i32 value, u8 *entry)
{
    return player->FUN_0044ffa0(reinterpret_cast<u8 *>(slot), value, entry);
}

struct PlayerOptionCallbackRow
{
    PlayerOptionCallback callbacks[4];
};
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerOptionCallbackRow, 12, g_PlayerOptionUpdateCallbacks) = {
    {{FUN_0044e3a0, NULL, NULL, NULL}},
    {{FUN_0044ea40, NULL, NULL, NULL}},
    {{FUN_0044eb70, FUN_0044eb70, FUN_0044eb70, FUN_0044eb70}},
    {{FUN_0044f930, FUN_0044f930, NULL, NULL}},
    {{NULL, NULL, NULL, NULL}},
    {{FUN_0044e3a0, NULL, NULL, NULL}},
    {{NULL, NULL, NULL, NULL}},
    {{FUN_0044ea40, NULL, NULL, NULL}},
    {{NULL, NULL, NULL, NULL}},
    {{FUN_0044ee70, FUN_0044ee70, FUN_0044ee70, FUN_0044ee70}},
    {{NULL, NULL, FUN_0044f5e0, NULL}},
    {{FUN_0044f930, FUN_0044f930, NULL, NULL}},
};
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerOptionCallbackRow, 12, g_PlayerOptionRenderCallbacks) = {
    {{reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender), NULL, NULL, NULL}},
    {{reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender), NULL, NULL, NULL}},
    {{reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender),
      reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender),
      reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender),
      reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender)}},
    {{reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender),
      reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender), NULL, NULL}},
    {{NULL, NULL, NULL, NULL}},
    {{reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender), NULL, NULL, NULL}},
    {{NULL, NULL, NULL, NULL}},
    {{reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender), NULL, NULL, NULL}},
    {{NULL, NULL, NULL, NULL}},
    {{reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender),
      reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender),
      reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender),
      reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender)}},
    {{NULL, NULL, reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender), NULL}},
    {{reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender),
      reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender), NULL, NULL}},
};
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerOptionCallback, 4, g_PlayerRoute3ExitUpdateCallbacks) = {
    NULL, NULL, FUN_0044f2d0, NULL};
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerOptionCallback, 4, g_PlayerRoute3ExitRenderCallbacks) = {
    NULL, NULL, reinterpret_cast<PlayerOptionCallback>(PlayerRoute2OptionRender), NULL};

DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerShotUpdateCallback, 9, g_PlayerShotUpdateCallbacks) = {
    NULL, FUN_00450240, PlayerShotUpdateFdd0, PlayerShotUpdateFdd0, PlayerShotUpdateFe20,
    PlayerShotUpdateFfa0, FUN_00450080, FUN_004501b0, FUN_00450110};
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerShotRenderCallback, 6, g_PlayerShotRenderCallbacks) = {
    NULL, FUN_00450320, NULL, FUN_00450580, FUN_004505d0, FUN_00450840};
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerShotRenderCallback, 2, g_PlayerShotTimerCallbacks) = {
    NULL, FUN_00450ad0};
typedef void *PlayerShotCollisionOrDifficultyEntry;
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerShotCollisionOrDifficultyEntry, 9,
                             g_PlayerShotCollisionOrDifficultyTable) = {
    NULL,
    reinterpret_cast<void *>(FUN_00450c50),
    reinterpret_cast<void *>(FUN_00450ee0),
    const_cast<char *>("Easy"),
    const_cast<char *>("Normal"),
    const_cast<char *>("Hard"),
    const_cast<char *>("Lunatic"),
    const_cast<char *>("Extra"),
    const_cast<char *>("LastWord")};

extern u16 g_GuiMessageInputCurrent;
extern u16 g_GuiMessageInputPrevious;

ZunBool IsResourceReloadDisabled();
void __fastcall PlayerBuildAabb(Float3 *topLeft, Float3 *bottomRight,
                                const Float3 *center, const Float3 *size);

// FUNCTION: th08 0x449ca0
Player::Player()
{
}

// FUNCTION: th08 0x449e50
PlayerOptionState::PlayerOptionState() {}

// FUNCTION: th08 0x449ea0
PlayerBombState::PlayerBombState() {}

// FUNCTION: th08 0x449ef0
PlayerShot::PlayerShot() {}

// FUNCTION: th08 0x449f70
PlayerBombWorkItem::PlayerBombWorkItem() {}

// FUNCTION: th08 0x449ff0
#pragma var_order(halfSize, yDelta, xDelta, i, rotated, delta, slot, boundsMax)
i32 Player::FUN_00449ff0(Float3 *position, Float3 *position2)
{
    Float3 delta;
    Float3 rotated;
    Float3 halfSize;
    Float3 boundsMax;
    PlayerUnkStruct0x40 *slot = this->playerSlotsC;
    i32 i;
    f32 xDelta;
    f32 yDelta;

    for (i = 0; i < ARRAY_SIZE_SIGNED(this->playerSlotsC); i++, slot++)
    {
        if (!slot->active)
            continue;

        if (slot->radius != 0.0)
        {
            xDelta = position->x - slot->center.x;
            yDelta = position->y - slot->center.y;
            if (xDelta * xDelta + yDelta * yDelta < slot->radius * slot->radius)
                goto hit;
            goto next;
        }

        if (slot->angle != 0.0f)
        {
            delta.x = position->x - slot->center.x;
            delta.y = position->y - slot->center.y;
            Rotate(&rotated, &delta, -slot->angle);
            halfSize.x = slot->size.x / 2.0f;
            halfSize.y = slot->size.y / 2.0f;
            if (-halfSize.x <= rotated.x && rotated.x <= halfSize.x && -halfSize.y <= rotated.y &&
                rotated.y <= halfSize.y)
                goto hit;
            goto next;
        }

        halfSize.x = slot->center.x - slot->size.x / 2.0f;
        halfSize.y = slot->center.y - slot->size.y / 2.0f;
        boundsMax.x = slot->size.x / 2.0f + slot->center.x;
        boundsMax.y = slot->size.y / 2.0f + slot->center.y;
        if (!(halfSize.x > position->x))
        {
            if (!(boundsMax.x < position->x))
            {
                if (!(halfSize.y > position->y))
                {
                    if (!(boundsMax.y < position->y))
                        goto hit;
                }
            }
        }
        goto next;

    next:
        ;
    }

    return 0;

hit:
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xe2a90) = slot->collisionValue;
    slot->hitAccumulator++;
    return 2;
}

// FUNCTION: th08 0x44a230
#pragma var_order(boundsMax, boundsMin)
i32 Player::FUN_0044a230(Float3 *position, Float3 *size)
{
    Float3 boundsMin;
    Float3 boundsMax;

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A90) = 6;
    if (this->FUN_00449ff0(position, size))
        return 2;

    boundsMin.x = position->x - size->x / 2.0f;
    boundsMin.y = position->y - size->y / 2.0f;
    boundsMax.x = size->x / 2.0f + position->x;
    boundsMax.y = size->y / 2.0f + position->y;

    if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x38C) > boundsMax.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x390) > boundsMax.y ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x398) < boundsMin.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x39C) < boundsMin.y)
        return 0;

    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 2;
    if (this->playerState != PLAYER_STATE_ALIVE)
        return 1;
    g_GameManager.RandomizeAntiTamper();
    this->Die();
    return 1;
}

// FUNCTION: th08 0x44a360
#pragma var_order(boundsMax, boundsMin)
i32 Player::FUN_0044a360(Float3 *position, Float3 *size)
{
    Float3 boundsMin;
    Float3 boundsMax;

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A90) = 6;
    boundsMin.x = position->x - size->x / 2.0f;
    boundsMin.y = position->y - size->y / 2.0f;
    boundsMax.x = size->x / 2.0f + position->x;
    boundsMax.y = size->y / 2.0f + position->y;

    if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x38C) > boundsMax.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x390) > boundsMax.y ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x398) < boundsMin.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x39C) < boundsMin.y)
        return 0;

    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 2;
    if (this->playerState != PLAYER_STATE_ALIVE)
        return 1;
    g_GameManager.RandomizeAntiTamper();
    this->Die();
    return 1;
}

// FUNCTION: th08 0x44a470
#pragma var_order(boundsMax, boundsMin)
i32 Player::FUN_0044a470(Float3 *position, Float3 *size)
{
    Float3 boundsMin;
    Float3 boundsMax;

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A90) = 6;
    if (this->FUN_00449ff0(position, size))
        return 2;

    boundsMin.x = position->x - size->x / 2.0f - 20.0f;
    boundsMin.y = position->y - size->y / 2.0f - 20.0f;
    boundsMax.x = size->x / 2.0f + position->x + 20.0f;
    boundsMax.y = size->y / 2.0f + position->y + 20.0f;

    if (this->playerState == PLAYER_STATE_DYING || this->playerState == PLAYER_STATE_SPAWNING)
        return 0;

    if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3A4) > boundsMax.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3B0) < boundsMin.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3A8) > boundsMax.y ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3B4) < boundsMin.y)
        return 0;

    this->FUN_0044a930(position, 0);
    return 1;
}

// FUNCTION: th08 0x44a5a0
#pragma var_order(itemMax, itemMin)
u32 Player::CalcItemBoxCollision(Float3 *position, Float3 *size)
{
    Float3 itemMin;
    Float3 itemMax;

    if (this->playerState != 0 && this->playerState != 3 && this->playerState != 4)
        return 0;

    itemMin = *position - *size / 2.0f;
    itemMax = *position + *size / 2.0f;

    if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3bc) > itemMax.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3c8) < itemMin.x ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3c0) > itemMax.y ||
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3cc) < itemMin.y)
        return 0;
    return 1;
}

// FUNCTION: th08 0x44a6a0
#pragma var_order(playerMin, incomingMax, incomingMin, playerMax)
u32 Player::CalcLaserHitbox(Float3 *position, Float3 *size, Float3 *origin, f32 angle, i32 graze)
{
    Float3 incomingMin;
    Float3 incomingMax;
    Float3 playerMin;
    Float3 playerMax;

    incomingMin = this->position - *origin;
    Rotate(&incomingMax, &incomingMin, -angle);
    incomingMax.z = 0.0f;
    incomingMin = incomingMax + *origin;

    playerMin = incomingMin - *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3D4);
    playerMax = incomingMin + *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3D4);
    incomingMin = *position - *size / 2.0f;
    incomingMax = *position + *size / 2.0f;

    if (!(playerMin.x > incomingMax.x))
    {
        if (!(playerMax.x < incomingMin.x))
        {
            if (!(playerMin.y > incomingMax.y))
            {
                if (!(playerMax.y < incomingMin.y))
                    goto lethalPath;
            }
        }
    }

grazePath:
    {
        if (!graze)
            return 0;

        incomingMin.x -= 48.0f;
        incomingMin.y -= 48.0f;
        incomingMax.x += 48.0f;
        incomingMax.y += 48.0f;

        if (playerMin.x > incomingMax.x || playerMax.x < incomingMin.x ||
            playerMin.y > incomingMax.y || playerMax.y < incomingMin.y)
            return 0;

        if (this->playerState == PLAYER_STATE_DYING || this->playerState == PLAYER_STATE_SPAWNING)
            return 0;
        this->FUN_0044a930(&this->position, 1);
        return 2;
    }

lethalPath:
    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 2;
    if (this->playerState != PLAYER_STATE_ALIVE)
        return 0;
    g_GameManager.RandomizeAntiTamper();
    this->Die();
    return 1;
}

// FUNCTION: th08 0x44a930
#pragma var_order(midpoint, gaugeGain, score)
void Player::FUN_0044a930(Float3 *position, i32 suppressExtraItems)
{
    Float3 midpoint;
    i32 gaugeGain;
    i32 score;

    if (g_Player.bombState.isInUse == 0)
    {
        gaugeGain = g_GameManager.GaugeIsExtremelyHuman()
                        ? 3
                        : (g_GameManager.GaugeIsModeratelyHuman() ? 2 : 1);

        if (g_GameManager.globals->grazeInStage < 99999)
            g_GameManager.globals->grazeInStage += gaugeGain;
        if (g_GameManager.globals->graze < 999999)
            g_GameManager.globals->graze += gaugeGain;
    }

    midpoint = (this->position + *position) / 2.0f;
    g_EffectManager.SpawnEffect(8, reinterpret_cast<D3DXVECTOR3 *>(&midpoint), 1, -1);
    g_GameManager.IncreaseSubrank(6);
    g_Gui.flags.grazeDisplayUpdateFrames = 2;
    g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(30), position->x);

    score = g_GameManager.GaugeIsModeratelyYoukai() ? 4000 : 2000;
    g_GameManager.AddScore(score);
    if (this->IsYoukai())
        g_GameManager.AddToYoukaiGauge(100, 0);

    if (!g_GameManager.IsSoloHuman() || g_GameManager.shotType == 10)
    {
        if (g_EnemyManager.HasBoss() && g_GameManager.GaugeIsExtremelyYoukai())
        {
            g_ItemManager.SpawnItem(position, ITEM_TIME2, 1);
            if (!suppressExtraItems && g_Spellcard.IsActive())
            {
                g_ItemManager.SpawnItem(position, ITEM_TIME2, 1);
                if (!g_GameManager.IsSoloYoukai())
                    g_ItemManager.SpawnItem(position, ITEM_TIME2, 1);
            }
        }
    }

    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 0x2000;
}

// FUNCTION: th08 0x44ab40
#pragma var_order(effectVm)
void Player::Die()
{
    AnmVm *effectVm;

    utils::DebugPrint("player DEAD");
    g_EclScriptedGlobalUpdateFreeze = 0;
    g_GameManager.UpdateAntiTamper();
    g_EffectManager.SpawnEffect(6, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 16, -1);
    this->playerState = PLAYER_STATE_DYING;
    this->timer = 0;
    g_SoundPlayer.PlaySoundPositionedByIdx(SOUND_PICHUN, this->position.x);
    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 0x200;

    // VC7 lowers the two-bit field read as two independent tests at /Od.
    // Read the typed flag storage once so the target's single extraction is
    // retained while the layout remains described by GameManagerFlags.
    if (((*reinterpret_cast<const u32 *>(&g_GameManager.flags) >>
          GameManagerFlags::PLAYER_DEATH_DISSOLVE_SHIFT) &
         GameManagerFlags::PLAYER_DEATH_DISSOLVE_MASK) != 0)
    {
        utils::DebugPrint(" desolve\n");
        this->deathbombWindowFrames = 2;
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x6) = 1;
    }
    else
    {
        g_GameManager.SetYoukaiGauge(0);
        if (g_GameManager.GetBombsRemaining() >= 1)
        {
            this->deathbombWindowFrames = g_GameManager.GetBombsRemaining() * 6;
            if (g_GameManager.GetTimeOrbs() >= g_GameManager.GetLastSpellTimeOrbThreshold())
                this->deathbombWindowFrames += 7;
            if (this->deathbombWindowFrames > 15)
                this->deathbombWindowFrames = 15;

            if (g_Spellcard.IsActive())
            {
                this->deathbombWindowFrames += this->deathbombWindowFrames;
                if (this->deathbombWindowFrames > 30)
                    this->deathbombWindowFrames = 30;
            }

            if (g_GameManager.shotType == 0 || g_GameManager.shotType == 4 ||
                g_GameManager.shotType == 5)
            {
                this->deathbombWindowFrames *= 9;
                this->deathbombWindowFrames /= 5;
            }

            utils::DebugPrint(" preDeadCount %d\n", this->deathbombWindowFrames);
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x206) = 0xFF;
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x205) = 0xFF;
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x204) = 0xFF;
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x207) =
                *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x203);
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x208) |= 0x20000;

            this->deathbombEffectVm =
                g_EffectManager.FUN_00425870(59, reinterpret_cast<D3DXVECTOR3 *>(&this->position),
                                              11, 1, 0xFFF0404F);
            effectVm = this->deathbombEffectVm;
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effectVm) + 0x50) = 0;
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(effectVm) + 0xA4) =
                this->deathbombWindowFrames;
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effectVm) + 0xF8) = 4;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x238) = 128.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x244) = 8.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x23C) = 32.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x248) = 0.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x208) = 128.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x20C) = 32.0f;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effectVm) + 0x324) = 64;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(effectVm) + 0x318) = 0;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x314) = 128.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x320) = 15.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(effectVm) + 0x334) = 6.0f;
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(effectVm) + 0x357) = 1;

            if (g_Spellcard.IsActive())
                g_GameManager.flags.unk10 = 1;
        }
        else
        {
            this->deathbombWindowFrames = 2;
            utils::DebugPrint(" Miss\n");
        }
    }

    g_ItemManager.CancelAutoCollect();
}

// FUNCTION: th08 0x44aec0
#pragma var_order(oldDirection, verticalSpeed, horizontalSpeed, focus, option, optionIndex, option2, optionExitIndex, route3Index, historyInitIndex, optionUpdateIndex, gaugeDelta, historyIndex, this)
i32 Player::FUN_0044aec0()
{
    typedef void (__fastcall *OptionUpdateCallback)(Player *, u8 *);

    i32 oldDirection;
    f32 verticalSpeed;
    f32 horizontalSpeed;
    i32 focus;
    u8 *option;
    u32 optionIndex;
    u8 *option2;
    u32 optionExitIndex;
    i32 route3Index;
    u32 historyInitIndex;
    u32 optionUpdateIndex;
    i32 gaugeDelta;
    i32 historyIndex;

    horizontalSpeed = 0.0f;
    verticalSpeed = 0.0f;
    oldDirection = *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98);

    if ((g_GuiMessageInputCurrent & 0x50) == 0x50)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 5;
    else if ((g_GuiMessageInputCurrent & 0x60) == 0x60)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 7;
    else if ((g_GuiMessageInputCurrent & 0x90) == 0x90)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 6;
    else if ((g_GuiMessageInputCurrent & 0xA0) == 0xA0)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 8;
    else if ((g_GuiMessageInputCurrent & 0x20) != 0)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 2;
    else if ((g_GuiMessageInputCurrent & 0x10) != 0)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 1;
    else if ((g_GuiMessageInputCurrent & 0x40) != 0)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 3;
    else if ((g_GuiMessageInputCurrent & 0x80) != 0)
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 4;
    else
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98) = 0;

    focus = this->bombState.isInUse
                ? (this->bombState.callbackSetIndex & 1)
                : (g_GuiMessageInputCurrent & 4);

    if (focus)
    {
        if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) != 1)
        {
            if (g_GameManager.shotType <= 3)
            {
                option = reinterpret_cast<u8 *>(this) + 0x40C;
                for (optionIndex = 0; optionIndex < 4; optionIndex++, option += 0x2F4)
                {
                    memset(option, 0, 0x2F4);
                    *reinterpret_cast<PlayerOptionCallback *>(option + 0x2EC) =
                        g_PlayerOptionUpdateCallbacks[g_GameManager.shotType].callbacks[optionIndex];
                    *reinterpret_cast<PlayerOptionCallback *>(option + 0x2F0) =
                        g_PlayerOptionRenderCallbacks[g_GameManager.shotType].callbacks[optionIndex];
                    if (*reinterpret_cast<void **>(option + 0x2EC) != NULL)
                    {
                        *reinterpret_cast<i32 *>(option + 0x2C8) = 1;
                        *reinterpret_cast<ZunTimer *>(option + 0x2E0) = 0;
                        *reinterpret_cast<i32 *>(option + 0x2D0) = optionIndex;
                    }
                    else
                    {
                        *reinterpret_cast<i32 *>(option + 0x2C8) = 0;
                    }
                }
            }

            if (g_GameManager.shotType < 4)
            {
                (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0xC))
                    ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x10), 5);
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) = 0.0f;
                if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) >= 4)
                    g_EffectManager.SpawnEffect(29, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 1, 0x80FF8080);
            }
            if (*reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xBE834) == NULL)
            {
                *reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xBE834) =
                    g_EffectManager.FUN_00425870(22, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 2, 1, -1);
            }
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) = 0;
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AE8) = 0;
        }
        else
        {
            ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8);
        }
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) >= 7)
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 5) = 1;
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) = 1;
    }
    else
    {
        if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) != 0)
        {
            option2 = reinterpret_cast<u8 *>(this) + 0x40C;
            if (g_GameManager.shotType < 3)
            {
                for (optionExitIndex = 0; optionExitIndex < 4; optionExitIndex++, option2 += 0x2F4)
                {
                    if (*reinterpret_cast<i32 *>(option2 + 0x2C8) != 0 &&
                        *reinterpret_cast<i32 *>(option2 + 0x2C8) != 3)
                    {
                        *reinterpret_cast<i32 *>(option2 + 0x2C8) = 3;
                        *reinterpret_cast<ZunTimer *>(option2 + 0x2E0) = 0;
                    }
                }
            }
            else if (g_GameManager.shotType == 3)
            {
                for (route3Index = 0; route3Index < 2; route3Index++, option2 += 0x2F4)
                {
                    if (*reinterpret_cast<i32 *>(option2 + 0x2C8) != 0 &&
                        *reinterpret_cast<i32 *>(option2 + 0x2C8) != 3)
                    {
                        *reinterpret_cast<i32 *>(option2 + 0x2C8) = 3;
                        *reinterpret_cast<ZunTimer *>(option2 + 0x2E0) = 0;
                    }
                }
                memset(option2, 0, 0x2F4);
                *reinterpret_cast<PlayerOptionCallback *>(option2 + 0x2EC) =
                    g_PlayerRoute3ExitUpdateCallbacks[route3Index];
                *reinterpret_cast<PlayerOptionCallback *>(option2 + 0x2F0) =
                    g_PlayerRoute3ExitRenderCallbacks[route3Index];
                *reinterpret_cast<i32 *>(option2 + 0x2C8) = 1;
                *reinterpret_cast<ZunTimer *>(option2 + 0x2E0) = 0;
                *reinterpret_cast<i32 *>(option2 + 0x2D0) = route3Index;
                for (historyInitIndex = 0; historyInitIndex < 16; ++historyInitIndex)
                    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2CC + historyInitIndex * 12) = this->position;
            }

            if (g_GameManager.shotType < 4)
            {
                (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0xC))
                    ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x10), 0);
                *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) = 0.0f;
                if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) >= 4)
                    g_EffectManager.SpawnEffect(28, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 1, 0x808080FF);
            }
            if (*reinterpret_cast<AnmVmBase **>(reinterpret_cast<u8 *>(this) + 0xBE834) != NULL)
                (*reinterpret_cast<AnmVmBase **>(reinterpret_cast<u8 *>(this) + 0xBE834))->SetInterrupt(1);
            *reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xBE834) = NULL;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) = 0;
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AE8) = 0;
        }
        else
        {
            ++*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8);
        }
        if (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) >= 7)
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 5) = 0;
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) = 0;
    }

    if (g_GameManager.shotType >= 4)
    {
        if ((g_GameManager.shotType & 1) != 0)
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 5) = 1;
        else
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 5) = 0;
    }

    if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) != 0)
    {
        switch (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98))
        {
        case 4: horizontalSpeed =  *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x28); break;
        case 3: horizontalSpeed = -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x28); break;
        case 1: verticalSpeed =   -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x28); break;
        case 2: verticalSpeed =    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x28); break;
        case 5: horizontalSpeed = -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x30); verticalSpeed = horizontalSpeed; break;
        case 7: verticalSpeed =    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x30); horizontalSpeed = -verticalSpeed; break;
        case 6: horizontalSpeed =  *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x30); verticalSpeed = -horizontalSpeed; break;
        case 8: horizontalSpeed =  *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->secondaryShtFile) + 0x30); verticalSpeed = horizontalSpeed; break;
        default: break;
        }
    }
    else
    {
        switch (*reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A98))
        {
        case 4: horizontalSpeed =  *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x24); break;
        case 3: horizontalSpeed = -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x24); break;
        case 1: verticalSpeed =   -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x24); break;
        case 2: verticalSpeed =    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x24); break;
        case 5: horizontalSpeed = -*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x2C); verticalSpeed = horizontalSpeed; break;
        case 7: verticalSpeed =    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x2C); horizontalSpeed = -verticalSpeed; break;
        case 6: horizontalSpeed =  *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x2C); verticalSpeed = -horizontalSpeed; break;
        case 8: horizontalSpeed =  *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this->primaryShtFile) + 0x2C); verticalSpeed = horizontalSpeed; break;
        default: break;
        }
    }

    horizontalSpeed *= *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x404);
    verticalSpeed *= *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x408);

#define SET_PLAYER_SCRIPT(idx) ((*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0xC))->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x10), (idx)))
    if (g_GameManager.shotType < 4)
    {
        if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) == 0)
        {
            if (horizontalSpeed < 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) >= 0.0f)
                SET_PLAYER_SCRIPT(1);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) < 0.0f)
                SET_PLAYER_SCRIPT(2);
            if (horizontalSpeed > 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) <= 0.0f)
                SET_PLAYER_SCRIPT(3);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) > 0.0f)
                SET_PLAYER_SCRIPT(4);
        }
        else
        {
            if (horizontalSpeed < 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) >= 0.0f)
                SET_PLAYER_SCRIPT(6);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) < 0.0f)
                SET_PLAYER_SCRIPT(7);
            if (horizontalSpeed > 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) <= 0.0f)
                SET_PLAYER_SCRIPT(8);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) > 0.0f)
                SET_PLAYER_SCRIPT(9);
        }
    }
    else
    {
        if ((g_GameManager.shotType & 1) != 0)
        {
            if (horizontalSpeed < 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) >= 0.0f)
                SET_PLAYER_SCRIPT(6);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) < 0.0f)
                SET_PLAYER_SCRIPT(7);
            if (horizontalSpeed > 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) <= 0.0f)
                SET_PLAYER_SCRIPT(8);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) > 0.0f)
                SET_PLAYER_SCRIPT(9);
        }
        else
        {
            if (horizontalSpeed < 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) >= 0.0f)
                SET_PLAYER_SCRIPT(1);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) < 0.0f)
                SET_PLAYER_SCRIPT(2);
            if (horizontalSpeed > 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) <= 0.0f)
                SET_PLAYER_SCRIPT(3);
            else if (horizontalSpeed == 0.0f && *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) > 0.0f)
                SET_PLAYER_SCRIPT(4);
        }
    }
#undef SET_PLAYER_SCRIPT

    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) = horizontalSpeed;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2AA0) = verticalSpeed;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3F8) = horizontalSpeed * g_EclGameTimeScale;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3FC) = verticalSpeed * g_EclGameTimeScale;
    this->position.operator float *()[0] += *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3F8);
    this->position.operator float *()[1] += *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x3FC);

    if (this->position.operator float *()[0] < g_GameManager.playerMovementTopLeftPos.x)
        this->position.operator float *()[0] = g_GameManager.playerMovementTopLeftPos.x;
    else if (this->position.operator float *()[0] >
             g_GameManager.playerMovementTopLeftPos.x + g_GameManager.playerMovementAreaSize.x)
    {
        this->position.operator float *()[0] =
            g_GameManager.playerMovementTopLeftPos.x + g_GameManager.playerMovementAreaSize.x;
    }
    if (this->position.operator float *()[1] < g_GameManager.playerMovementTopLeftPos.y)
        this->position.operator float *()[1] = g_GameManager.playerMovementTopLeftPos.y;
    else if (this->position.operator float *()[1] >
             g_GameManager.playerMovementTopLeftPos.y + g_GameManager.playerMovementAreaSize.y)
    {
        this->position.operator float *()[1] =
            g_GameManager.playerMovementTopLeftPos.y + g_GameManager.playerMovementAreaSize.y;
    }

    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x38C) = this->position - *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3D4);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x398) = this->position + *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3D4);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3A4) = this->position - *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3E0);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3B0) = this->position + *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3E0);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3BC) = this->position - *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3EC);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3C8) = this->position + *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x3EC);

    for (optionUpdateIndex = 0; optionUpdateIndex < 4; ++optionUpdateIndex)
    {
        if (*reinterpret_cast<OptionUpdateCallback *>(reinterpret_cast<u8 *>(this) + 0x6F8 + optionUpdateIndex * 0x2F4) != NULL)
        {
            (*reinterpret_cast<OptionUpdateCallback *>(reinterpret_cast<u8 *>(this) + 0x6F8 + optionUpdateIndex * 0x2F4))(
                this, reinterpret_cast<u8 *>(this) + 0x40C + optionUpdateIndex * 0x2F4);
            g_AnmManager->ExecuteScript(
                reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x40C + optionUpdateIndex * 0x2F4));
            (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0x6EC + optionUpdateIndex * 0x2F4))++;
        }
    }

    if ((g_GuiMessageInputCurrent & 1) != 0 && !g_Gui.IsDialogPresent() && !g_GameManager.IsTampered())
        this->FUN_00451640();

    if (!g_Gui.IsDialogPresent() && *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 8) >= 30 &&
        this->bombState.isInUse == 0)
    {
        gaugeDelta = 0;
        if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) >= 0)
        {
            if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AD0) > 0)
                (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AD0))--;
            else
            {
                gaugeDelta = (i32)((f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AE8) > 300.0f
                                       ? 21.0f
                                       : (f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AE8) / 15.0f);
                if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) == 0)
                    gaugeDelta = -gaugeDelta;
                g_GameManager.AddToYoukaiGauge((i32)((f32)gaugeDelta * g_EclGameTimeScale), 0);
                (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AE8))++;
            }
        }
        else
        {
            if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AD0) >= 4)
                *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AE8) = 0;
            if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AD0) >= 30)
            {
                if (fabs((double)g_GameManager.GetYoukaiGauge()) <= 9.0)
                {
                    g_GameManager.SetYoukaiGauge(0);
                }
                else
                {
                    if (g_GameManager.GaugeIsExtremelyYoukai()) gaugeDelta = -5;
                    else if (g_GameManager.GaugeIsModeratelyYoukai()) gaugeDelta = -3;
                    else if (g_GameManager.GetYoukaiGauge() > 0) gaugeDelta = -2;
                    else if (!g_GameManager.GaugeIsModeratelyHuman()) gaugeDelta = 2;
                    else if (!g_GameManager.GaugeIsExtremelyHuman()) gaugeDelta = 3;
                    else gaugeDelta = 5;
                    g_GameManager.AddToYoukaiGauge((i32)((f32)gaugeDelta * g_EclGameTimeScale), 0);
                }
            }
            else
                (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AD0))++;
        }
    }

    if ((g_GameManager.GaugeIsExtremelyHuman() || g_GameManager.GaugeIsExtremelyYoukai()) &&
        *reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xE2B24) == NULL)
    {
        *reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xE2B24) =
            g_EffectManager.FUN_00425870(25, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 8, 1, -1);
    }
    if (*reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xE2B24) != NULL)
    {
        *reinterpret_cast<Float3 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2B24) + 0x2A4) = this->position;
        if (!g_GameManager.GaugeIsExtremelyHuman() && !g_GameManager.GaugeIsExtremelyYoukai())
        {
            *reinterpret_cast<u8 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2B24) + 0x350) = 0;
            *reinterpret_cast<void **>(reinterpret_cast<u8 *>(this) + 0xE2B24) = NULL;
        }
    }

    if (verticalSpeed != 0.0f || horizontalSpeed != 0.0f)
    {
        for (historyIndex = 15; historyIndex > 0; --historyIndex)
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2CC + historyIndex * 12) =
                *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2CC + (historyIndex - 1) * 12);
        *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2CC) = this->position;
    }
    return 0;
}
// FUNCTION: th08 0x44c1b0
#pragma var_order(yDelta, xDelta, this)
f32 Player::FUN_0044c1b0(Float3 *position)
{
    f32 yDelta;
    f32 xDelta;

    xDelta = reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2B4)->operator float *()[0] - position->x;
    yDelta = reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2B4)->operator float *()[1] - position->y;

    if (yDelta == 0.0f && xDelta == 0.0f)
    {
        return ZUN_PI / 2.0f;
    }

    return VectorAngle(yDelta, xDelta);
}

#pragma var_order(primaryShtFile, player, secondaryShtFile)
// FUNCTION: th08 0x44c230
ZunResult Player::RegisterChain(u32 playerType)
{
    Player *player = &g_Player;
    PlayerRawShtFile *secondaryShtFile;
    PlayerRawShtFile *primaryShtFile;

    if (IsResourceReloadDisabled())
    {
        primaryShtFile = player->primaryShtFile;
        secondaryShtFile = player->secondaryShtFile;
    }

    memset(player, 0, sizeof(*player));

    if (IsResourceReloadDisabled())
    {
        player->primaryShtFile = primaryShtFile;
        player->secondaryShtFile = secondaryShtFile;
    }

    player->timer = 0;
    player->playerType = playerType;

    player->calcChain = g_Chain.CreateElem((ChainCallback)Player::OnUpdate);
    player->calcChain->arg = player;
    player->calcChain->addedCallback = (ChainLifetimeCallback)Player::AddedCallback;
    player->calcChain->deletedCallback = (ChainLifetimeCallback)Player::DeletedCallback;
    if (g_Chain.AddToCalcChain(player->calcChain, 9))
        return ZUN_ERROR;

    player->drawChainHighPrio = g_Chain.CreateElem((ChainCallback)Player::OnDrawHighPrio);
    player->drawChainLowPrio = g_Chain.CreateElem((ChainCallback)Player::OnDrawLowPrio);
    player->drawChainHighPrio->arg = player;
    player->drawChainLowPrio->arg = player;
    g_Chain.AddToDrawChain(player->drawChainHighPrio, 9);
    g_Chain.AddToDrawChain(player->drawChainLowPrio, 10);

    return ZUN_SUCCESS;
}


// FUNCTION: th08 0x44c390
ChainCallbackResult Player::OnUpdate(Player *player)
{
    if (*reinterpret_cast<i8 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x2C) != 0)
    {
        if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xBE834) != 0)
        {
            *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(player) + 0xBE834) + 0x1F8) |= 0x80000;
        }
        if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xE2B24) != 0)
        {
            *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(player) + 0xE2B24) + 0x1F8) |= 0x80000;
        }
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xBE834) != 0)
    {
        *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(player) + 0xBE834) + 0x1F8) &= 0xfff7ffff;
    }
    if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xE2B24) != 0)
    {
        *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(player) + 0xE2B24) + 0x1F8) &= 0xfff7ffff;
    }
    player->FUN_0044c5b0();
    player->FUN_0044c650();
    if (player->playerState == PLAYER_STATE_DYING)
    {
        if (player->FUN_0044cbf0() != 0)
        {
            goto updateD180;
        }
    }
    else if (player->playerState == PLAYER_STATE_SPAWNING)
    {
updateD180:
        player->FUN_0044d180();
    }
    player->FUN_0044d2c0();
    if (player->playerState != PLAYER_STATE_DYING && player->playerState != PLAYER_STATE_SPAWNING)
    {
        player->FUN_0044aec0();
    }
    g_AnmManager->ExecuteScript(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(player) + 0x10));
    player->FUN_00451150();
    player->FUN_00451500();
    player->FUN_0044d420();
    if (!g_Gui.IsDialogPresent())
    {
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE10) += 1;
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE14) += 1;
        if (g_GameManager.GaugeIsExtremelyHuman())
        {
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE1C) += 1;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE24) += 1;
            g_GameManager.AddScore(100);
        }
        else if (g_GameManager.GaugeIsExtremelyYoukai())
        {
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE18) += 1;
            *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(&g_GameManager) + 0x3DE20) += 1;
            g_GameManager.AddScore(100);
        }
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(index, slot)
// FUNCTION: th08 0x44c5b0
void Player::FUN_0044c5b0()
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsB;
    i32 index;

    for (index = 0; index < 384; index++, slot++)
    {
        if (slot->lifetime < 0)
            continue;

        slot->lifetime--;
        slot->radius += slot->radiusGrowth;
        slot->size.x += slot->sizeGrowth.x;
        slot->size.y += slot->sizeGrowth.y;

        if (slot->lifetime <= 0)
            slot->Deactivate();
    }
}

// FUNCTION: th08 0x44c650
#pragma var_order(isForced, i)
void Player::FUN_0044c650()
{
    u32 i;
    i32 isForced;
    isForced = 0;
    if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x6) != 0 &&
        this->deathbombWindowFrames == 1)
    {
        isForced = 1;
        goto acceptBomb;
    }

    if (this->bombInputLockFrames != 0)
        --this->bombInputLockFrames;

    if (this->bombState.isInUse != 0)
    {
        if (this->bombState.timer.FUN_0040d3d0())
            g_Gui.flags.pointDisplayUpdateFrames = 2;

        if (this->bombState.timer >= this->bombState.duration)
        {
            g_Spellcard.spellcard_fun_00416130();
            this->bombState.isInUse = 0;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x408) = 1.0f;
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x404) = 1.0f;

            if (this->bombState.callbackSetIndex == 4)
            {
                *reinterpret_cast<u32 *>(&g_GameManager.flags) &= 0xFFFFFE7Fu;
                for (i = 0; i < 8; i++)
                {
                    if (g_EnemyManager.bosses[i] != NULL)
                    {
                        reinterpret_cast<EclOperands::EnemyOverlay *>(
                            g_EnemyManager.bosses[i])->FUN_0042adb0(0);
                        g_EnemyManager.bosses[i]->life = 0;
                        g_EnemyManager.bosses[i]->flags1 &= ~ENEMY_FLAG_PAUSE_TIMER;
                    }
                }
                ScreenEffect::RegisterChain(SCREEN_EFFECT_UNK3, 30, 1, -1, 0, 21);
            }
        }
        else
        {
            this->bombState.calcCallbacks.callbacks[this->bombState.callbackSetIndex](this);
            this->bombState.timer++;
        }

        if (this->bombState.callbackSetIndex < 4)
        {
            if ((this->bombState.callbackSetIndex & 1) != 0)
                g_GameManager.AddToYoukaiGauge(26000 / this->bombState.duration, 1);
            else
                g_GameManager.AddToYoukaiGauge(-26000 / this->bombState.duration, 1);
        }
        return;
    }

    if ((g_GuiMessageInputCurrent & 2) != 0 && !g_GameManager.IsTampered() && !g_Gui.IsDialogPresent() &&
        this->deathbombWindowFrames != 0 &&
        g_GameManager.GetBombsRemaining() > 0 &&
        this->bombInputLockFrames == 0)
    {
        if ((((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3) != 0) ||
            (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 14) & 1) != 0))
        {
            if ((g_GuiMessageInputCurrent & 2) != 0)
            {
                if ((g_GuiMessageInputCurrent & 2) != (g_GuiMessageInputPrevious & 2))
                    g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(41), 0);
            }
            goto done;
        }

acceptBomb:
    *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 1;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x6) = 0;
    if (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3) != 0)
    {
        this->bombState.callbackSetIndex = 4;
    }
    else
    {
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x208) &= 0xFFFDFFFFu;
        if (this->deathbombEffectVm != NULL)
        {
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this->deathbombEffectVm) + 0x350) = 0;
            this->deathbombEffectVm = NULL;
        }
        *reinterpret_cast<u32 *>(&g_GameManager.flags) &= 0xFFFFFBFFu;
        g_AnmManager->SetMixColorDefault();

        this->bombState.callbackSetIndex = this->optionModeFlag;
        if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x4))
            this->bombState.callbackSetIndex = 1 - this->bombState.callbackSetIndex;

        if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x4))
        {
            this->bombState.callbackSetIndex += 2;
            if (isForced)
            {
                this->bombState.bombsConsumed = g_GameManager.GetBombsRemaining();
                g_GameManager.SetBombCount(0);
            }
            else
            {
                if (g_GameManager.GetBombsRemaining() < 2)
                {
                    this->bombState.bombsConsumed = g_GameManager.GetBombsRemaining();
                    g_GameManager.SetBombCount(0);
                }
                else
                {
                    this->bombState.bombsConsumed = 2;
                    g_GameManager.AddToBombCount(-2);
                }
            }
            ++g_PlayerDeathbombCount;
        }
        else
        {
            ++g_PlayerNormalBombCount;
            g_GameManager.AddToBombCount(-1);
        }
        g_GameManager.AddToBombsUsed(1);
    }

    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x4) = 0;
    g_Gui.flags.bombDisplayUpdateFrames = 2;
    this->bombState.isInUse = 1;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A7C) = 1;
    this->bombState.timer = 0;
    this->bombState.duration = 999;

    {
        this->bombState.calcCallbacks.callbacks[this->bombState.callbackSetIndex](this);
    }
    this->bombState.timer++;
    g_GameManager.DecreaseSubrank(200);
    g_Spellcard.FUN_0044cba0();

    this->deathbombWindowFrames += 6;
    if (this->deathbombWindowFrames > g_Player.primaryShtFile->deathbombWindowFrames)
    {
        this->deathbombWindowFrames = g_Player.primaryShtFile->deathbombWindowFrames;
    }
        goto done;
    }
    else
    {
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A7C) = 0;
    }

done:
    return;
}
// FUNCTION: th08 0x44cbf0
#pragma var_order(value, this)
i32 Player::FUN_0044cbf0()
{
    f32 value;

    if (this->deathbombWindowFrames != 0)
    {
        g_GameManager.AddTimeOrbs(-15);
        --this->deathbombWindowFrames;
        *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 4) = 1;
        if (this->deathbombWindowFrames == 0)
        {
            if (this->deathbombEffectVm != NULL)
            {
                *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this->deathbombEffectVm) + 0x350) = 0;
                this->deathbombEffectVm = NULL;
            }
            g_EffectManager.FUN_00425870(12, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 3, 1, 0xFF4040FF);
            g_EffectManager.SpawnEffect(6, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 16, -1);
            g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(15), this->position.x);
            *reinterpret_cast<u32 *>(&g_GameManager.flags) &= ~0x400u;
            g_AnmManager->SetMixColorDefault();
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x208) &= ~0x20000u;
            *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xDA) |= 4;
            g_GameManager.character = 0;
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 4) = 0;
            g_Spellcard.FUN_0044d150();
            g_GameManager.AddToDeaths(1);
            g_Gui.flags.timeDisplayUpdateFrames = 2;
            g_GameManager.AddTimeOrbs(g_GameManager.globals->currentTimeOrbs > 5000
                                           ? -500
                                           : -g_GameManager.globals->currentTimeOrbs / 10);

            if (g_GameManager.GetLives() > 0)
            {
                if (g_GameManager.GetPower() <= 16)
                    g_GameManager.SetPower(0);
                else
                    g_GameManager.AddPower(-16);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_BIG, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_SMALL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_SMALL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_SMALL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_SMALL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_SMALL, ITEM_STATE_UNK2);
                if (g_GameManager.GetBombsRemaining() > 0 &&
                    (g_GameManager.shotType == 2 || g_GameManager.shotType == 8 || g_GameManager.shotType == 9))
                    g_ItemManager.SpawnItem(&this->position, ITEM_BOMB, ITEM_STATE_UNK2);
                g_Gui.flags.powerDisplayUpdateFrames = 2;
                g_ItemManager.CancelAutoCollect();
            }
            else
            {
                g_GameManager.SetPower(0);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_FULL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_FULL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_FULL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_FULL, ITEM_STATE_UNK2);
                g_ItemManager.SpawnItem(&this->position, ITEM_POWER_FULL, ITEM_STATE_UNK2);
                g_Gui.flags.powerDisplayUpdateFrames = 2;
            }
            g_GameManager.DecreaseSubrank(1600);
        }
    }
    else
    {
        value = (f32)this->timer / 30.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2C) = 3.0f * value + 1.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x28) = 1.0f - 1.0f * value;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x200) =
        ((i32)(255.0f - (f32)this->timer * 255.0f / 30.0f) << 24) | 0xFFFFFF;
    reinterpret_cast<AnmVmBase *>(reinterpret_cast<u8 *>(this) + 0x10)->SetBlendModeAdditive();
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A9C) = 0;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2AA0) = 0;

    if ((i32)this->timer >= 30)
    {
        this->playerState = PLAYER_STATE_SPAWNING;
        this->position.operator float *()[0] = g_GameManager.arcadeRegionSize.x / 2.0f;
        this->position.operator float *()[1] = g_GameManager.arcadeRegionSize.y - 64.0f;
        this->position.operator float *()[2] = 0.2f;
        this->timer = 0;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x28) = 3.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2C) = 3.0f;
        if ((g_GameManager.shotType < 4 && *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) == 0) ||
            (g_GameManager.shotType & 1) == 0)
            (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0xC))
                ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x10), 0);
        else
            (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(this) + 0xC))
                ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(this) + 0x10), 5);

        if (g_GameManager.GetLives() <= 0)
        {
            g_GameManager.showRetryMenu = 1;
        }
        else
        {
            g_GameManager.AddLives(-1);
            g_Gui.flags.lifeDisplayUpdateFrames = 2;
            g_GameManager.SetBombCount((i32)*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_Player.primaryShtFile) + 4));
            g_Gui.flags.bombDisplayUpdateFrames = 2;
            return 1;
        }
    }
    }
    return 0;
}
// FUNCTION: th08 0x44d180
#pragma var_order(value, this)
void Player::FUN_0044d180()
{
    f32 value;

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2A70) = 60;
    value = 1.0f - (f32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AF4) / 60.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2C) = 2.0f * value + 1.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x28) = 1.0f - 1.0f * value;
    reinterpret_cast<AnmVmBase *>(reinterpret_cast<u8 *>(this) + 0x10)->SetBlendModeAdditive();
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x408) = 1.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x404) = 1.0f;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x200) =
        (((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AF4) * 0xFF) / 30 << 24) | 0xFFFFFF;
    this->deathbombWindowFrames = 0;

    if ((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AF4) >= 30)
    {
        *reinterpret_cast<i8 *>(this) = 3;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x28) = 1.0f;
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2C) = 1.0f;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0x200) = 0xFFFFFFFF;
        reinterpret_cast<AnmVmBase *>(reinterpret_cast<u8 *>(this) + 0x10)->SetBlendModeNormal();
        if (!g_GameManager.flags.isSpellPractice)
        {
            *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AF4) = 240;
        }
        this->deathbombWindowFrames = g_Player.primaryShtFile->deathbombWindowFrames;
    }
}
// FUNCTION: th08 0x44d2c0
void Player::FUN_0044d2c0()
{
    if (this->playerStateSlotCooldown != 0)
    {
        this->playerStateSlotCooldown--;
        this->FUN_0044de60(&this->position, 768.0f, 896.0f, -1, 0);
    }

    if (this->playerState == PLAYER_STATE_DEAD)
    {
        this->stateFlag = false;

        if (this->stateEffect != NULL)
            this->stateEffect->position = this->position;

        this->timer--;
        if ((i32)this->timer <= 0)
        {
            if (this->stateEffect != NULL)
            {
                this->stateEffect->active = false;
                this->stateEffect = NULL;
            }

            this->playerState = PLAYER_STATE_ALIVE;
            this->timer = 0;
            this->mainVm.color1.d3dColor = -1;
        }
        else if ((i32)this->timer % 8 < 2)
        {
            this->mainVm.color1.d3dColor = 0xfff02020;
        }
        else
        {
            this->mainVm.color1.d3dColor = -1;
        }
    }
    else
    {
        this->timer++;
    }
}

// FUNCTION: th08 0x44d420
void Player::FUN_0044d420()
{
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xE2AA4) = Float3(-999.0f, -999.0f, 0.0f);
    *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0xE2AB0) = Float3(-999.0f, -999.0f, 0.0f);
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xE2AC0) = 0;

    if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2B8) >= 400.0f)
    {
        if (g_AsciiManager.GetGaugeInterrupt() != 2)
        {
            if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2B4) < 160.0f)
            {
                g_AsciiManager.SetGaugeInterrupt(2);
                goto doneTop;
            }
        }

        if (g_AsciiManager.GetGaugeInterrupt() == 2)
        {
            if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2B4) > 160.0f)
            {
                g_AsciiManager.SetGaugeInterrupt(3);
            }
        }

doneTop:
        return;
    }

    if (g_AsciiManager.GetGaugeInterrupt() == 2)
    {
        g_AsciiManager.SetGaugeInterrupt(3);
    }
}
// FUNCTION: th08 0x44d530
#pragma var_order(i, this)
ChainCallbackResult Player::OnDrawHighPrio(Player *player)
{
    u32 i;

    player->FUN_004512f0();

    if (player->bombState.isInUse != 0)
    {
        player->bombState.drawCallbacks.callbacks[player->bombState.callbackSetIndex](player);
    }

    if (!g_GameManager.showRetryMenu)
    {
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x218) =
            g_GameManager.arcadeRegionTopLeftPos.x + *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x2B4);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x21C) =
            g_GameManager.arcadeRegionTopLeftPos.y + *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x2B8);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x220) = 0.1f;
        g_AnmManager->DrawNoRotation(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(player) + 0x10));
    }

    for (i = 0; i < 4; i++)
    {
        if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + i * 0x2F4 + 0x6FC) != 0)
        {
            reinterpret_cast<void (__fastcall *)(Player *, u8 *)>(
                *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + i * 0x2F4 + 0x6FC))(
                player, reinterpret_cast<u8 *>(player) + i * 0x2F4 + 0x40C);
        }
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x44d630
ChainCallbackResult Player::OnDrawLowPrio(Player *player)
{
    player->FUN_00451400();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x44d650
#pragma var_order(i, shotSlot, option, m, player)
ZunResult Player::AddedCallback(Player *player)
{
    u32 i;
    u8 *shotSlot;
    u8 *option;
    u32 m;

    if (IsResourceReloadEnabled())
    {
        if (Player::LoadShtFile(&player->primaryShtFile, g_Player1ShtFiles[g_GameManager.shotType]) != ZUN_SUCCESS)
            return ZUN_ERROR;
        if (Player::LoadShtFile(&player->secondaryShtFile, g_Player2ShtFile[g_GameManager.shotType]) != ZUN_SUCCESS)
            return ZUN_ERROR;
        *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(player) + 0xC) =
            g_AnmManager->PreloadAnm(5, g_PlayerAnmFilenames[g_GameManager.shotType]);
        if (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(player) + 0xC) == NULL)
            return ZUN_ERROR;
    }
    else
    {
        *reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(player) + 0xC) = g_AnmManager->GetAnm(5);
    }

    if (g_GameManager.shotType < 4 || (g_GameManager.shotType & 1) == 0)
        (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(player) + 0xC))
            ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(player) + 0x10), 0);
    else
        (*reinterpret_cast<AnmLoaded **>(reinterpret_cast<u8 *>(player) + 0xC))
            ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(reinterpret_cast<u8 *>(player) + 0x10), 5);

    player->position.operator float *()[0] = g_GameManager.arcadeRegionSize.x / 2.0f;
    player->position.operator float *()[1] = g_GameManager.arcadeRegionSize.y - 64.0f;
    player->position.operator float *()[2] = 0.49f;

    for (i = 0; i < 0x180; ++i)
        reinterpret_cast<PlayerUnkStruct0x40 *>(player->playerSlotsB)[i].Reset();

    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3D8) =
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_Player.primaryShtFile) + 0xC) / 2.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3D4) =
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3D8);
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3DC) = 5.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3E4) =
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_Player.primaryShtFile) + 0x10) / 2.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3E0) =
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3E4);
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3E8) = 5.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3F0) =
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_Player.primaryShtFile) + 0x18) / 2.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3EC) =
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3F0);
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x3F4) = 5.0f;

    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(player) + 0xE2A98) = 0;
    player->playerState = PLAYER_STATE_SPAWNING;
    player->timer = g_GameManager.IsSpellPractice() ? 10 : 120;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(player) + 2) = 1;

    shotSlot = reinterpret_cast<u8 *>(player) + 0xBE838;
    for (i = 0; (i32)i < 0x80; ++i, shotSlot += 0x484)
        *reinterpret_cast<i16 *>(shotSlot + 0x462) = 0;

    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(player) + 0xE2AC4) = -1;
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(player) + 0xE2AD0) = 0;
    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(player) + 0xE2AE8) = 0;

    player->bombState.calcCallbacks = g_PlayerBombCallbackTable[g_GameManager.shotType * 2];
    player->bombState.drawCallbacks = g_PlayerBombCallbackTable[g_GameManager.shotType * 2 + 1];

    player->bombState.isInUse = 0;
    *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(player) + 0xE2B0C) = 0xBFC90FDB;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x408) = 1.0f;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0x404) = 1.0f;
    player->deathbombWindowFrames = g_Player.primaryShtFile->deathbombWindowFrames;

    if (IsResourceReloadEnabled())
        g_AsciiManager.SetGaugeInterrupt(1);
    g_AsciiManager.FUN_00422bb0(0, 2);
    g_AsciiManager.FUN_00422bb0(1, 2);
    g_AsciiManager.FUN_00422bb0(2, 2);

    g_PlayerGaugeBounds[0] = -10000;
    g_PlayerGaugeBounds[2] = -8000;
    g_PlayerGaugeBounds[4] = -2000;
    g_PlayerGaugeBounds[1] = 10000;
    g_PlayerGaugeBounds[3] = 8000;
    g_PlayerGaugeBounds[5] = 2000;
    if (g_GameManager.shotType == 3)
    {
        g_PlayerGaugeBounds[0] = -5000;
        g_PlayerGaugeBounds[2] = -3000;
        g_PlayerGaugeBounds[4] = -2000;
    }
    else if (g_GameManager.shotType == 10)
    {
        g_PlayerGaugeBounds[0] = -5000;
        g_PlayerGaugeBounds[2] = -3000;
        g_PlayerGaugeBounds[4] = -2000;
        g_PlayerGaugeBounds[1] = 5000;
        g_PlayerGaugeBounds[3] = 3000;
        g_PlayerGaugeBounds[5] = 2000;
    }
    else if (g_GameManager.IsSoloHuman())
    {
        g_PlayerGaugeBounds[1] = 2000;
        g_PlayerGaugeBounds[3] = 8000;
        g_PlayerGaugeBounds[5] = 2001;
    }
    else if (g_GameManager.IsSoloYoukai())
    {
        g_PlayerGaugeBounds[0] = -2000;
        g_PlayerGaugeBounds[2] = -8000;
        g_PlayerGaugeBounds[4] = -2001;
    }

    *reinterpret_cast<void **>(reinterpret_cast<u8 *>(player) + 0xE2B24) = NULL;
    for (i = 0; i < 16; ++i)
        *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(player) + 0x2CC + i * 12) = player->position;
    *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(player) + 3) = 2;

    if (g_GameManager.shotType > 3)
    {
        option = reinterpret_cast<u8 *>(player) + 0x40C;
        for (m = 0; m < 4; ++m, option += 0x2F4)
        {
            memset(option, 0, 0x2F4);
            *reinterpret_cast<PlayerOptionCallback *>(option + 0x2EC) =
                g_PlayerOptionUpdateCallbacks[g_GameManager.shotType].callbacks[m];
            *reinterpret_cast<PlayerOptionCallback *>(option + 0x2F0) =
                g_PlayerOptionRenderCallbacks[g_GameManager.shotType].callbacks[m];
            if (*reinterpret_cast<void **>(option + 0x2EC) != NULL)
            {
                *reinterpret_cast<i32 *>(option + 0x2C8) = 1;
                *reinterpret_cast<ZunTimer *>(option + 0x2E0) = 0;
                *reinterpret_cast<i32 *>(option + 0x2D0) = m;
            }
            else
            {
                *reinterpret_cast<i32 *>(option + 0x2C8) = 0;
            }
        }
    }

    if (g_GameManager.IsSoloHuman())
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(player) + 0xE2B2C) = 27;
    else
        *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(player) + 0xE2B2C) = 40;
    g_EnemyManager.spawnTemplate.playerShotHitAccumulator =
        player->damageAccumulatorThreshold;
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x44dc60
ZunResult Player::DeletedCallback(Player *player)
{
    if (IsBulletManagerAnmReleaseRequired())
    {
        g_AnmManager->ReleaseAnm(5);
        g_AsciiManager.SetGaugeInterrupt(99);
        g_AsciiManager.FUN_00422bb0(0, 99);
        g_AsciiManager.FUN_00422bb0(1, 99);
        g_AsciiManager.FUN_00422bb0(2, 99);

        if (g_Player.primaryShtFile != NULL)
        {
            g_ZunMemory.Free(g_Player.primaryShtFile);
            g_Player.primaryShtFile = NULL;
        }

        if (g_Player.secondaryShtFile != NULL)
        {
            g_ZunMemory.Free(g_Player.secondaryShtFile);
            g_Player.secondaryShtFile = NULL;
        }
    }

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x44dd10
void Player::CutChain()
{
    g_Chain.Cut(g_Player.calcChain);
    g_Player.calcChain = NULL;
    g_Chain.Cut(g_Player.drawChainHighPrio);
    g_Player.drawChainHighPrio = NULL;
    g_Chain.Cut(g_Player.drawChainLowPrio);
    g_Player.drawChainLowPrio = NULL;
}

// FUNCTION: th08 0x44dd70
#pragma var_order(i, entry, header, path)
ZunResult Player::LoadShtFile(PlayerRawShtFile **header, const char *path)
{
    i32 i;
    u8 *entry;

    *header = reinterpret_cast<PlayerRawShtFile *>(FileSystem::OpenFile(path, NULL, 0));
    if (*header == NULL)
    {
        return ZUN_ERROR;
    }

    for (i = 0; i < *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(*header) + 0x2); i++)
    {
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(*header) + i * 8 + 0x38) +=
            reinterpret_cast<u32>(*header);
        entry = *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(*header) + i * 8 + 0x38);

        while (*reinterpret_cast<i16 *>(entry) >= 0)
        {
            *reinterpret_cast<PlayerShotUpdateCallback *>(entry + 0x28) =
                g_PlayerShotUpdateCallbacks[*reinterpret_cast<u32 *>(entry + 0x28)];
            *reinterpret_cast<PlayerShotRenderCallback *>(entry + 0x2C) =
                g_PlayerShotRenderCallbacks[*reinterpret_cast<u32 *>(entry + 0x2C)];
            *reinterpret_cast<PlayerShotRenderCallback *>(entry + 0x30) =
                g_PlayerShotTimerCallbacks[*reinterpret_cast<u32 *>(entry + 0x30)];

            *reinterpret_cast<PlayerShotCollisionOrDifficultyEntry *>(entry + 0x34) =
                g_PlayerShotCollisionOrDifficultyTable[*reinterpret_cast<u32 *>(entry + 0x34)];
            entry += 0x38;
        }
    }

    return ZUN_SUCCESS;
}

#pragma var_order(slot, index)
// FUNCTION: th08 0x44de60
PlayerUnkStruct0x40 *Player::FUN_0044de60(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsC;
    i32 index;

    for (index = 0; index < 191; index++, slot++)
    {
        if (!slot->active)
            break;
    }

    slot->Reset();
    slot->active = true;
    slot->center.x = center->x;
    slot->center.y = center->y;
    slot->size.x = value1;
    slot->size.y = value2;
    slot->lifetime = value4;
    slot->collisionValue = value3;

    return slot;
}

#pragma var_order(slot, index)
// FUNCTION: th08 0x44df00
PlayerUnkStruct0x40 *Player::FUN_0044df00(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsC;
    i32 index;

    for (index = 0; index < 191; index++, slot++)
    {
        if (!slot->active)
            break;
    }

    slot->Reset();
    slot->active = true;
    slot->center.x = center->x;
    slot->center.y = center->y;
    slot->radius = value1;
    slot->radiusGrowth = value2;
    slot->lifetime = value3;
    slot->collisionValue = value4;

    return slot;
}

#pragma var_order(slot, index)
// FUNCTION: th08 0x44dfa0
PlayerUnkStruct0x40 *Player::FUN_0044dfa0(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsB;
    i32 index;

    for (index = 0; index < 191; index++, slot++)
    {
        if (!slot->active)
            break;
    }

    slot->Reset();
    slot->active = true;
    slot->center.x = center->x;
    slot->center.y = center->y;
    slot->size.x = value1;
    slot->size.y = value2;
    slot->lifetime = value4;
    slot->damage = value3;

    return slot;
}

#pragma var_order(slot, index)
// FUNCTION: th08 0x44e040
PlayerUnkStruct0x40 *Player::FUN_0044e040(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerUnkStruct0x40 *slot = this->playerSlotsB;
    i32 index;

    for (index = 0; index < 191; index++, slot++)
    {
        if (!slot->active)
            break;
    }

    slot->Reset();
    slot->active = true;
    slot->center.x = center->x;
    slot->center.y = center->y;
    slot->radius = value1;
    slot->radiusGrowth = value2;
    slot->lifetime = value4;
    slot->damage = value3;

    return slot;
}

// FUNCTION: th08 0x44e0e0
ZunBool IsResourceReloadDisabled()
{
    return !IsResourceReloadEnabled();
}

/* The target emits these cross-subsystem definitions in the Player translation unit. */
// FUNCTION: th08 0x44e0f0
void AnmVmBase::SetBlendModeAdditive()
{
    this->blendMode = AnmBlendMode_Additive;
}

// FUNCTION: th08 0x44e120
void AnmVmBase::SetBlendModeNormal()
{
    this->blendMode = AnmBlendMode_Normal;
}

// FUNCTION: th08 0x44e140
void GameManager::SetYoukaiGauge(u16 value)
{
    this->globals->youkaiGauge = value;
}

// FUNCTION: th08 0x44e160
void GameManager::RandomizeAntiTamper()
{
    this->globals->rng1[0] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    this->globals->rng1[1] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    this->globals->rng1[2] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    this->globals->rng1[3] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    this->globals->rng1[4] = g_Rng.GetRandomU32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    this->globals->rng4[0] = g_Rng.GetRandomF32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    this->globals->rng4[1] = g_Rng.GetRandomF32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
    this->globals->rng4[2] = g_Rng.GetRandomF32InRange(ANTITAMPER_RNG_RANGE) + ANTITAMPER_RNG_ADD;
}

// FUNCTION: th08 0x44e260
void GameManager::AddToDeaths(i32 amount)
{
    if (this->IsTampered())
    {
        CRASH_GAME();
    }
    this->globals->deaths += (f32)amount;
    this->globals->deathInStage += (f32)amount;
    *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0x3DA9C) += 1;
    this->UpdateAntiTamper();
}

// FUNCTION: th08 0x44e2e0
void GameManager::AddToBombsUsed(i32 amount)
{
    if (this->IsTampered())
    {
        CRASH_GAME();
    }
    this->globals->bombsUsed += (f32)amount;
    this->globals->bombsUsedInStage += (f32)amount;
    this->UpdateAntiTamper();
}

// FUNCTION: th08 0x44e350
void PlayerUnkStruct0x40::Deactivate()
{
    this->active = false;
}

// FUNCTION: th08 0x44e370
void PlayerUnkStruct0x40::Reset()
{
    memset(this, 0, sizeof(*this));
    this->collisionInterval = 1;
}

void __fastcall PlayerOptionHomingToPlayer(Player *player, u8 *option);
void __fastcall PlayerOptionHomingToTarget(Player *player, u8 *option);

// FUNCTION: th08 0x44e3a0
i32 __fastcall FUN_0044e3a0(Player *player, PlayerOptionState *option)
{
    switch (option->state2C8)
    {
    case 1:
        player->anmFile->SetAndExecuteScriptIdx(&option->vm, 18);
        option->position = player->position;
        option->position.y -= 96.0f;
        if (option->position.y < 32.0f)
            option->position.y = 32.0f;
        option->state2C8 = 2;
        player->optionHomingTarget = NULL;
        break;

    case 2:
        switch (option->substate2CC)
        {
        case 0:
            PlayerOptionHomingToPlayer(player, reinterpret_cast<u8 *>(option));
            if (option->velocity.x < 0.0f)
            {
                option->vm.SetInterrupt(2);
                option->substate2CC = 1;
                if (option->vm.scale.x < 0.0f)
                    option->vm.scale.x = -option->vm.scale.x;
            }
            else if (option->velocity.x > 0.0f)
            {
                option->vm.SetInterrupt(2);
                option->substate2CC = 2;
                if (option->vm.scale.x > 0.0f)
                    option->vm.scale.x = -option->vm.scale.x;
            }
            break;

        case 1:
            PlayerOptionHomingToPlayer(player, reinterpret_cast<u8 *>(option));
            if (option->velocity.x == 0.0f)
            {
                option->vm.SetInterrupt(1);
                option->substate2CC = 0;
                if (option->vm.scale.x < 0.0f)
                    option->vm.scale.x = -option->vm.scale.x;
            }
            else if (option->velocity.x > 0.0f)
            {
                option->vm.SetInterrupt(2);
                option->substate2CC = 2;
                if (option->vm.scale.x > 0.0f)
                    option->vm.scale.x = -option->vm.scale.x;
            }
            break;

        case 2:
            PlayerOptionHomingToPlayer(player, reinterpret_cast<u8 *>(option));
            if (option->velocity.x == 0.0f)
            {
                option->vm.SetInterrupt(1);
                option->substate2CC = 0;
                if (option->vm.scale.x < 0.0f)
                    option->vm.scale.x = -option->vm.scale.x;
            }
            else if (option->velocity.x < 0.0f)
            {
                option->vm.SetInterrupt(2);
                option->substate2CC = 1;
                if (option->vm.scale.x < 0.0f)
                    option->vm.scale.x = -option->vm.scale.x;
            }
            break;

        case 3:
            if (player->optionHomingTarget != NULL)
                PlayerOptionHomingToTarget(player, reinterpret_cast<u8 *>(option));
            if (((player->timerE2AC4 < 0) && ((g_CurFrameInput & 1) == 0)) ||
                player->optionHomingTarget == NULL)
            {
                player->optionHomingTarget = NULL;
                option->vm.SetInterrupt(1);
                option->substate2CC = 0;
            }
            break;

        default:
            break;
        }
        break;

    case 3:
        if (option->timer == 0)
            option->vm.SetInterrupt(5);
        if (option->timer > 16)
        {
            option->state2C8 = 0;
            option->updateCallback = NULL;
            option->renderCallback = NULL;
        }
        break;
    }
    return 0;
}

// FUNCTION: th08 0x44e770
#pragma var_order(delta, target, player, option)
void __fastcall PlayerOptionHomingToPlayer(Player *player, u8 *option)
{
    Float3 target;
    Float3 delta;

    target = player->position;
    target.y -= 96.0f;
    if (target.y < 32.0f)
        target.y = 32.0f;

    delta = target - *reinterpret_cast<Float3 *>(option + 0x2A4);
    delta /= 16.0f;
    *reinterpret_cast<Float3 *>(option + 0x2BC) +=
        (delta - *reinterpret_cast<Float3 *>(option + 0x2BC)) * 0.2f;
    *reinterpret_cast<Float3 *>(option + 0x2A4) += *reinterpret_cast<Float3 *>(option + 0x2BC);

    if (fabsf(*reinterpret_cast<f32 *>(option + 0x2BC)) < 0.05f)
        *reinterpret_cast<f32 *>(option + 0x2BC) = 0.0f;

    if (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(player) + 0xE2AC4) >= 0 &&
        *reinterpret_cast<void **>(reinterpret_cast<u8 *>(player) + 0xE2ABC) != NULL &&
        *reinterpret_cast<ZunTimer *>(option + 0x2E0) >= 10)
    {
        reinterpret_cast<AnmVmBase *>(option)->SetInterrupt(3);
        *reinterpret_cast<i32 *>(option + 0x2CC) = 3;
    }
    else
    {
        *reinterpret_cast<void **>(reinterpret_cast<u8 *>(player) + 0xE2ABC) = NULL;
    }
}

// FUNCTION: th08 0x44e8d0
#pragma var_order(delta, target, player, option)
void __fastcall PlayerOptionHomingToTarget(Player *player, u8 *option)
{
    Float3 target;
    Float3 delta;

    target = reinterpret_cast<Enemy *>(
        *reinterpret_cast<void **>(reinterpret_cast<u8 *>(player) + 0xE2ABC))->worldPosition;
    target.y += 32.0f;
    if (target.y < 32.0f)
        target.y = 32.0f;

    delta = target - *reinterpret_cast<Float3 *>(option + 0x2A4);
    delta /= 16.0f;
    *reinterpret_cast<Float3 *>(option + 0x2BC) +=
        (delta - *reinterpret_cast<Float3 *>(option + 0x2BC)) * 0.2f;
    *reinterpret_cast<Float3 *>(option + 0x2A4) += *reinterpret_cast<Float3 *>(option + 0x2BC);

    if (fabsf(*reinterpret_cast<f32 *>(option + 0x2BC)) < 0.05f)
        *reinterpret_cast<f32 *>(option + 0x2BC) = 0.0f;
}

// FUNCTION: th08 0x44e9e0
i32 __fastcall PlayerRoute2OptionRender(Player *, u8 *option)
{
    reinterpret_cast<AnmVm *>(option)->pos.x =
        g_GameManager.arcadeRegionTopLeftPos.x + *reinterpret_cast<f32 *>(option + 0x2A4);
    reinterpret_cast<AnmVm *>(option)->pos.y =
        g_GameManager.arcadeRegionTopLeftPos.y + *reinterpret_cast<f32 *>(option + 0x2A8);
    reinterpret_cast<AnmVm *>(option)->pos.z = 0.49f;
    g_AnmManager->Draw2D(reinterpret_cast<AnmVm *>(option));
    return 0;
}
// FUNCTION: th08 0x44ea40
i32 __fastcall FUN_0044ea40(Player *player, PlayerOptionState *option)
{
    switch (option->state2C8)
    {
    case 1:
        player->anmFile->SetAndExecuteScriptIdx(&option->vm, 29);
        option->state2C8 = 2;
        // Fall through: the option starts following immediately.
    case 2:
        if (player->bombState.isInUse == 0)
        {
            option->position = player->position;
            option->position.y -= 32.0f;
        }
        break;

    case 3:
        option->position = player->position;
        option->position.y -= 32.0f;
        if (option->timer == 0)
            option->vm.SetInterrupt(5);
        if (option->timer > 16)
        {
            option->state2C8 = 0;
            option->updateCallback = NULL;
            option->renderCallback = NULL;
        }
        break;
    }
    return 0;
}

// FUNCTION: th08 0x44eb70
i32 __fastcall FUN_0044eb70(Player *player, PlayerOptionState *option)
{
    switch (option->state2C8)
    {
    case 1:
        player->anmFile->SetAndExecuteScriptIdx(&option->vm, 24);
        option->state2C8 = 2;
        option->target = player->position;
        switch (option->optionIndex)
        {
        case 0:
            option->target.x -= 30.0f;
            option->target.y -= 16.0f;
            option->orbitAngle = 0.0f;
            break;
        case 1:
            option->target.x -= 10.0f;
            option->target.y -= 32.0f;
            option->orbitAngle = ZUN_PI;
            break;
        case 2:
            option->target.x += 10.0f;
            option->target.y -= 32.0f;
            option->orbitAngle = 0.0f;
            break;
        case 3:
            option->target.x += 30.0f;
            option->target.y -= 16.0f;
            option->orbitAngle = ZUN_PI;
            break;
        default:
            break;
        }
        // Fall through to update the orbit immediately.
    case 2:
        if (option->timer > 12)
        {
            switch (option->optionIndex)
            {
            case 0:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.02617993950843811f);
                break;
            case 1:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, -0.03490658476948738f);
                break;
            case 2:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.03490658476948738f);
                break;
            case 3:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, -0.02617993950843811f);
                break;
            default:
                break;
            }
        }
        option->position.FromAngleMagnitude(option->orbitAngle, 8.0f);
        option->position += option->target;
        break;

    case 3:
        if (option->timer == 0)
            option->vm.SetInterrupt(5);
        if (option->timer > 16)
        {
            option->state2C8 = 0;
            option->updateCallback = NULL;
            option->renderCallback = NULL;
        }
        break;
    }
    return 0;
}

// FUNCTION: th08 0x0044ee70
i32 __fastcall FUN_0044ee70(Player *player, PlayerOptionState *option)
{
    Float3 desired;

    switch (option->state2C8)
    {
    case 1:
        player->anmFile->SetAndExecuteScriptIdx(&option->vm, 24);
        option->state2C8 = 2;
        option->target = player->position;
        switch (option->optionIndex)
        {
        case 0:
            option->target.x -= 30.0f;
            option->target.y -= 16.0f;
            option->orbitAngle = 0.0f;
            break;
        case 1:
            option->target.x -= 10.0f;
            option->target.y -= 32.0f;
            option->orbitAngle = ZUN_PI;
            break;
        case 2:
            option->target.x += 10.0f;
            option->target.y -= 32.0f;
            option->orbitAngle = 0.0f;
            break;
        case 3:
            option->target.x += 30.0f;
            option->target.y -= 16.0f;
            option->orbitAngle = ZUN_PI;
            break;
        default:
            break;
        }
        // Fall through into the orbit update.
    case 2:
        if (option->timer > 12)
        {
            switch (option->optionIndex)
            {
            case 0:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.02617993950843811f);
                break;
            case 1:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, -0.03490658476948738f);
                break;
            case 2:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.03490658476948738f);
                break;
            case 3:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, -0.02617993950843811f);
                break;
            default:
                break;
            }
        }

        option->vm.color1.d3dColor = 0xFFFF8080;
        option->position.FromAngleMagnitude(option->orbitAngle, 8.0f);
        if (player->optionModeFlag == 0)
        {
            desired = player->position;
            option->vm.color1.d3dColor = 0xFF80FFFF;
            switch (option->optionIndex)
            {
            case 0:
                desired.x -= 30.0f;
                desired.y -= 16.0f;
                break;
            case 1:
                desired.x -= 10.0f;
                desired.y -= 32.0f;
                break;
            case 2:
                desired.x += 10.0f;
                desired.y -= 32.0f;
                break;
            case 3:
                desired.x += 30.0f;
                desired.y -= 16.0f;
                break;
            default:
                break;
            }
            option->target = (desired - option->target) * 0.2f + option->target;
        }
        option->position += option->target;
        option->position.z = 0.0f;
        g_EffectManager.SpawnEffect(
            47, reinterpret_cast<D3DXVECTOR3 *>(&option->position), 1, 0x80602050);
        break;

    case 3:
        if (option->timer == 0)
            option->vm.SetInterrupt(5);
        if (option->timer > 16)
        {
            option->state2C8 = 0;
            option->updateCallback = NULL;
            option->renderCallback = NULL;
        }
        break;
    }
    return 0;
}

// FUNCTION: th08 0x0044f2d0
#pragma var_order(angleDifference, targetAngle)
i32 __fastcall FUN_0044f2d0(Player *player, PlayerOptionState *option)
{
    f32 targetAngle;
    f32 angleDifference;

    switch (option->state2C8)
    {
    case 1:
        player->anmFile->SetAndExecuteScriptIdx(&option->vm, 21);
        option->state2C8 = 2;
        option->target = player->vectors2CC[15];
        option->orbitAngle = 0.0f;
        option->facingAngle = -ZUN_PI / 2.0f;
        // Fall through into the normal update.
    case 2:
        option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.052359879016876221f);
        option->position.FromAngleMagnitude(option->orbitAngle, 8.0f);
        option->target = (player->vectors2CC[15] - option->target) * 0.05f + option->target;
        option->position += option->target;
        option->position.z = 0.0f;
        g_EffectManager.SpawnEffect(
            47, reinterpret_cast<D3DXVECTOR3 *>(&option->position), 1, 0x80405080);

        switch (player->movementDirection)
        {
        case 0:
            goto optionUpdateDone;
        case 1:
            targetAngle = ZUN_PI / 2.0f;
            break;
        case 2:
            targetAngle = -ZUN_PI / 2.0f;
            break;
        case 3:
            targetAngle = 0.0f;
            break;
        case 4:
            targetAngle = ZUN_PI;
            break;
        case 5:
            targetAngle = ZUN_PI / 4.0f;
            break;
        case 6:
            targetAngle = 3.0f * ZUN_PI / 4.0f;
            break;
        case 7:
            targetAngle = -ZUN_PI / 4.0f;
            break;
        case 8:
            targetAngle = -3.0f * ZUN_PI / 4.0f;
            break;
        default:
            break;
        }

        angleDifference = fabsf(option->facingAngle - targetAngle);
        if (angleDifference > ZUN_PI)
        {
            targetAngle += option->facingAngle > targetAngle ? ZUN_2PI : -ZUN_2PI;
            angleDifference = fabsf(option->facingAngle - targetAngle);
        }
        if (angleDifference > ZUN_PI / 2.0f)
        {
            option->facingAngle = targetAngle;
        }
        else
        {
            option->facingAngle = AddNormalizeAngle(
                (targetAngle - option->facingAngle) * 0.07f, option->facingAngle);
        }
optionUpdateDone:
        break;

    case 3:
        if (option->timer == 0)
            option->vm.SetInterrupt(5);
        if (option->timer > 16)
        {
            option->state2C8 = 0;
            option->updateCallback = NULL;
            option->renderCallback = NULL;
        }
        break;
    }
    return 0;
}

// FUNCTION: th08 0x0044f5e0
#pragma var_order(angleDifference, targetAngle)
i32 __fastcall FUN_0044f5e0(Player *player, PlayerOptionState *option)
{
    f32 targetAngle;
    f32 angleDifference;

    switch (option->state2C8)
    {
    case 1:
        player->anmFile->SetAndExecuteScriptIdx(&option->vm, 21);
        option->state2C8 = 2;
        option->target = player->vectors2CC[15];
        option->orbitAngle = 0.0f;
        option->facingAngle = -ZUN_PI / 2.0f;
        // Fall through into the normal update.
    case 2:
        option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.052359879016876221f);
        option->position.FromAngleMagnitude(option->orbitAngle, 8.0f);
        option->target = (player->vectors2CC[15] - option->target) * 0.05f + option->target;
        option->position += option->target;
        option->position.z = 0.0f;
        option->vm.color1.d3dColor = 0xFFFF8080;

        if (player->optionModeFlag == 0)
        {
            option->vm.color1.d3dColor = 0xFFFFFFFF;
            switch (player->movementDirection)
            {
            case 0:
                goto optionUpdateDone;
            case 1:
                targetAngle = ZUN_PI / 2.0f;
                break;
            case 2:
                targetAngle = -ZUN_PI / 2.0f;
                break;
            case 3:
                targetAngle = 0.0f;
                break;
            case 4:
                targetAngle = ZUN_PI;
                break;
            case 5:
                targetAngle = ZUN_PI / 4.0f;
                break;
            case 6:
                targetAngle = 3.0f * ZUN_PI / 4.0f;
                break;
            case 7:
                targetAngle = -ZUN_PI / 4.0f;
                break;
            case 8:
                targetAngle = -3.0f * ZUN_PI / 4.0f;
                break;
            default:
                break;
            }

            angleDifference = fabsf(option->facingAngle - targetAngle);
            if (angleDifference > ZUN_PI)
            {
                targetAngle += option->facingAngle > targetAngle ? ZUN_2PI : -ZUN_2PI;
                angleDifference = fabsf(option->facingAngle - targetAngle);
            }
            if (angleDifference > ZUN_PI / 2.0f)
            {
                option->facingAngle = targetAngle;
            }
            else
            {
                option->facingAngle = AddNormalizeAngle(
                    (targetAngle - option->facingAngle) * 0.07f, option->facingAngle);
            }

            g_EffectManager.SpawnEffect(
                47, reinterpret_cast<D3DXVECTOR3 *>(&option->position), 1, 0x80405080);
        }
        else
        {
            g_EffectManager.SpawnEffect(
                47, reinterpret_cast<D3DXVECTOR3 *>(&option->position), 1, 0xFFF05080);
        }
optionUpdateDone:
        break;

    case 3:
        if (option->timer == 0)
            option->vm.SetInterrupt(5);
        if (option->timer > 16)
        {
            option->state2C8 = 0;
            option->updateCallback = NULL;
            option->renderCallback = NULL;
        }
        break;
    }
    return 0;
}

// FUNCTION: th08 0x0044f930
i32 __fastcall FUN_0044f930(Player *player, PlayerOptionState *option)
{
    Float3 base = player->position;

    switch (option->state2C8)
    {
    case 1:
        player->anmFile->SetAndExecuteScriptIdx(&option->vm, 21);
        option->state2C8 = 2;
        option->target = base;
        switch (option->optionIndex)
        {
        case 0:
            option->orbitAngle = 0.0f;
            break;
        case 1:
            option->orbitAngle = -ZUN_PI;
            break;
        default:
            break;
        }
        // Fall through.
    case 2:
        switch (option->optionIndex)
        {
        case 0:
            base.x -= 32.0f;
            option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.052359879016876221f);
            break;
        case 1:
            base.x += 32.0f;
            option->orbitAngle = AddNormalizeAngle(option->orbitAngle, -0.052359879016876221f);
            break;
        default:
            break;
        }

        option->position.FromAngleMagnitude(option->orbitAngle, 6.0f);
        option->target = (base - option->target) * 0.09f + option->target;
        option->position += option->target;
        option->position.z = 0.0f;
        g_EffectManager.SpawnEffect(
            47, reinterpret_cast<D3DXVECTOR3 *>(&option->position), 1, 0x80602050);
        break;

    case 3:
        if (option->timer == 0)
            option->vm.SetInterrupt(5);
        if (option->timer > 16)
        {
            option->state2C8 = 0;
            option->updateCallback = NULL;
            option->renderCallback = NULL;
        }
        break;
    }
    return 0;
}

// FUNCTION: th08 0x44fb70
void __fastcall Player::FUN_0044fb70(u8 *slot, u8 *entry)
{
    if (*reinterpret_cast<i16 *>(entry + 0x20) == 0)
    {
        *reinterpret_cast<Float3 *>(slot + 0x2A4) =
            *reinterpret_cast<Float3 *>(reinterpret_cast<u8 *>(this) + 0x2B4);
    }
    else
    {
        *reinterpret_cast<Float3 *>(slot + 0x2A4) = *reinterpret_cast<Float3 *>(
            reinterpret_cast<u8 *>(this) + ((*reinterpret_cast<i16 *>(entry + 0x20) - 1) * 0x2F4) + 0x6B0);
    }

    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[0] += *reinterpret_cast<f32 *>(entry + 0x4);
    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[1] += *reinterpret_cast<f32 *>(entry + 0x8);
    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[2] = 0.495f;

    *reinterpret_cast<u32 *>(slot + 0x430) = *reinterpret_cast<u32 *>(entry + 0x0C);
    *reinterpret_cast<u32 *>(slot + 0x434) = *reinterpret_cast<u32 *>(entry + 0x10);
    *reinterpret_cast<f32 *>(slot + 0x438) = 1.0f;
    *reinterpret_cast<u32 *>(slot + 0x450) = *reinterpret_cast<u32 *>(entry + 0x14);
    *reinterpret_cast<u32 *>(slot + 0x44C) = *reinterpret_cast<u32 *>(entry + 0x18);
    *reinterpret_cast<f32 *>(slot + 0x43C) = cosf(*reinterpret_cast<f32 *>(entry + 0x14)) * *reinterpret_cast<f32 *>(entry + 0x18);
    *reinterpret_cast<f32 *>(slot + 0x440) = sinf(*reinterpret_cast<f32 *>(entry + 0x14)) * *reinterpret_cast<f32 *>(entry + 0x18);

    *reinterpret_cast<ZunTimer *>(slot + 0x454) = 0;
    *reinterpret_cast<u8 *>(slot + 0x46C) = *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0x3);
    *reinterpret_cast<i16 *>(slot + 0x464) = *reinterpret_cast<i16 *>(entry + 0x22);
    *reinterpret_cast<i16 *>(slot + 0x460) = *reinterpret_cast<i16 *>(entry + 0x1C);
    *reinterpret_cast<i16 *>(slot + 0x46E) = *reinterpret_cast<i16 *>(entry + 0x24);

    if (*reinterpret_cast<i16 *>(entry + 0x26) >= 0)
    {
        g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(*reinterpret_cast<i16 *>(entry + 0x26)),
                                               *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0x2B4));
    }

    reinterpret_cast<AnmLoaded *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xC))
        ->SetAndExecuteScriptIdx(reinterpret_cast<AnmVm *>(slot), *reinterpret_cast<i16 *>(entry + 0x24) + 10);

    *reinterpret_cast<u8 *>(slot + 0x470) = 0;
    if (g_GameManager.GaugeIsExtremelyYoukai())
    {
        if (*reinterpret_cast<i16 *>(entry + 0x1E) > 0)
        {
            *reinterpret_cast<u8 *>(slot + 0x470) = 1;
        }
    }
}

// FUNCTION: th08 0x44fd80
#pragma var_order(slot, this)
i32 __fastcall Player::FUN_0044fd80(u8 *slot, i32 value, u8 *entry)
{
    if (value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        this->FUN_0044fb70(slot, entry);
        return 1;
    }

    return 0;
}


// FUNCTION: th08 0x44fdd0
#pragma var_order(slot, this)
i32 __fastcall Player::FUN_0044fdd0(u8 *slot, i32 value, u8 *entry)
{
    if (this->bombState.isInUse == 0 &&
        value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        this->FUN_0044fb70(slot, entry);
        return 1;
    }

    return 0;
}

// FUNCTION: th08 0x44fe20
#pragma var_order(index, i, this, slot)
i32 __fastcall Player::FUN_0044fe20(u8 *slot, i32 value, u8 *entry)
{
    i32 index;
    i32 i;

    index = *reinterpret_cast<i16 *>(entry + 2);
    if (this->bombState.isInUse != 0)
    {
        return 0;
    }
    if (g_GameManager.flags.unk13)
    {
        return 0;
    }

    if (*reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xE2A44 + (index << 4)) != 0)
    {
        if (*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A80 + index * 4) != entry)
        {
            *reinterpret_cast<i16 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A44 + (index << 4)) + 0x1FE) = 1;
            *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(this) + 0xE2A44 + (index << 4)) = 0;
        }
        return 0;
    }

    *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2A38 + (index << 4)) = 999;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A44 + (index << 4)) = slot;
    *reinterpret_cast<i16 *>(slot + 0x466) = index;
    *reinterpret_cast<i16 *>(slot + 0x468) = *reinterpret_cast<i16 *>(entry + 0x20);
    *reinterpret_cast<u32 *>(slot + 0x444) = *reinterpret_cast<u32 *>(entry + 4);
    *reinterpret_cast<u32 *>(slot + 0x448) = *reinterpret_cast<u32 *>(entry + 8);
    *reinterpret_cast<i16 *>(slot + 0x46A) = *reinterpret_cast<i16 *>(entry);
    this->FUN_0044fb70(slot, entry);

    for (i = 31; i >= 0; i--)
    {
        *reinterpret_cast<u32 *>(slot + i * 0xC + 0x2B0) = 0xC479C000;
    }
    *reinterpret_cast<u32 *>(slot + 0x2A4) = 0xC479C000;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A80 + index * 4) = entry;

    return 1;
}

// FUNCTION: th08 0x44ffa0
#pragma var_order(magnitude, angle, this, slot)
i32 __fastcall Player::FUN_0044ffa0(u8 *slot, i32 value, u8 *entry)
{
    f32 angle;
    f32 magnitude;

    if (value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        this->FUN_0044fb70(slot, entry);
        if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2AB0) > -100.0f)
        {
            angle = AddNormalizeAngle(
                VectorAngle(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2AB4) - *reinterpret_cast<f32 *>(slot + 0x2A8),
                            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(this) + 0xE2AB0) - *reinterpret_cast<f32 *>(slot + 0x2A4)),
                *reinterpret_cast<f32 *>(entry + 0x14) + ZUN_PI / 2.0f);
            magnitude = *reinterpret_cast<f32 *>(entry + 0x18) * 1.5f;
            reinterpret_cast<Float3 *>(slot + 0x43C)->FromAngleMagnitude(angle, magnitude);
            *reinterpret_cast<u32 *>(slot + 0x450) = *reinterpret_cast<u32 *>(&angle);
        }
        return 1;
    }

    return 0;
}

// FUNCTION: th08 0x00450080
#pragma var_order(magnitude, angle)
i32 __fastcall FUN_00450080(Player *player, PlayerShot *slot, i32 value, u8 *entry)
{
    f32 angle;
    f32 magnitude;

    if (value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        player->FUN_0044fb70(reinterpret_cast<u8 *>(slot), entry);
        angle = AddNormalizeAngle(
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(player) + 0xE2B0C),
            *reinterpret_cast<f32 *>(entry + 0x14) + ZUN_PI / 2.0f);
        magnitude = *reinterpret_cast<f32 *>(entry + 0x18);
        reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, magnitude);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x450) = angle;
        return 1;
    }
    return 0;
}

// FUNCTION: th08 0x00450110
#pragma var_order(magnitude, angle)
i32 __fastcall FUN_00450110(Player *player, PlayerShot *slot, i32 value, u8 *entry)
{
    f32 angle;
    f32 magnitude;

    if (player->bombState.isInUse == 0 &&
        value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        player->FUN_0044fb70(reinterpret_cast<u8 *>(slot), entry);
        angle = AddNormalizeAngle(player->optionStates[2].facingAngle, *reinterpret_cast<f32 *>(entry + 0x14));
        magnitude = *reinterpret_cast<f32 *>(entry + 0x18);
        reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, magnitude);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x450) = angle;
        return 1;
    }
    return 0;
}

// FUNCTION: th08 0x004501b0
i32 __fastcall FUN_004501b0(Player *player, PlayerShot *slot, i32 value, u8 *entry)
{
    if (value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        player->FUN_0044fb70(reinterpret_cast<u8 *>(slot), entry);
        *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x450) =
            g_Rng.GetRandomF32() * ZUN_PI / 48.0f - ZUN_PI / 2.0f;
        reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x450),
            *reinterpret_cast<f32 *>(entry + 0x18));
        return 1;
    }
    return 0;
}

// FUNCTION: th08 0x00450240
#pragma var_order(magnitude, angle)
i32 __fastcall FUN_00450240(Player *player, PlayerShot *slot, i32 value, u8 *entry)
{
    f32 angle;
    f32 magnitude;

    if (value % *reinterpret_cast<i16 *>(entry) == *reinterpret_cast<i16 *>(entry + 2))
    {
        player->FUN_0044fb70(reinterpret_cast<u8 *>(slot), entry);
        if (player->optionHomingTarget != NULL)
        {
            angle = AddNormalizeAngle(
                VectorAngle(player->optionHomingTarget->position.y - slot->position.y,
                            player->optionHomingTarget->position.x - slot->position.x),
                *reinterpret_cast<f32 *>(entry + 0x14) + ZUN_PI / 2.0f);
            magnitude = *reinterpret_cast<f32 *>(entry + 0x18) * 1.5f;
            reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, magnitude);
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x450) = angle;
        }
        return 1;
    }
    return 0;
}

// FUNCTION: th08 0x00450320
#pragma var_order(yDelta, xDelta, magnitude)
i32 __fastcall FUN_00450320(Player *player, PlayerShot *slot)
{
    f32 xDelta;
    f32 yDelta;
    f32 magnitude;
    if (slot->state == 1)
    {
        if (player->tailPosition0.x > -100.0f && (i32)slot->timer < 40 && slot->timer.FUN_0040d3d0())
        {
            xDelta = player->tailPosition0.x - slot->position.operator float *()[0];
            yDelta = player->tailPosition0.y - slot->position.operator float *()[1];
            magnitude = sqrtf(xDelta * xDelta + yDelta * yDelta) / (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) / 4.0f);
            if (magnitude < 1.0f) magnitude = 1.0f;
            xDelta = xDelta / magnitude + slot->velocity.x;
            yDelta = yDelta / magnitude + slot->velocity.y;
            magnitude = sqrtf(xDelta * xDelta + yDelta * yDelta);
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) = magnitude > 10.0f ? 10.0f : magnitude;
            if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) < 1.0f) *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) = 1.0f;
            slot->velocity.x = xDelta * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) / magnitude;
            slot->velocity.y = yDelta * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) / magnitude;
        }
        else if (*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) < 10.0f)
        {
            *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) += 1.0f / 3.0f;
            xDelta = slot->velocity.x;
            yDelta = slot->velocity.y;
            magnitude = sqrtf(xDelta * xDelta + yDelta * yDelta);
            slot->velocity.x = xDelta * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) / magnitude;
            slot->velocity.y = yDelta * *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x44C) / magnitude;
        }
    }
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x450) = VectorAngle(slot->velocity.y, slot->velocity.x);
    return 0;
}

// FUNCTION: th08 0x00450580
i32 __fastcall FUN_00450580(Player *player, PlayerShot *slot)
{
    if (slot->state == 1)
        slot->velocity.y -= g_Rng.GetRandomF32InRange(0.1f) + 0.27f;
    return 0;
}

// FUNCTION: th08 0x004505d0
i32 __fastcall FUN_004505d0(Player *player, PlayerShot *slot)
{
    if (player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].instruction !=
        reinterpret_cast<EclTimelineInstruction *>(slot))
    {
        if (slot->vm.FUN_004396f8()) slot->vm.pendingInterrupt = 1;
    }
    if (g_Gui.IsDialogPresent() || player->bombState.isInUse != 0 || g_GameManager.flags.unk13)
    {
        if ((i32)player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer > 20)
            player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer = 20;
    }
    if (player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer <= 0)
    {
        player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer = 0;
        player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].instruction = NULL;
        slot->state = 0;
        return 1;
    }
    if (player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer <= 70)
    {
        if (slot->vm.FUN_004396f8()) slot->vm.pendingInterrupt = 1;
    }
    slot->position.x += *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x444);
    slot->position.z = 0.44f;
    if (player->playerState == PLAYER_STATE_DYING) return 1;
    *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(slot) + 0x1C) = slot->position.y / 14.0f;
    slot->hitboxSize.y = slot->position.y;
    slot->position.y /= 2.0f;
    if (player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer < 100)
        player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].timer--;
    if (g_GameManager.GaugeIsExtremelyYoukai())
    {
        slot->vm.color1.r = 0xFF; slot->vm.color1.g = 0xD0; slot->vm.color1.b = 0xB0;
    }
    else
    {
        slot->vm.color1.r = 0xFF; slot->vm.color1.g = 0xFF; slot->vm.color1.b = 0xFF;
    }
    return 0;
}

// FUNCTION: th08 0x00450840
#pragma var_order(damageSlot, i)
i32 __fastcall FUN_00450840(Player *player, PlayerShot *slot)
{
    PlayerUnkStruct0x40 *damageSlot;
    i32 i;
    if (player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].instruction !=
            reinterpret_cast<EclTimelineInstruction *>(slot) ||
        g_Gui.IsDialogPresent() ||
        (i32)player->timerE2AC4 < 0 ||
        player->playerState == PLAYER_STATE_DYING ||
        player->bombState.isInUse != 0 ||
        g_GameManager.flags.unk13)
    {
        slot->vm.pendingInterrupt = 1;
        player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].instruction = NULL;
        *reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(slot) + 0x474) = 0;
    }
    if (player->optionStates[0].state2C8 == 0)
    {
        player->timelines[*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x466)].instruction = NULL;
        return 1;
    }
    for (i = 0; i < *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x46A); i++)
    {
        if (slot->vectors[i * 2].x >= -900.0f)
        {
            damageSlot = player->FUN_0044dfa0(&slot->vectors[i * 2], 16.0f, 448.0f, 1, 0);
            reinterpret_cast<u8 *>(damageSlot)[0x3D] = 1;
        }
    }
    for (i = 31; i > 0; i--)
    {
        slot->vectors[i] = slot->vectors[i - 1];
        slot->vectors[i].y -= 1.0f;
    }
    slot->vectors[0] = slot->position;
    slot->position = player->optionStates[0].position;
    slot->position.z = 0.44f;
    slot->hitboxSize.y = 448.0f;
    slot->position.y -= 208.0f;
    if (g_GameManager.GaugeIsExtremelyYoukai())
    {
        slot->vm.color1.r = 0xFF; slot->vm.color1.g = 0xD0; slot->vm.color1.b = 0xB0;
    }
    else
    {
        slot->vm.color1.r = 0xFF; slot->vm.color1.g = 0xFF; slot->vm.color1.b = 0xFF;
    }
    return 0;
}

// FUNCTION: th08 0x00450ad0
#pragma var_order(color, i, originalColor)
i32 __fastcall FUN_00450ad0(Player *player, PlayerShot *slot)
{
    i32 color;
    i32 i;
    i32 originalColor;

    color = slot->vm.color1.a;
    originalColor = color;
    color = color * 3 / 4;
    for (i = 0; i < *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x46A) * 2; i += 2)
    {
        if (slot->vectors[i].x == -999.0f)
            break;
        slot->vm.pos.x = slot->vectors[i].x;
        slot->vm.pos.y = slot->vectors[i].y;
        slot->vm.pos.z = slot->vectors[i].z;
        if (i != 0)
            slot->vm.color1.a = color - ((color / 2) * i) / *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(slot) + 0x46A);
        slot->vm.pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
        slot->vm.pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
        if (g_GameManager.GaugeIsExtremelyYoukai())
        {
            slot->vm.color1.r = 0xFF;
            slot->vm.color1.g = 0x40;
            slot->vm.color1.b = 0x40;
        }
        g_AnmManager->Draw2D(&slot->vm);
    }
    slot->vm.color1.a = originalColor;
    return 0;
}

// FUNCTION: th08 0x00450c50
i32 __fastcall FUN_00450c50(Player *player, PlayerShot *slot, Float3 *effectPosition)
{
    f32 angle;

    if (slot->state == 2)
    {
        if ((i32)slot->timer % 2 != 0)
            return 1;
        if (g_Spellcard.IsActive() && (i32)slot->timer % 4 != 0)
            return 1;
        slot->damage /= 3;
        if (slot->damage == 0)
            slot->damage = 1;
        slot->velocity.x *= 0.88f;
        slot->velocity.y *= 0.88f;
    }
    else
    {
        angle = g_Rng.GetRandomF32InRange(ZUN_PI / 2.0f) - 3.0f * ZUN_PI / 4.0f;
        switch (slot->vm.scriptIndex)
        {
        case 12: slot->hitboxSize.x = 48.0f; slot->hitboxSize.y = 48.0f; reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, 6.0f); break;
        case 14: slot->hitboxSize.x = 64.0f; slot->hitboxSize.y = 64.0f; reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, 6.0f); break;
        case 16: slot->hitboxSize.x = 80.0f; slot->hitboxSize.y = 80.0f; reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, 6.0f); break;
        case 18: slot->hitboxSize.x = 96.0f; slot->hitboxSize.y = 96.0f; reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, 6.0f); break;
        case 20: slot->hitboxSize.x = 128.0f; slot->hitboxSize.y = 128.0f; reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, 6.0f); break;
        default: break;
        }
    }
    if ((i32)slot->timer % 6 == 0)
        g_EffectManager.SpawnEffect(5, reinterpret_cast<D3DXVECTOR3 *>(effectPosition), 1, -1);
    return 0;
}

// FUNCTION: th08 0x00450ee0
i32 __fastcall FUN_00450ee0(Player *player, PlayerShot *slot, Float3 *effectPosition)
{
    (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(player) + 0xE2A94))++;
    if (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(player) + 0xE2A94) % 8 == 0)
    {
        Float3 position;
        position = *effectPosition;
        position.x = slot->position.x;
        g_EffectManager.SpawnEffect(5, reinterpret_cast<D3DXVECTOR3 *>(&position), 1, -1);
    }
    return 0;
}

// FUNCTION: th08 0x450f60
#pragma var_order(i, table, slot, result, entry, this, value)
void __fastcall Player::FUN_00450f60(i32 value)
{
    unsigned __int64 *table;
    u8 *entry;
    u8 *slot;
    i32 result;
    i32 i;

    table = (*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 3) == 0)
                ? reinterpret_cast<unsigned __int64 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A74) + 0x38)
                : reinterpret_cast<unsigned __int64 *>(*reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(this) + 0xE2A78) + 0x38);

    if (this->bombState.isInUse != 0 &&
        ((g_GameManager.shotType == 2 &&
          (this->bombState.callbackSetIndex & 1) != 0) ||
         g_GameManager.shotType == 9) &&
        this->bombState.timer >= 60)
    {
        table += ((this->bombState.callbackSetIndex & 2) ? 7 : 6);
    }
    else
    {
        while (g_GameManager.GetPower() >= *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(table) + 4))
        {
            table++;
        }
    }

    entry = *reinterpret_cast<u8 **>(table);
    slot = reinterpret_cast<u8 *>(this) + 0xBE838;
    for (i = 0; i < 0x80; i++, slot += 0x484)
    {
        if (*reinterpret_cast<i16 *>(slot + 0x462) != 0)
        {
            continue;
        }

processEntry:
        if (*reinterpret_cast<u32 *>(entry + 0x28) != 0)
        {
            result = reinterpret_cast<i32 (__fastcall *)(Player *, u8 *, i32, u8 *)>(
                *reinterpret_cast<u32 *>(entry + 0x28))(this, slot, value, entry);
        }
        else
        {
            result = this->FUN_0044fd80(slot, value, entry);
        }

        if (result == 1)
        {
            *reinterpret_cast<u32 *>(slot + 0x1F8) |= 0x2000;
            *reinterpret_cast<i16 *>(slot + 0x462) = 1;
            *reinterpret_cast<u8 **>(slot + 0x480) = entry;
            *reinterpret_cast<u32 *>(slot + 0x474) =
                *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(slot + 0x480) + 0x2C);
            *reinterpret_cast<u32 *>(slot + 0x478) =
                *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(slot + 0x480) + 0x30);
            *reinterpret_cast<u32 *>(slot + 0x47C) =
                *reinterpret_cast<u32 *>(*reinterpret_cast<u8 **>(slot + 0x480) + 0x34);
        }

        entry += 0x38;
        if (*reinterpret_cast<i16 *>(entry) < 0)
        {
            return;
        }
        if (result == 0)
        {
            goto processEntry;
        }
    }
}

// FUNCTION: th08 0x451150
#pragma var_order(i, slot, this)
void Player::FUN_00451150()
{
    u8 *slot;
    i32 i;

    if (g_GameManager.flags.unk10)
    {
        return;
    }

    slot = reinterpret_cast<u8 *>(this) + 0xBE838;
    for (i = 0; i < 0x80; i++, slot += 0x484)
    {
        if (*reinterpret_cast<i16 *>(slot + 0x462) == 0)
        {
            continue;
        }

        if (*reinterpret_cast<u32 *>(slot + 0x474) != 0)
        {
            if (reinterpret_cast<i32 (__fastcall *)(Player *, u8 *)>(*reinterpret_cast<u32 *>(slot + 0x474))(this, slot) != 0)
            {
                *reinterpret_cast<i16 *>(slot + 0x462) = 0;
                continue;
            }
        }

        reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[0] +=
            *reinterpret_cast<f32 *>(0x17CE8E0) * *reinterpret_cast<f32 *>(slot + 0x43C);
        reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[1] +=
            *reinterpret_cast<f32 *>(0x17CE8E0) * *reinterpret_cast<f32 *>(slot + 0x440);

        if (*reinterpret_cast<i16 *>(slot + 0x464) != 4 && *reinterpret_cast<i16 *>(slot + 0x464) != 5)
        {
            if (!g_GameManager.IsWithinPlayfield(
                    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[0],
                    reinterpret_cast<Float3 *>(slot + 0x2A4)->operator float *()[1],
                    *reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(slot + 0x224) + 0x34),
                    *reinterpret_cast<f32 *>(*reinterpret_cast<u8 **>(slot + 0x224) + 0x30)))
            {
                *reinterpret_cast<i16 *>(slot + 0x462) = 0;
            }
        }

        if (g_AnmManager->ExecuteScript(reinterpret_cast<AnmVm *>(slot)) != ZUN_SUCCESS)
        {
            *reinterpret_cast<i16 *>(slot + 0x462) = 0;
        }
        (*reinterpret_cast<ZunTimer *>(slot + 0x454))++;
    }
}
// FUNCTION: th08 0x4512f0
#pragma var_order(i, slot, this)
void Player::FUN_004512f0()
{
    u8 *slot;
    i32 i;

    slot = reinterpret_cast<u8 *>(this) + 0xBE838;
    for (i = 0; i < 0x80; i++, slot += 0x484)
    {
        if (*reinterpret_cast<i16 *>(slot + 0x462) != 1)
        {
            continue;
        }
        if (*reinterpret_cast<i16 *>(slot + 0x1FC) != 0)
        {
            reinterpret_cast<AnmVm *>(slot)->SetZRotation(*reinterpret_cast<f32 *>(slot + 0x450));
        }
        *reinterpret_cast<f32 *>(slot + 0x208) = g_GameManager.arcadeRegionTopLeftPos.x + *reinterpret_cast<f32 *>(slot + 0x2A4);
        *reinterpret_cast<f32 *>(slot + 0x20C) = g_GameManager.arcadeRegionTopLeftPos.y + *reinterpret_cast<f32 *>(slot + 0x2A8);
        *reinterpret_cast<f32 *>(slot + 0x210) = 0.4f;
        if (*reinterpret_cast<i8 *>(slot + 0x470) != 0)
        {
            *reinterpret_cast<u8 *>(slot + 0x1F2) = 0xff;
            *reinterpret_cast<u8 *>(slot + 0x1F1) = 0x40;
            *reinterpret_cast<u8 *>(slot + 0x1F0) = 0x40;
        }
        g_AnmManager->Draw2D(reinterpret_cast<AnmVm *>(slot));
        if (*reinterpret_cast<u32 *>(slot + 0x478) != 0)
        {
            reinterpret_cast<void (__fastcall *)(Player *, u8 *)>(*reinterpret_cast<u32 *>(slot + 0x478))(this, slot);
        }
    }
}

// FUNCTION: th08 0x451400
#pragma var_order(i, slot, this)
void Player::FUN_00451400()
{
    u8 *slot;
    i32 i;

    slot = reinterpret_cast<u8 *>(this) + 0xBE838;
    for (i = 0; i < 0x80; i++, slot += 0x484)
    {
        if (*reinterpret_cast<i16 *>(slot + 0x462) != 2)
        {
            continue;
        }
        if (*reinterpret_cast<i16 *>(slot + 0x1FC) != 0)
        {
            reinterpret_cast<AnmVm *>(slot)->SetZRotation(*reinterpret_cast<f32 *>(slot + 0x450));
        }
        *reinterpret_cast<f32 *>(slot + 0x208) = g_GameManager.arcadeRegionTopLeftPos.x + *reinterpret_cast<f32 *>(slot + 0x2A4);
        *reinterpret_cast<f32 *>(slot + 0x20C) = g_GameManager.arcadeRegionTopLeftPos.y + *reinterpret_cast<f32 *>(slot + 0x2A8);
        *reinterpret_cast<f32 *>(slot + 0x210) = 0.2f;
        if (*reinterpret_cast<i8 *>(slot + 0x470) != 0)
        {
            *reinterpret_cast<u8 *>(slot + 0x1F2) = 0xff;
            *reinterpret_cast<u8 *>(slot + 0x1F1) = 0x40;
            *reinterpret_cast<u8 *>(slot + 0x1F0) = 0x40;
        }
        g_AnmManager->DrawPlayerBullet(reinterpret_cast<AnmVm *>(slot));
    }
}
// FUNCTION: th08 0x451500
i32 Player::FUN_00451500()
{
    if (*reinterpret_cast<i32 *>(0x164D2C8) < 20)
    {
        return 0;
    }

    if ((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) < 0)
    {
        return 0;
    }

    if (this->FUN_00451d50())
    {
        return 0;
    }

    if (reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4)->FUN_0040d3d0())
    {
        if (*reinterpret_cast<i32 *>(0x17D6ED4) == 0 ||
            (g_GameManager.shotType != 1 && g_GameManager.shotType != 7 &&
             g_GameManager.shotType != 6))
        {
            this->FUN_00450f60((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4));
        }
    }

    (*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4))++;

    if ((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) >= 20)
    {
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) = -1;
    }

    if ((*reinterpret_cast<u16 *>(0x164D52C) & 1) != 0)
    {
        if ((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) < 0)
        {
            if (!g_Gui.IsDialogPresent())
            {
                *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) = 0;
            }
        }
    }

    if (*reinterpret_cast<i8 *>(this) == 2 || *reinterpret_cast<i8 *>(this) == 1)
    {
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) = -1;
    }

    return 0;
}

// FUNCTION: th08 0x451640
void Player::FUN_00451640()
{
    if ((i32)*reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) < 0)
        *reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AC4) = 0;
}

// FUNCTION: th08 0x451670
#pragma var_order(bullet, i, enemyBottomRight, savedRotation, bulletBottomRight, enemyTopLeft, damage, region, bulletTopLeft)
i32 Player::FUN_00451670(Float3 *enemyPosition, Float3 *enemySize, i32 *hitAccumulator, i32 *bombHit)
{
    Float3 enemyTopLeft;
    Float3 enemyBottomRight;
    Float3 bulletTopLeft;
    Float3 bulletBottomRight;
    i32 damage;
    i32 i;
    i32 savedRotation;
    PlayerUnkStruct0x40 *region;
    PlayerShot *bullet;

    damage = 0;
    if (!reinterpret_cast<ZunTimer *>(reinterpret_cast<u8 *>(this) + 0xE2AF4)->FUN_0040d3d0())
        return 0;

    PlayerBuildAabb(&enemyTopLeft, &enemyBottomRight, enemyPosition, enemySize);
    bullet = reinterpret_cast<PlayerShot *>(reinterpret_cast<u8 *>(this) + 0xBE838);
    if (bombHit != NULL)
        *bombHit = 0;

    for (i = 0; i < 128; i++, bullet++)
    {
        if (bullet->state == 0 || (bullet->state != 1 && bullet->type != 3))
            continue;

        PlayerBuildAabb(&bulletTopLeft, &bulletBottomRight, &bullet->position, &bullet->hitboxSize);
        if (bulletTopLeft.y > enemyBottomRight.y || bulletTopLeft.x > enemyBottomRight.x ||
            bulletBottomRight.y < enemyTopLeft.y || bulletBottomRight.x < enemyTopLeft.x)
            continue;

        if ((bullet->type == 4 || bullet->type == 5) && (bullet->timer % 2) != 0)
            continue;
        if (bullet->collisionCallback != NULL && bullet->collisionCallback(this, bullet, enemyPosition))
            continue;

        if (this->bombState.isInUse == 0)
            damage += bullet->damage;
        else
            damage += bullet->damage / 5 ? bullet->damage / 5 : 1;

        while (*hitAccumulator >= g_Player.damageAccumulatorThreshold)
        {
            if (g_GameManager.GaugeIsExtremelyHuman())
            {
                if (*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(bullet->shtEntry) + 0x1E) < 0)
                    g_ItemManager.SpawnItem(&bullet->position, static_cast<ItemType>(7), 3);
            }
            *hitAccumulator -= g_Player.damageAccumulatorThreshold;
        }

        if (bullet->type != 4 && bullet->type != 5 && bullet->type != 6)
        {
            if (bullet->state == 1)
            {
                savedRotation = *reinterpret_cast<i32 *>(&bullet->vm.rotation.z);
                this->anmFile->SetAndExecuteScriptIdx(&bullet->vm, bullet->animationIndex + 11);
                *reinterpret_cast<i32 *>(&bullet->vm.rotation.z) = savedRotation;
                g_EffectManager.SpawnEffect(5, reinterpret_cast<D3DXVECTOR3 *>(&bullet->position), 1, -1);
                bullet->position.operator float *()[2] = 0.1f;
            }
            bullet->state = 2;
            if (bullet->type != 3)
            {
                bullet->velocity.x /= 8.0f;
                bullet->velocity.y /= 8.0f;
            }
        }
    }

    *hitAccumulator += damage > 50 ? 50 : damage;

    {
        region = this->playerSlotsB;
        for (i = 0; i < 192; i++, region++)
        {
            if (!region->active)
                continue;
            if ((region->lifetime % region->collisionInterval) != 0)
                continue;

            if (region->radius == 0.0f)
            {
                if (region->angle == 0.0f)
                {
                    if (region->center.x - region->size.x / 2.0f > enemyBottomRight.x ||
                        region->center.x + region->size.x / 2.0f < enemyTopLeft.x ||
                        region->center.y - region->size.y / 2.0f > enemyBottomRight.y ||
                        region->center.y + region->size.y / 2.0f < enemyTopLeft.y)
                        continue;
                }
                else
                {
                    bulletTopLeft.x = enemyPosition->x - region->center.x;
                    bulletTopLeft.y = enemyPosition->y - region->center.y;
                    Rotate(&bulletBottomRight, &bulletTopLeft, -region->angle);
                    if (-region->size.x / 2.0f > enemySize->x / 2.0f + bulletBottomRight.x ||
                        region->size.x / 2.0f < bulletBottomRight.x - enemySize->x / 2.0f ||
                        -region->size.y / 2.0f > enemySize->y / 2.0f + bulletBottomRight.y ||
                        region->size.y / 2.0f < bulletBottomRight.y - enemySize->y / 2.0f)
                        continue;
                }
            }
            else if (region->radius * region->radius <
                     (region->center.x - enemyPosition->x) * (region->center.x - enemyPosition->x) +
                         (region->center.y - enemyPosition->y) * (region->center.y - enemyPosition->y))
            {
                continue;
            }

            damage += region->damage;
            region->hitAccumulator += region->damage;
            if (region->hitCap > 0 && region->hitCap <= region->hitAccumulator)
            {
                region->damage = 0;
                damage -= region->hitAccumulator - region->hitCap;
            }

            if (region->mode == 0 && (++*reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(this) + 0xE2A94) % 4) == 0)
            {
                if (i < 192)
                    g_EffectManager.SpawnEffect(3, reinterpret_cast<D3DXVECTOR3 *>(enemyPosition), 1, -1);
                else
                    g_EffectManager.SpawnEffect(5, reinterpret_cast<D3DXVECTOR3 *>(enemyPosition), 1, -1);
            }
            if (this->bombState.isInUse != 0 && bombHit != NULL)
                *bombHit = 1;
        }
    }

    if (g_GameManager.GaugeIsExtremelyYoukai() && damage != 0)
        damage = damage * 106 / 100;
    return damage;
}

// FUNCTION: th08 0x451ce0
void __fastcall PlayerBuildAabb(Float3 *topLeft, Float3 *bottomRight, const Float3 *center, const Float3 *size)
{
    topLeft->x = center->x - size->x * 0.5f;
    topLeft->y = center->y - size->y * 0.5f;
    bottomRight->x = center->x + size->x * 0.5f;
    bottomRight->y = center->y + size->y * 0.5f;
}

// FUNCTION: th08 0x451d50
i32 Player::FUN_00451d50()
{
    return *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xfdc) != 0 &&
           *reinterpret_cast<i32 *>(reinterpret_cast<u8 *>(this) + 0xfe0) == 4;
}
} /* namespace th08 */
