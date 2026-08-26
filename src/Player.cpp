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

void __fastcall UpdateFantasyOrbBomb(Player *player);
void __fastcall DrawFantasyOrbBomb(Player *player);
void __fastcall UpdateFantasySealBlinkDeathbomb(Player *player);
void __fastcall DrawFantasySealBlinkDeathbomb(Player *player);
void __fastcall UpdateDissolveSpell(Player *player);
void __fastcall DrawDissolveSpell(Player *player);
void __fastcall UpdateArtfulSacrificeBomb(Player *player);
void __fastcall DrawDefaultBombTint(Player *player);
void __fastcall UpdateReturnInanimatenessDeathbomb(Player *player);
void __fastcall DrawReturnInanimatenessDeathbomb(Player *player);
void __fastcall UpdateMasterSparkBomb(Player *player);
void __fastcall DrawMasterSparkBomb(Player *player);
void __fastcall UpdateFinalSparkDeathbomb(Player *player);
void __fastcall UpdateRedNightlessCastleBomb(Player *player);
void __fastcall DrawRedNightlessCastleBomb(Player *player);
void __fastcall UpdateScarletDevilDeathbomb(Player *player);
void __fastcall DrawScarletDevilDeathbomb(Player *player);
void __fastcall UpdateKillingDollBomb(Player *player);
void __fastcall DrawKillingDollBomb(Player *player);
void __fastcall UpdateNightMistPhantomKillerDeathbomb(Player *player);
void __fastcall DrawNightMistPhantomKillerDeathbomb(Player *player);
void __fastcall UpdateQuadrupleBarrierBomb(Player *player);
void __fastcall UpdateEternalNightQuadrupleBarrierDeathbomb(Player *player);
void __fastcall DrawEternalNightQuadrupleBarrierDeathbomb(Player *player);
void __fastcall UpdateSlashOfPresentWorldBomb(Player *player);
void __fastcall DrawSlashOfPresentWorldBomb(Player *player);
void __fastcall UpdateSlashOfFutureEternityDeathbomb(Player *player);
void __fastcall DrawSlashOfFutureEternityDeathbomb(Player *player);
void __fastcall UpdateGhastlyDreamBomb(Player *player);
void __fastcall DrawGhastlyDreamBomb(Player *player);
void __fastcall UpdateEternalSleepInDreamlandDeathbomb(Player *player);
void __fastcall DrawEternalSleepInDreamlandDeathbomb(Player *player);

DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerBombCallbackSet, 24, g_PlayerBombCallbacksByShotType) = {
    {{UpdateFantasyOrbBomb, UpdateQuadrupleBarrierBomb, UpdateFantasySealBlinkDeathbomb, UpdateEternalNightQuadrupleBarrierDeathbomb, UpdateDissolveSpell}},
    {{DrawFantasyOrbBomb, DrawDefaultBombTint, DrawFantasySealBlinkDeathbomb, DrawEternalNightQuadrupleBarrierDeathbomb, DrawDissolveSpell}},
    {{UpdateMasterSparkBomb, UpdateArtfulSacrificeBomb, UpdateFinalSparkDeathbomb, UpdateReturnInanimatenessDeathbomb, UpdateDissolveSpell}},
    {{DrawMasterSparkBomb, DrawDefaultBombTint, DrawMasterSparkBomb, DrawReturnInanimatenessDeathbomb, DrawDissolveSpell}},
    {{UpdateKillingDollBomb, UpdateRedNightlessCastleBomb, UpdateNightMistPhantomKillerDeathbomb, UpdateScarletDevilDeathbomb, UpdateDissolveSpell}},
    {{DrawKillingDollBomb, DrawRedNightlessCastleBomb, DrawNightMistPhantomKillerDeathbomb, DrawScarletDevilDeathbomb, DrawDissolveSpell}},
    {{UpdateSlashOfPresentWorldBomb, UpdateGhastlyDreamBomb, UpdateSlashOfFutureEternityDeathbomb, UpdateEternalSleepInDreamlandDeathbomb, UpdateDissolveSpell}},
    {{DrawSlashOfPresentWorldBomb, DrawGhastlyDreamBomb, DrawSlashOfFutureEternityDeathbomb, DrawEternalSleepInDreamlandDeathbomb, DrawDissolveSpell}},
    {{UpdateFantasyOrbBomb, UpdateFantasyOrbBomb, UpdateFantasySealBlinkDeathbomb, UpdateFantasySealBlinkDeathbomb, UpdateDissolveSpell}},
    {{DrawFantasyOrbBomb, DrawFantasyOrbBomb, DrawFantasySealBlinkDeathbomb, DrawFantasySealBlinkDeathbomb, DrawDissolveSpell}},
    {{UpdateQuadrupleBarrierBomb, UpdateQuadrupleBarrierBomb, UpdateEternalNightQuadrupleBarrierDeathbomb, UpdateEternalNightQuadrupleBarrierDeathbomb, UpdateDissolveSpell}},
    {{DrawDefaultBombTint, DrawDefaultBombTint, DrawEternalNightQuadrupleBarrierDeathbomb, DrawEternalNightQuadrupleBarrierDeathbomb, DrawDissolveSpell}},
    {{UpdateMasterSparkBomb, UpdateMasterSparkBomb, UpdateFinalSparkDeathbomb, UpdateFinalSparkDeathbomb, UpdateDissolveSpell}},
    {{DrawMasterSparkBomb, DrawMasterSparkBomb, DrawMasterSparkBomb, DrawMasterSparkBomb, DrawDissolveSpell}},
    {{UpdateArtfulSacrificeBomb, UpdateArtfulSacrificeBomb, UpdateReturnInanimatenessDeathbomb, UpdateReturnInanimatenessDeathbomb, UpdateDissolveSpell}},
    {{DrawDefaultBombTint, DrawDefaultBombTint, DrawReturnInanimatenessDeathbomb, DrawReturnInanimatenessDeathbomb, DrawDissolveSpell}},
    {{UpdateKillingDollBomb, UpdateKillingDollBomb, UpdateNightMistPhantomKillerDeathbomb, UpdateNightMistPhantomKillerDeathbomb, UpdateDissolveSpell}},
    {{DrawKillingDollBomb, DrawKillingDollBomb, DrawNightMistPhantomKillerDeathbomb, DrawNightMistPhantomKillerDeathbomb, DrawDissolveSpell}},
    {{UpdateRedNightlessCastleBomb, UpdateRedNightlessCastleBomb, UpdateScarletDevilDeathbomb, UpdateScarletDevilDeathbomb, UpdateDissolveSpell}},
    {{DrawRedNightlessCastleBomb, DrawRedNightlessCastleBomb, DrawScarletDevilDeathbomb, DrawScarletDevilDeathbomb, DrawDissolveSpell}},
    {{UpdateSlashOfPresentWorldBomb, UpdateSlashOfPresentWorldBomb, UpdateSlashOfFutureEternityDeathbomb, UpdateSlashOfFutureEternityDeathbomb, UpdateDissolveSpell}},
    {{DrawSlashOfPresentWorldBomb, DrawSlashOfPresentWorldBomb, DrawSlashOfFutureEternityDeathbomb, DrawSlashOfFutureEternityDeathbomb, DrawDissolveSpell}},
    {{UpdateGhastlyDreamBomb, UpdateGhastlyDreamBomb, UpdateEternalSleepInDreamlandDeathbomb, UpdateEternalSleepInDreamlandDeathbomb, UpdateDissolveSpell}},
    {{DrawGhastlyDreamBomb, DrawGhastlyDreamBomb, DrawEternalSleepInDreamlandDeathbomb, DrawEternalSleepInDreamlandDeathbomb, DrawDissolveSpell}},
};
i32 __fastcall UpdateHomingOption(Player *player, PlayerOptionState *option);
i32 __fastcall UpdateBombAnchorOption(Player *player, PlayerOptionState *option);
i32 __fastcall UpdateOrbitingOption(Player *player, PlayerOptionState *option);
i32 __fastcall UpdateModeSensitiveOrbitingOption(Player *player, PlayerOptionState *option);
i32 __fastcall UpdateFacingTrailOption(Player *player, PlayerOptionState *option);
i32 __fastcall UpdateModeSensitiveFacingOption(Player *player, PlayerOptionState *option);
i32 __fastcall UpdateTwinOrbitingOption(Player *player, PlayerOptionState *option);
i32 __fastcall DrawPlayerOption(Player *player, PlayerOptionState *option);
i32 __fastcall SpawnShotAlongPlayerAngle(Player *player, PlayerShot *shot, i32 value,
                                         PlayerShotDescriptor *descriptor);
i32 __fastcall SpawnShotAlongOptionAngle(Player *player, PlayerShot *shot, i32 value,
                                         PlayerShotDescriptor *descriptor);
i32 __fastcall SpawnRandomizedShot(Player *player, PlayerShot *shot, i32 value,
                                   PlayerShotDescriptor *descriptor);
i32 __fastcall SpawnHomingShot(Player *player, PlayerShot *shot, i32 value,
                               PlayerShotDescriptor *descriptor);
i32 __fastcall UpdateHomingShot(Player *player, PlayerShot *shot);
i32 __fastcall UpdateFallingShot(Player *player, PlayerShot *shot);
i32 __fastcall UpdatePersistentShot(Player *player, PlayerShot *shot);
i32 __fastcall UpdateShotTrail(Player *player, PlayerShot *shot);
i32 __fastcall DrawShotTrail(Player *player, PlayerShot *shot);
i32 __fastcall ApplyShotHitBehavior(Player *player, PlayerShot *shot, Float3 *effectPosition);
i32 __fastcall SpawnPeriodicShotHitEffect(Player *player, PlayerShot *shot,
                                          Float3 *effectPosition);

static i32 __fastcall SpawnShotUnlessBombingCallback(Player *player, PlayerShot *shot,
                                                     i32 value, PlayerShotDescriptor *descriptor)
{
    return player->SpawnShotOnScheduleUnlessBombing(shot, value, descriptor);
}

static i32 __fastcall SpawnPersistentShotCallback(Player *player, PlayerShot *shot,
                                                  i32 value, PlayerShotDescriptor *descriptor)
{
    return player->SpawnPersistentShot(shot, value, descriptor);
}

static i32 __fastcall SpawnShotAimedAtTrackedPointCallback(Player *player, PlayerShot *shot,
                                                           i32 value,
                                                           PlayerShotDescriptor *descriptor)
{
    return player->SpawnShotAimedAtTrackedPoint(shot, value, descriptor);
}

struct PlayerOptionCallbackRow
{
    PlayerOptionCallback callbacks[4];
};
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerOptionCallbackRow, 12, g_PlayerOptionUpdateCallbacks) = {
    {{UpdateHomingOption, NULL, NULL, NULL}},
    {{UpdateBombAnchorOption, NULL, NULL, NULL}},
    {{UpdateOrbitingOption, UpdateOrbitingOption, UpdateOrbitingOption, UpdateOrbitingOption}},
    {{UpdateTwinOrbitingOption, UpdateTwinOrbitingOption, NULL, NULL}},
    {{NULL, NULL, NULL, NULL}},
    {{UpdateHomingOption, NULL, NULL, NULL}},
    {{NULL, NULL, NULL, NULL}},
    {{UpdateBombAnchorOption, NULL, NULL, NULL}},
    {{NULL, NULL, NULL, NULL}},
    {{UpdateModeSensitiveOrbitingOption, UpdateModeSensitiveOrbitingOption, UpdateModeSensitiveOrbitingOption, UpdateModeSensitiveOrbitingOption}},
    {{NULL, NULL, UpdateModeSensitiveFacingOption, NULL}},
    {{UpdateTwinOrbitingOption, UpdateTwinOrbitingOption, NULL, NULL}},
};
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerOptionCallbackRow, 12, g_PlayerOptionRenderCallbacks) = {
    {{DrawPlayerOption, NULL, NULL, NULL}},
    {{DrawPlayerOption, NULL, NULL, NULL}},
    {{DrawPlayerOption, DrawPlayerOption, DrawPlayerOption, DrawPlayerOption}},
    {{DrawPlayerOption, DrawPlayerOption, NULL, NULL}},
    {{NULL, NULL, NULL, NULL}},
    {{DrawPlayerOption, NULL, NULL, NULL}},
    {{NULL, NULL, NULL, NULL}},
    {{DrawPlayerOption, NULL, NULL, NULL}},
    {{NULL, NULL, NULL, NULL}},
    {{DrawPlayerOption, DrawPlayerOption, DrawPlayerOption, DrawPlayerOption}},
    {{NULL, NULL, DrawPlayerOption, NULL}},
    {{DrawPlayerOption, DrawPlayerOption, NULL, NULL}},
};
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerOptionCallback, 4, g_PlayerRoute3ExitUpdateCallbacks) = {
    NULL, NULL, UpdateFacingTrailOption, NULL};
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerOptionCallback, 4, g_PlayerRoute3ExitRenderCallbacks) = {
    NULL, NULL, DrawPlayerOption, NULL};

DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerShotSpawnCallback, 9, g_PlayerShotSpawnCallbacks) = {
    NULL, SpawnHomingShot, SpawnShotUnlessBombingCallback, SpawnShotUnlessBombingCallback,
    SpawnPersistentShotCallback, SpawnShotAimedAtTrackedPointCallback,
    SpawnShotAlongPlayerAngle, SpawnRandomizedShot, SpawnShotAlongOptionAngle};
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerShotUpdateCallback, 6, g_PlayerShotUpdateCallbacks) = {
    NULL, UpdateHomingShot, NULL, UpdateFallingShot, UpdatePersistentShot, UpdateShotTrail};
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerShotDrawCallback, 2, g_PlayerShotDrawCallbacks) = {
    NULL, DrawShotTrail};
typedef void *PlayerShotCollisionOrDifficultyEntry;
DIFFABLE_STATIC_ARRAY_ASSIGN(PlayerShotCollisionOrDifficultyEntry, 9,
                             g_PlayerShotCollisionOrDifficultyTable) = {
    NULL,
    reinterpret_cast<void *>(ApplyShotHitBehavior),
    reinterpret_cast<void *>(SpawnPeriodicShotHitEffect),
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
i32 Player::CheckBulletCancelCollision(Float3 *position, Float3 *position2)
{
    Float3 delta;
    Float3 rotated;
    Float3 halfSize;
    Float3 boundsMax;
    PlayerCollisionRegion *slot = this->cancelRegions;
    i32 i;
    f32 xDelta;
    f32 yDelta;

    for (i = 0; i < ARRAY_SIZE_SIGNED(this->cancelRegions); i++, slot++)
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
    this->bulletCancelItemType = slot->collisionValue;
    slot->hitAccumulator++;
    return 2;
}

// FUNCTION: th08 0x44a230
#pragma var_order(boundsMax, boundsMin)
i32 Player::CheckBulletCollision(Float3 *position, Float3 *size)
{
    Float3 boundsMin;
    Float3 boundsMax;

    this->bulletCancelItemType = 6;
    if (this->CheckBulletCancelCollision(position, size))
        return 2;

    boundsMin.x = position->x - size->x / 2.0f;
    boundsMin.y = position->y - size->y / 2.0f;
    boundsMax.x = size->x / 2.0f + position->x;
    boundsMax.y = size->y / 2.0f + position->y;

    if (this->hurtboxBoundsMin.x > boundsMax.x ||
        this->hurtboxBoundsMin.y > boundsMax.y ||
        this->hurtboxBoundsMax.x < boundsMin.x ||
        this->hurtboxBoundsMax.y < boundsMin.y)
        return 0;

    g_ReplayManager->flags |= 2;
    if (this->playerState != PLAYER_STATE_ALIVE)
        return 1;
    g_GameManager.RandomizeAntiTamper();
    this->Die();
    return 1;
}

// FUNCTION: th08 0x44a360
#pragma var_order(boundsMax, boundsMin)
i32 Player::CheckLethalCollision(Float3 *position, Float3 *size)
{
    Float3 boundsMin;
    Float3 boundsMax;

    this->bulletCancelItemType = 6;
    boundsMin.x = position->x - size->x / 2.0f;
    boundsMin.y = position->y - size->y / 2.0f;
    boundsMax.x = size->x / 2.0f + position->x;
    boundsMax.y = size->y / 2.0f + position->y;

    if (this->hurtboxBoundsMin.x > boundsMax.x ||
        this->hurtboxBoundsMin.y > boundsMax.y ||
        this->hurtboxBoundsMax.x < boundsMin.x ||
        this->hurtboxBoundsMax.y < boundsMin.y)
        return 0;

    g_ReplayManager->flags |= 2;
    if (this->playerState != PLAYER_STATE_ALIVE)
        return 1;
    g_GameManager.RandomizeAntiTamper();
    this->Die();
    return 1;
}

// FUNCTION: th08 0x44a470
#pragma var_order(boundsMax, boundsMin)
i32 Player::CheckGrazeCollision(Float3 *position, Float3 *size)
{
    Float3 boundsMin;
    Float3 boundsMax;

    this->bulletCancelItemType = 6;
    if (this->CheckBulletCancelCollision(position, size))
        return 2;

    boundsMin.x = position->x - size->x / 2.0f - 20.0f;
    boundsMin.y = position->y - size->y / 2.0f - 20.0f;
    boundsMax.x = size->x / 2.0f + position->x + 20.0f;
    boundsMax.y = size->y / 2.0f + position->y + 20.0f;

    if (this->playerState == PLAYER_STATE_DYING || this->playerState == PLAYER_STATE_SPAWNING)
        return 0;

    if (this->grazeBoundsMin.x > boundsMax.x ||
        this->grazeBoundsMax.x < boundsMin.x ||
        this->grazeBoundsMin.y > boundsMax.y ||
        this->grazeBoundsMax.y < boundsMin.y)
        return 0;

    this->AwardGraze(position, 0);
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

    if (this->itemCollectionBoundsMin.x > itemMax.x ||
        this->itemCollectionBoundsMax.x < itemMin.x ||
        this->itemCollectionBoundsMin.y > itemMax.y ||
        this->itemCollectionBoundsMax.y < itemMin.y)
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

    playerMin = incomingMin - this->hurtboxHalfSize;
    playerMax = incomingMin + this->hurtboxHalfSize;
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
        this->AwardGraze(&this->position, 1);
        return 2;
    }

lethalPath:
    g_ReplayManager->flags |= 2;
    if (this->playerState != PLAYER_STATE_ALIVE)
        return 0;
    g_GameManager.RandomizeAntiTamper();
    this->Die();
    return 1;
}

// FUNCTION: th08 0x44a930
#pragma var_order(midpoint, gaugeGain, score)
void Player::AwardGraze(Float3 *position, i32 suppressExtraItems)
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

    g_ReplayManager->flags |= 0x2000;
}

// FUNCTION: th08 0x44ab40
#pragma var_order(effectVm)
void Player::Die()
{
    AnmVm *effectVm;

    utils::DebugPrint("player DEAD");
    g_GameManager.scriptedUpdateFreeze = 0;
    g_GameManager.UpdateAntiTamper();
    g_EffectManager.SpawnEffect(6, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 16, -1);
    this->playerState = PLAYER_STATE_DYING;
    this->timer = 0;
    g_SoundPlayer.PlaySoundPositionedByIdx(SOUND_PICHUN, this->position.x);
    g_ReplayManager->flags |= 0x200;

    // VC7 lowers the two-bit field read as two independent tests at /Od.
    // Read the typed flag storage once so the target's single extraction is
    // retained while the layout remains described by GameManagerFlags.
    if (((*reinterpret_cast<const u32 *>(&g_GameManager.flags) >>
          GameManagerFlags::PLAYER_DEATH_DISSOLVE_SHIFT) &
         GameManagerFlags::PLAYER_DEATH_DISSOLVE_MASK) != 0)
    {
        utils::DebugPrint(" desolve\n");
        this->deathbombWindowFrames = 2;
        this->forceDeathbombAtWindowEnd = 1;
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
            this->mainVm.color2.r = 0xFF;
            this->mainVm.color2.g = 0xFF;
            this->mainVm.color2.b = 0xFF;
            this->mainVm.color2.a = this->mainVm.color1.a;
            this->mainVm.flagsWord |= 0x20000;

            this->deathbombEffectVm =
                g_EffectManager.FUN_00425870(59, reinterpret_cast<D3DXVECTOR3 *>(&this->position),
                                              11, 1, 0xFFF0404F);
            effectVm = this->deathbombEffectVm;
            effectVm->interpCurrentTimers[AnmInterp_Pos] = 0;
            effectVm->interpEndTimers[AnmInterp_Pos] = this->deathbombWindowFrames;
            effectVm->interpModes[AnmInterp_Pos] = AnmInterpMode_EaseOut;
            effectVm->posInitial.x = 128.0f;
            effectVm->posFinal.x = 8.0f;
            effectVm->posInitial.y = 32.0f;
            effectVm->posFinal.y = 0.0f;
            effectVm->pos.x = 128.0f;
            effectVm->pos.y = 32.0f;
            reinterpret_cast<Effect *>(effectVm)->vertexSegmentCount = 64;
            reinterpret_cast<Effect *>(effectVm)->angle = 0.0f;
            reinterpret_cast<Effect *>(effectVm)->radius = 128.0f;
            reinterpret_cast<Effect *>(effectVm)->shapeThickness = 15.0f;
            reinterpret_cast<Effect *>(effectVm)->radialWaveCount = 6.0f;
            reinterpret_cast<Effect *>(effectVm)->updateDuringFreeze = 1;

            if (g_Spellcard.IsActive())
                g_GameManager.flags.deathbombFreezeActive = 1;
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
i32 Player::UpdateMovementAndOptions()
{
    i32 oldDirection;
    f32 verticalSpeed;
    f32 horizontalSpeed;
    i32 focus;
    PlayerOptionState *option;
    u32 optionIndex;
    PlayerOptionState *option2;
    u32 optionExitIndex;
    i32 route3Index;
    u32 historyInitIndex;
    u32 optionUpdateIndex;
    i32 gaugeDelta;
    i32 historyIndex;

    horizontalSpeed = 0.0f;
    verticalSpeed = 0.0f;
    oldDirection = this->movementDirection;

    if ((g_GuiMessageInputCurrent & 0x50) == 0x50)
        this->movementDirection = PLAYER_DIRECTION_UP_LEFT;
    else if ((g_GuiMessageInputCurrent & 0x60) == 0x60)
        this->movementDirection = PLAYER_DIRECTION_DOWN_LEFT;
    else if ((g_GuiMessageInputCurrent & 0x90) == 0x90)
        this->movementDirection = PLAYER_DIRECTION_UP_RIGHT;
    else if ((g_GuiMessageInputCurrent & 0xA0) == 0xA0)
        this->movementDirection = PLAYER_DIRECTION_DOWN_RIGHT;
    else if ((g_GuiMessageInputCurrent & 0x20) != 0)
        this->movementDirection = PLAYER_DIRECTION_DOWN;
    else if ((g_GuiMessageInputCurrent & 0x10) != 0)
        this->movementDirection = PLAYER_DIRECTION_UP;
    else if ((g_GuiMessageInputCurrent & 0x40) != 0)
        this->movementDirection = PLAYER_DIRECTION_LEFT;
    else if ((g_GuiMessageInputCurrent & 0x80) != 0)
        this->movementDirection = PLAYER_DIRECTION_RIGHT;
    else
        this->movementDirection = PLAYER_DIRECTION_NONE;

    focus = this->bombState.isInUse
                ? (this->bombState.callbackVariant & 1)
                : (g_GuiMessageInputCurrent & 4);

    if (focus)
    {
        if (this->optionModeFlag != 1)
        {
            if (g_GameManager.shotType <= 3)
            {
                option = this->optionStates;
                for (optionIndex = 0; optionIndex < 4; optionIndex++, option++)
                {
                    memset(option, 0, 0x2F4);
                    option->updateCallback =
                        g_PlayerOptionUpdateCallbacks[g_GameManager.shotType].callbacks[optionIndex];
                    option->renderCallback =
                        g_PlayerOptionRenderCallbacks[g_GameManager.shotType].callbacks[optionIndex];
                    if (option->updateCallback != NULL)
                    {
                        option->state2C8 = 1;
                        option->timer = 0;
                        option->optionIndex = optionIndex;
                    }
                    else
                    {
                        option->state2C8 = 0;
                    }
                }
            }

            if (g_GameManager.shotType < 4)
            {
                this->anmFile->SetAndExecuteScriptIdx(&this->mainVm, 5);
                this->currentHorizontalSpeed = 0.0f;
                if (this->focusTransitionFrames >= 4)
                    g_EffectManager.SpawnEffect(29, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 1, 0x80FF8080);
            }
            if (this->focusEffect == NULL)
            {
                this->focusEffect = reinterpret_cast<Effect *>(
                    g_EffectManager.FUN_00425870(22, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 2, 1, -1));
            }
            this->focusTransitionFrames = 0;
            this->timerE2AE8 = 0;
        }
        else
        {
            ++this->focusTransitionFrames;
        }
        if (this->focusTransitionFrames >= 7)
            this->isYoukai = 1;
        this->optionModeFlag = 1;
    }
    else
    {
        if (this->optionModeFlag != 0)
        {
            option2 = this->optionStates;
            if (g_GameManager.shotType < 3)
            {
                for (optionExitIndex = 0; optionExitIndex < 4; optionExitIndex++, option2++)
                {
                    if (option2->state2C8 != 0 && option2->state2C8 != 3)
                    {
                        option2->state2C8 = 3;
                        option2->timer = 0;
                    }
                }
            }
            else if (g_GameManager.shotType == 3)
            {
                for (route3Index = 0; route3Index < 2; route3Index++, option2++)
                {
                    if (option2->state2C8 != 0 && option2->state2C8 != 3)
                    {
                        option2->state2C8 = 3;
                        option2->timer = 0;
                    }
                }
                memset(option2, 0, 0x2F4);
                option2->updateCallback =
                    g_PlayerRoute3ExitUpdateCallbacks[route3Index];
                option2->renderCallback =
                    g_PlayerRoute3ExitRenderCallbacks[route3Index];
                option2->state2C8 = 1;
                option2->timer = 0;
                option2->optionIndex = route3Index;
                for (historyInitIndex = 0; historyInitIndex < 16; ++historyInitIndex)
                    this->positionHistory[historyInitIndex] = this->position;
            }

            if (g_GameManager.shotType < 4)
            {
                this->anmFile->SetAndExecuteScriptIdx(&this->mainVm, 0);
                this->currentHorizontalSpeed = 0.0f;
                if (this->focusTransitionFrames >= 4)
                    g_EffectManager.SpawnEffect(28, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 1, 0x808080FF);
            }
            if (this->focusEffect != NULL)
                this->focusEffect->vm.SetInterrupt(1);
            this->focusEffect = NULL;
            this->focusTransitionFrames = 0;
            this->timerE2AE8 = 0;
        }
        else
        {
            ++this->focusTransitionFrames;
        }
        if (this->focusTransitionFrames >= 7)
            this->isYoukai = 0;
        this->optionModeFlag = 0;
    }

    if (g_GameManager.shotType >= 4)
    {
        if ((g_GameManager.shotType & 1) != 0)
            this->isYoukai = 1;
        else
            this->isYoukai = 0;
    }

    if (this->optionModeFlag != 0)
    {
        switch (this->movementDirection)
        {
        case PLAYER_DIRECTION_RIGHT: horizontalSpeed =  this->secondaryShtFile->focusedAxisSpeed; break;
        case PLAYER_DIRECTION_LEFT: horizontalSpeed = -this->secondaryShtFile->focusedAxisSpeed; break;
        case PLAYER_DIRECTION_UP: verticalSpeed =   -this->secondaryShtFile->focusedAxisSpeed; break;
        case PLAYER_DIRECTION_DOWN: verticalSpeed =    this->secondaryShtFile->focusedAxisSpeed; break;
        case PLAYER_DIRECTION_UP_LEFT: horizontalSpeed = -this->secondaryShtFile->focusedDiagonalSpeed; verticalSpeed = horizontalSpeed; break;
        case PLAYER_DIRECTION_DOWN_LEFT: verticalSpeed =    this->secondaryShtFile->focusedDiagonalSpeed; horizontalSpeed = -verticalSpeed; break;
        case PLAYER_DIRECTION_UP_RIGHT: horizontalSpeed =  this->secondaryShtFile->focusedDiagonalSpeed; verticalSpeed = -horizontalSpeed; break;
        case PLAYER_DIRECTION_DOWN_RIGHT: horizontalSpeed =  this->secondaryShtFile->focusedDiagonalSpeed; verticalSpeed = horizontalSpeed; break;
        default: break;
        }
    }
    else
    {
        switch (this->movementDirection)
        {
        case PLAYER_DIRECTION_RIGHT: horizontalSpeed =  this->primaryShtFile->normalAxisSpeed; break;
        case PLAYER_DIRECTION_LEFT: horizontalSpeed = -this->primaryShtFile->normalAxisSpeed; break;
        case PLAYER_DIRECTION_UP: verticalSpeed =   -this->primaryShtFile->normalAxisSpeed; break;
        case PLAYER_DIRECTION_DOWN: verticalSpeed =    this->primaryShtFile->normalAxisSpeed; break;
        case PLAYER_DIRECTION_UP_LEFT: horizontalSpeed = -this->primaryShtFile->normalDiagonalSpeed; verticalSpeed = horizontalSpeed; break;
        case PLAYER_DIRECTION_DOWN_LEFT: verticalSpeed =    this->primaryShtFile->normalDiagonalSpeed; horizontalSpeed = -verticalSpeed; break;
        case PLAYER_DIRECTION_UP_RIGHT: horizontalSpeed =  this->primaryShtFile->normalDiagonalSpeed; verticalSpeed = -horizontalSpeed; break;
        case PLAYER_DIRECTION_DOWN_RIGHT: horizontalSpeed =  this->primaryShtFile->normalDiagonalSpeed; verticalSpeed = horizontalSpeed; break;
        default: break;
        }
    }

    horizontalSpeed *= this->horizontalSpeedMultiplier;
    verticalSpeed *= this->verticalSpeedMultiplier;

#define SET_PLAYER_SCRIPT(idx) (this->anmFile->SetAndExecuteScriptIdx(&this->mainVm, (idx)))
    if (g_GameManager.shotType < 4)
    {
        if (this->optionModeFlag == 0)
        {
            if (horizontalSpeed < 0.0f && this->currentHorizontalSpeed >= 0.0f)
                SET_PLAYER_SCRIPT(1);
            else if (horizontalSpeed == 0.0f && this->currentHorizontalSpeed < 0.0f)
                SET_PLAYER_SCRIPT(2);
            if (horizontalSpeed > 0.0f && this->currentHorizontalSpeed <= 0.0f)
                SET_PLAYER_SCRIPT(3);
            else if (horizontalSpeed == 0.0f && this->currentHorizontalSpeed > 0.0f)
                SET_PLAYER_SCRIPT(4);
        }
        else
        {
            if (horizontalSpeed < 0.0f && this->currentHorizontalSpeed >= 0.0f)
                SET_PLAYER_SCRIPT(6);
            else if (horizontalSpeed == 0.0f && this->currentHorizontalSpeed < 0.0f)
                SET_PLAYER_SCRIPT(7);
            if (horizontalSpeed > 0.0f && this->currentHorizontalSpeed <= 0.0f)
                SET_PLAYER_SCRIPT(8);
            else if (horizontalSpeed == 0.0f && this->currentHorizontalSpeed > 0.0f)
                SET_PLAYER_SCRIPT(9);
        }
    }
    else
    {
        if ((g_GameManager.shotType & 1) != 0)
        {
            if (horizontalSpeed < 0.0f && this->currentHorizontalSpeed >= 0.0f)
                SET_PLAYER_SCRIPT(6);
            else if (horizontalSpeed == 0.0f && this->currentHorizontalSpeed < 0.0f)
                SET_PLAYER_SCRIPT(7);
            if (horizontalSpeed > 0.0f && this->currentHorizontalSpeed <= 0.0f)
                SET_PLAYER_SCRIPT(8);
            else if (horizontalSpeed == 0.0f && this->currentHorizontalSpeed > 0.0f)
                SET_PLAYER_SCRIPT(9);
        }
        else
        {
            if (horizontalSpeed < 0.0f && this->currentHorizontalSpeed >= 0.0f)
                SET_PLAYER_SCRIPT(1);
            else if (horizontalSpeed == 0.0f && this->currentHorizontalSpeed < 0.0f)
                SET_PLAYER_SCRIPT(2);
            if (horizontalSpeed > 0.0f && this->currentHorizontalSpeed <= 0.0f)
                SET_PLAYER_SCRIPT(3);
            else if (horizontalSpeed == 0.0f && this->currentHorizontalSpeed > 0.0f)
                SET_PLAYER_SCRIPT(4);
        }
    }
#undef SET_PLAYER_SCRIPT

    this->currentHorizontalSpeed = horizontalSpeed;
    this->currentVerticalSpeed = verticalSpeed;
    this->velocity.x = horizontalSpeed * g_Supervisor.framerateMultiplier;
    this->velocity.y = verticalSpeed * g_Supervisor.framerateMultiplier;
    this->position.operator float *()[0] += this->velocity.x;
    this->position.operator float *()[1] += this->velocity.y;

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

    this->hurtboxBoundsMin = this->position - this->hurtboxHalfSize;
    this->hurtboxBoundsMax = this->position + this->hurtboxHalfSize;
    this->grazeBoundsMin = this->position - this->grazeHalfSize;
    this->grazeBoundsMax = this->position + this->grazeHalfSize;
    this->itemCollectionBoundsMin = this->position - this->itemCollectionHalfSize;
    this->itemCollectionBoundsMax = this->position + this->itemCollectionHalfSize;

    for (optionUpdateIndex = 0; optionUpdateIndex < 4; ++optionUpdateIndex)
    {
        if (this->optionStates[optionUpdateIndex].updateCallback != NULL)
        {
            this->optionStates[optionUpdateIndex].updateCallback(
                this, &this->optionStates[optionUpdateIndex]);
            g_AnmManager->ExecuteScript(&this->optionStates[optionUpdateIndex].vm);
            this->optionStates[optionUpdateIndex].timer++;
        }
    }

    if ((g_GuiMessageInputCurrent & 1) != 0 && !g_Gui.IsDialoguePresent() && !g_GameManager.IsTampered())
        this->StartShooting();

    if (!g_Gui.IsDialoguePresent() && this->focusTransitionFrames >= 30 &&
        this->bombState.isInUse == 0)
    {
        gaugeDelta = 0;
        if (this->shotTimer >= 0)
        {
            if (this->timerE2AD0 > 0)
                this->timerE2AD0--;
            else
            {
                gaugeDelta = (i32)((f32)this->timerE2AE8 > 300.0f
                                       ? 21.0f
                                       : (f32)this->timerE2AE8 / 15.0f);
                if (this->optionModeFlag == 0)
                    gaugeDelta = -gaugeDelta;
                g_GameManager.AddToYoukaiGauge((i32)((f32)gaugeDelta * g_Supervisor.framerateMultiplier), 0);
                this->timerE2AE8++;
            }
        }
        else
        {
            if (this->timerE2AD0 >= 4)
                this->timerE2AE8 = 0;
            if (this->timerE2AD0 >= 30)
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
                    g_GameManager.AddToYoukaiGauge((i32)((f32)gaugeDelta * g_Supervisor.framerateMultiplier), 0);
                }
            }
            else
                this->timerE2AD0++;
        }
    }

    if ((g_GameManager.GaugeIsExtremelyHuman() || g_GameManager.GaugeIsExtremelyYoukai()) &&
        this->extremeGaugeEffect == NULL)
    {
        this->extremeGaugeEffect = reinterpret_cast<Effect *>(
            g_EffectManager.FUN_00425870(25, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 8, 1, -1));
    }
    if (this->extremeGaugeEffect != NULL)
    {
        this->extremeGaugeEffect->position = this->position;
        if (!g_GameManager.GaugeIsExtremelyHuman() && !g_GameManager.GaugeIsExtremelyYoukai())
        {
            this->extremeGaugeEffect->active = 0;
            this->extremeGaugeEffect = NULL;
        }
    }

    if (verticalSpeed != 0.0f || horizontalSpeed != 0.0f)
    {
        for (historyIndex = 15; historyIndex > 0; --historyIndex)
            this->positionHistory[historyIndex] =
                this->positionHistory[historyIndex - 1];
        this->positionHistory[0] = this->position;
    }
    return 0;
}
// FUNCTION: th08 0x44c1b0
#pragma var_order(yDelta, xDelta, this)
f32 Player::AngleToPoint(Float3 *position)
{
    f32 yDelta;
    f32 xDelta;

    xDelta = this->position.operator float *()[0] - position->x;
    yDelta = this->position.operator float *()[1] - position->y;

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
    if (g_GameManager.scriptedUpdateFreeze != 0)
    {
        if (player->focusEffect != NULL)
        {
            player->focusEffect->vm.flagsWord |= 0x80000;
        }
        if (player->extremeGaugeEffect != NULL)
        {
            player->extremeGaugeEffect->vm.flagsWord |= 0x80000;
        }
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }
    if (player->focusEffect != NULL)
    {
        player->focusEffect->vm.flagsWord &= 0xfff7ffff;
    }
    if (player->extremeGaugeEffect != NULL)
    {
        player->extremeGaugeEffect->vm.flagsWord &= 0xfff7ffff;
    }
    player->UpdateCollisionRegions();
    player->UpdateBombState();
    if (player->playerState == PLAYER_STATE_DYING)
    {
        if (player->UpdateDeathAndRespawn() != 0)
        {
            goto updateD180;
        }
    }
    else if (player->playerState == PLAYER_STATE_SPAWNING)
    {
updateD180:
        player->UpdateRespawnAnimation();
    }
    player->UpdateInvulnerability();
    if (player->playerState != PLAYER_STATE_DYING && player->playerState != PLAYER_STATE_SPAWNING)
    {
        player->UpdateMovementAndOptions();
    }
    g_AnmManager->ExecuteScript(&player->mainVm);
    player->UpdateShots();
    player->UpdateShooting();
    player->UpdateGaugePosition();
    if (!g_Gui.IsDialoguePresent())
    {
        g_GameManager.runActiveFrames += 1;
        g_GameManager.stageActiveFrames += 1;
        if (g_GameManager.GaugeIsExtremelyHuman())
        {
            g_GameManager.runExtremeHumanFrames += 1;
            g_GameManager.stageExtremeHumanFrames += 1;
            g_GameManager.AddScore(100);
        }
        else if (g_GameManager.GaugeIsExtremelyYoukai())
        {
            g_GameManager.runExtremeYoukaiFrames += 1;
            g_GameManager.stageExtremeYoukaiFrames += 1;
            g_GameManager.AddScore(100);
        }
    }
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(index, slot)
// FUNCTION: th08 0x44c5b0
void Player::UpdateCollisionRegions()
{
    PlayerCollisionRegion *slot = this->damageRegions;
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
void Player::UpdateBombState()
{
    u32 i;
    i32 isForced;
    isForced = 0;
    if (this->forceDeathbombAtWindowEnd != 0 &&
        this->deathbombWindowFrames == 1)
    {
        isForced = 1;
        goto acceptBomb;
    }

    if (this->bombInputLockFrames != 0)
        --this->bombInputLockFrames;

    if (this->bombState.isInUse != 0)
    {
        if (this->bombState.timer.HasTicked())
            g_Gui.flags.pointDisplayUpdateFrames = 2;

        if (this->bombState.timer >= this->bombState.duration)
        {
            g_Spellcard.spellcard_fun_00416130();
            this->bombState.isInUse = 0;
            this->verticalSpeedMultiplier = 1.0f;
            this->horizontalSpeedMultiplier = 1.0f;

            if (this->bombState.callbackVariant == PLAYER_BOMB_CALLBACK_SPECIAL)
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
            this->bombState.updateCallbacks.callbacks[this->bombState.callbackVariant](this);
            this->bombState.timer++;
        }

        if (this->bombState.callbackVariant < PLAYER_BOMB_CALLBACK_SPECIAL)
        {
            if ((this->bombState.callbackVariant & 1) != 0)
                g_GameManager.AddToYoukaiGauge(26000 / this->bombState.duration, 1);
            else
                g_GameManager.AddToYoukaiGauge(-26000 / this->bombState.duration, 1);
        }
        return;
    }

    if ((g_GuiMessageInputCurrent & 2) != 0 && !g_GameManager.IsTampered() && !g_Gui.IsDialoguePresent() &&
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
    g_ReplayManager->flags |= 1;
    this->forceDeathbombAtWindowEnd = 0;
    if (((*reinterpret_cast<u32 *>(&g_GameManager.flags) >> 7) & 3) != 0)
    {
        this->bombState.callbackVariant = PLAYER_BOMB_CALLBACK_SPECIAL;
    }
    else
    {
        this->mainVm.flagsWord &= 0xFFFDFFFFu;
        if (this->deathbombEffectVm != NULL)
        {
            reinterpret_cast<Effect *>(this->deathbombEffectVm)->active = 0;
            this->deathbombEffectVm = NULL;
        }
        *reinterpret_cast<u32 *>(&g_GameManager.flags) &= 0xFFFFFBFFu;
        g_AnmManager->SetMixColorDefault();

        this->bombState.callbackVariant = this->optionModeFlag;
        if (this->deathbombPending)
            this->bombState.callbackVariant = 1 - this->bombState.callbackVariant;

        if (this->deathbombPending)
        {
            this->bombState.callbackVariant += 2;
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

    this->deathbombPending = 0;
    g_Gui.flags.bombDisplayUpdateFrames = 2;
    this->bombState.isInUse = 1;
    this->itemTimeOrbMode = 1;
    this->bombState.timer = 0;
    this->bombState.duration = 999;

    {
        this->bombState.updateCallbacks.callbacks[this->bombState.callbackVariant](this);
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
        this->itemTimeOrbMode = 0;
    }

done:
    return;
}
// FUNCTION: th08 0x44cbf0
#pragma var_order(value, this)
i32 Player::UpdateDeathAndRespawn()
{
    f32 value;

    if (this->deathbombWindowFrames != 0)
    {
        g_GameManager.AddTimeOrbs(-15);
        --this->deathbombWindowFrames;
        this->deathbombPending = 1;
        if (this->deathbombWindowFrames == 0)
        {
            if (this->deathbombEffectVm != NULL)
            {
                reinterpret_cast<Effect *>(this->deathbombEffectVm)->active = 0;
                this->deathbombEffectVm = NULL;
            }
            g_EffectManager.FUN_00425870(12, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 3, 1, 0xFF4040FF);
            g_EffectManager.SpawnEffect(6, reinterpret_cast<D3DXVECTOR3 *>(&this->position), 16, -1);
            g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(15), this->position.x);
            *reinterpret_cast<u32 *>(&g_GameManager.flags) &= ~0x400u;
            g_AnmManager->SetMixColorDefault();
            this->mainVm.flagsWord &= ~0x20000u;
            g_ReplayManager->flags |= 4;
            g_GameManager.character = 0;
            this->deathbombPending = 0;
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
    this->mainVm.scale.y = 3.0f * value + 1.0f;
    this->mainVm.scale.x = 1.0f - 1.0f * value;
    this->mainVm.color1.d3dColor =
        ((i32)(255.0f - (f32)this->timer * 255.0f / 30.0f) << 24) | 0xFFFFFF;
    reinterpret_cast<AnmVmBase *>(&this->mainVm)->SetBlendModeAdditive();
    this->currentHorizontalSpeed = 0;
    this->currentVerticalSpeed = 0;

    if ((i32)this->timer >= 30)
    {
        this->playerState = PLAYER_STATE_SPAWNING;
        this->position.operator float *()[0] = g_GameManager.arcadeRegionSize.x / 2.0f;
        this->position.operator float *()[1] = g_GameManager.arcadeRegionSize.y - 64.0f;
        this->position.operator float *()[2] = 0.2f;
        this->timer = 0;
        this->mainVm.scale.x = 3.0f;
        this->mainVm.scale.y = 3.0f;
        if ((g_GameManager.shotType < 4 && this->optionModeFlag == 0) ||
            (g_GameManager.shotType & 1) == 0)
            this->anmFile->SetAndExecuteScriptIdx(&this->mainVm, 0);
        else
            this->anmFile->SetAndExecuteScriptIdx(&this->mainVm, 5);

        if (g_GameManager.GetLives() <= 0)
        {
            g_GameManager.showRetryMenu = 1;
        }
        else
        {
            g_GameManager.AddLives(-1);
            g_Gui.flags.lifeDisplayUpdateFrames = 2;
            g_GameManager.SetBombCount((i32)g_Player.primaryShtFile->initialBombCount);
            g_Gui.flags.bombDisplayUpdateFrames = 2;
            return 1;
        }
    }
    }
    return 0;
}
// FUNCTION: th08 0x44d180
#pragma var_order(value, this)
void Player::UpdateRespawnAnimation()
{
    f32 value;

    this->playerStateSlotCooldown = 60;
    value = 1.0f - (f32)this->timer / 60.0f;
    this->mainVm.scale.y = 2.0f * value + 1.0f;
    this->mainVm.scale.x = 1.0f - 1.0f * value;
    reinterpret_cast<AnmVmBase *>(&this->mainVm)->SetBlendModeAdditive();
    this->verticalSpeedMultiplier = 1.0f;
    this->horizontalSpeedMultiplier = 1.0f;
    this->mainVm.color1.d3dColor =
        (((i32)this->timer * 0xFF) / 30 << 24) | 0xFFFFFF;
    this->deathbombWindowFrames = 0;

    if ((i32)this->timer >= 30)
    {
        this->playerState = PLAYER_STATE_INVULNERABLE;
        this->mainVm.scale.x = 1.0f;
        this->mainVm.scale.y = 1.0f;
        this->mainVm.color1.d3dColor = 0xFFFFFFFF;
        reinterpret_cast<AnmVmBase *>(&this->mainVm)->SetBlendModeNormal();
        if (!g_GameManager.flags.isSpellPractice)
        {
            this->timer = 240;
        }
        this->deathbombWindowFrames = g_Player.primaryShtFile->deathbombWindowFrames;
    }
}
// FUNCTION: th08 0x44d2c0
void Player::UpdateInvulnerability()
{
    if (this->playerStateSlotCooldown != 0)
    {
        this->playerStateSlotCooldown--;
        this->CreateRectCancelRegion(&this->position, 768.0f, 896.0f, -1, 0);
    }

    if (this->playerState == PLAYER_STATE_INVULNERABLE)
    {
        this->deathbombPending = false;

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
void Player::UpdateGaugePosition()
{
    this->tailPosition0 = Float3(-999.0f, -999.0f, 0.0f);
    this->tailPosition1 = Float3(-999.0f, -999.0f, 0.0f);
    this->enemyTrackedPositionValid = 0;

    if (this->position.y >= 400.0f)
    {
        if (g_AsciiManager.GetGaugeInterrupt() != 2)
        {
            if (this->position.x < 160.0f)
            {
                g_AsciiManager.SetGaugeInterrupt(2);
                goto doneTop;
            }
        }

        if (g_AsciiManager.GetGaugeInterrupt() == 2)
        {
            if (this->position.x > 160.0f)
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

    player->DrawActiveShots();

    if (player->bombState.isInUse != 0)
    {
        player->bombState.drawCallbacks.callbacks[player->bombState.callbackVariant](player);
    }

    if (!g_GameManager.showRetryMenu)
    {
        player->mainVm.pos.x =
            g_GameManager.arcadeRegionTopLeftPos.x + player->position.x;
        player->mainVm.pos.y =
            g_GameManager.arcadeRegionTopLeftPos.y + player->position.y;
        player->mainVm.pos.z = 0.1f;
        g_AnmManager->DrawNoRotation(&player->mainVm);
    }

    for (i = 0; i < 4; i++)
    {
        if (player->optionStates[i].renderCallback != NULL)
        {
            player->optionStates[i].renderCallback(player, &player->optionStates[i]);
        }
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x44d630
ChainCallbackResult Player::OnDrawLowPrio(Player *player)
{
    player->DrawHitShots();
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x44d650
#pragma var_order(i, shotSlot, option, m, player)
ZunResult Player::AddedCallback(Player *player)
{
    u32 i;
    PlayerShot *shotSlot;
    PlayerOptionState *option;
    u32 m;

    if (IsResourceReloadEnabled())
    {
        if (Player::LoadShtFile(&player->primaryShtFile, g_Player1ShtFiles[g_GameManager.shotType]) != ZUN_SUCCESS)
            return ZUN_ERROR;
        if (Player::LoadShtFile(&player->secondaryShtFile, g_Player2ShtFile[g_GameManager.shotType]) != ZUN_SUCCESS)
            return ZUN_ERROR;
        player->anmFile = g_AnmManager->PreloadAnm(5, g_PlayerAnmFilenames[g_GameManager.shotType]);
        if (player->anmFile == NULL)
            return ZUN_ERROR;
    }
    else
    {
        player->anmFile = g_AnmManager->GetAnm(5);
    }

    if (g_GameManager.shotType < 4 || (g_GameManager.shotType & 1) == 0)
        player->anmFile->SetAndExecuteScriptIdx(&player->mainVm, 0);
    else
        player->anmFile->SetAndExecuteScriptIdx(&player->mainVm, 5);

    player->position.operator float *()[0] = g_GameManager.arcadeRegionSize.x / 2.0f;
    player->position.operator float *()[1] = g_GameManager.arcadeRegionSize.y - 64.0f;
    player->position.operator float *()[2] = 0.49f;

    for (i = 0; i < 0x180; ++i)
        reinterpret_cast<PlayerCollisionRegion *>(player->damageRegions)[i].Reset();

    player->hurtboxHalfSize.y = g_Player.primaryShtFile->hurtboxSize / 2.0f;
    player->hurtboxHalfSize.x = player->hurtboxHalfSize.y;
    player->hurtboxHalfSize.z = 5.0f;
    player->grazeHalfSize.y = g_Player.primaryShtFile->grazeBoxSize / 2.0f;
    player->grazeHalfSize.x = player->grazeHalfSize.y;
    player->grazeHalfSize.z = 5.0f;
    player->itemCollectionHalfSize.y = g_Player.primaryShtFile->itemCollectionBoxSize / 2.0f;
    player->itemCollectionHalfSize.x = player->itemCollectionHalfSize.y;
    player->itemCollectionHalfSize.z = 5.0f;

    player->movementDirection = PLAYER_DIRECTION_NONE;
    player->playerState = PLAYER_STATE_SPAWNING;
    player->timer = g_GameManager.IsSpellPractice() ? 10 : 120;
    player->unknown2 = 1;

    shotSlot = player->shots;
    for (i = 0; (i32)i < 0x80; ++i, shotSlot++)
        shotSlot->state = PLAYER_SHOT_INACTIVE;

    player->shotTimer = -1;
    player->timerE2AD0 = 0;
    player->timerE2AE8 = 0;

    player->bombState.updateCallbacks = g_PlayerBombCallbacksByShotType[g_GameManager.shotType * 2];
    player->bombState.drawCallbacks = g_PlayerBombCallbacksByShotType[g_GameManager.shotType * 2 + 1];

    player->bombState.isInUse = 0;
    player->baseShotAngle = -ZUN_PI / 2.0f;
    player->verticalSpeedMultiplier = 1.0f;
    player->horizontalSpeedMultiplier = 1.0f;
    player->deathbombWindowFrames = g_Player.primaryShtFile->deathbombWindowFrames;

    if (IsResourceReloadEnabled())
        g_AsciiManager.SetGaugeInterrupt(1);
    g_AsciiManager.SetBossMarkerInterrupt(0, 2);
    g_AsciiManager.SetBossMarkerInterrupt(1, 2);
    g_AsciiManager.SetBossMarkerInterrupt(2, 2);

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

    player->extremeGaugeEffect = NULL;
    for (i = 0; i < 16; ++i)
        player->positionHistory[i] = player->position;
    player->optionModeFlag = 2;

    if (g_GameManager.shotType > 3)
    {
        option = player->optionStates;
        for (m = 0; m < 4; ++m, option++)
        {
            memset(option, 0, 0x2F4);
            option->updateCallback =
                g_PlayerOptionUpdateCallbacks[g_GameManager.shotType].callbacks[m];
            option->renderCallback =
                g_PlayerOptionRenderCallbacks[g_GameManager.shotType].callbacks[m];
            if (option->updateCallback != NULL)
            {
                option->state2C8 = 1;
                option->timer = 0;
                option->optionIndex = m;
            }
            else
            {
                option->state2C8 = 0;
            }
        }
    }

    if (g_GameManager.IsSoloHuman())
        player->damageAccumulatorThreshold = 27;
    else
        player->damageAccumulatorThreshold = 40;
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
        g_AsciiManager.SetBossMarkerInterrupt(0, 99);
        g_AsciiManager.SetBossMarkerInterrupt(1, 99);
        g_AsciiManager.SetBossMarkerInterrupt(2, 99);

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
#pragma var_order(i, descriptor, header, path)
ZunResult Player::LoadShtFile(PlayerRawShtFile **header, const char *path)
{
    i32 i;
    PlayerShotDescriptor *descriptor;

    *header = reinterpret_cast<PlayerRawShtFile *>(FileSystem::OpenFile(path, NULL, 0));
    if (*header == NULL)
    {
        return ZUN_ERROR;
    }

    for (i = 0; i < (*header)->shotPowerLevelCount; i++)
    {
        reinterpret_cast<u32 &>((*header)->shotPowerLevels[i].descriptors) +=
            reinterpret_cast<u32>(*header);
        descriptor = (*header)->shotPowerLevels[i].descriptors;

        while (descriptor->fireInterval >= 0)
        {
            descriptor->spawnCallback =
                g_PlayerShotSpawnCallbacks[reinterpret_cast<u32>(descriptor->spawnCallback)];
            descriptor->updateCallback =
                g_PlayerShotUpdateCallbacks[reinterpret_cast<u32>(descriptor->updateCallback)];
            descriptor->drawCallback =
                g_PlayerShotDrawCallbacks[reinterpret_cast<u32>(descriptor->drawCallback)];

            descriptor->collisionCallback = reinterpret_cast<PlayerShotCollisionCallback>(
                g_PlayerShotCollisionOrDifficultyTable[
                    reinterpret_cast<u32>(descriptor->collisionCallback)]);
            descriptor++;
        }
    }

    return ZUN_SUCCESS;
}

#pragma var_order(slot, index)
// FUNCTION: th08 0x44de60
PlayerCollisionRegion *Player::CreateRectCancelRegion(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerCollisionRegion *slot = this->cancelRegions;
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
PlayerCollisionRegion *Player::CreateCircleCancelRegion(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerCollisionRegion *slot = this->cancelRegions;
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
PlayerCollisionRegion *Player::CreateRectDamageRegion(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerCollisionRegion *slot = this->damageRegions;
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
PlayerCollisionRegion *Player::CreateCircleDamageRegion(const Float3 *center, f32 value1, f32 value2, i32 value3, i32 value4)
{
    PlayerCollisionRegion *slot = this->damageRegions;
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
    this->hscr.numDeaths += 1;
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
void PlayerCollisionRegion::Deactivate()
{
    this->active = false;
}

// FUNCTION: th08 0x44e370
void PlayerCollisionRegion::Reset()
{
    memset(this, 0, sizeof(*this));
    this->collisionInterval = 1;
}

void __fastcall UpdateOptionHomingToPlayer(Player *player, PlayerOptionState *option);
void __fastcall UpdateOptionHomingToTarget(Player *player, PlayerOptionState *option);

// FUNCTION: th08 0x44e3a0
i32 __fastcall UpdateHomingOption(Player *player, PlayerOptionState *option)
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
            UpdateOptionHomingToPlayer(player, option);
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
            UpdateOptionHomingToPlayer(player, option);
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
            UpdateOptionHomingToPlayer(player, option);
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
                UpdateOptionHomingToTarget(player, option);
            if (((player->shotTimer < 0) && ((g_CurFrameInput & 1) == 0)) ||
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
void __fastcall UpdateOptionHomingToPlayer(Player *player, PlayerOptionState *option)
{
    Float3 target;
    Float3 delta;

    target = player->position;
    target.y -= 96.0f;
    if (target.y < 32.0f)
        target.y = 32.0f;

    delta = target - option->position;
    delta /= 16.0f;
    option->velocity += (delta - option->velocity) * 0.2f;
    option->position += option->velocity;

    if (fabsf(option->velocity.x) < 0.05f)
        option->velocity.x = 0.0f;

    if (player->shotTimer >= 0 &&
        player->optionHomingTarget != NULL &&
        option->timer >= 10)
    {
        option->vm.SetInterrupt(3);
        option->substate2CC = 3;
    }
    else
    {
        player->optionHomingTarget = NULL;
    }
}

// FUNCTION: th08 0x44e8d0
#pragma var_order(delta, target, player, option)
void __fastcall UpdateOptionHomingToTarget(Player *player, PlayerOptionState *option)
{
    Float3 target;
    Float3 delta;

    target = player->optionHomingTarget->worldPosition;
    target.y += 32.0f;
    if (target.y < 32.0f)
        target.y = 32.0f;

    delta = target - option->position;
    delta /= 16.0f;
    option->velocity += (delta - option->velocity) * 0.2f;
    option->position += option->velocity;

    if (fabsf(option->velocity.x) < 0.05f)
        option->velocity.x = 0.0f;
}

// FUNCTION: th08 0x44e9e0
i32 __fastcall DrawPlayerOption(Player *, PlayerOptionState *option)
{
    option->vm.pos.x = g_GameManager.arcadeRegionTopLeftPos.x + option->position.x;
    option->vm.pos.y = g_GameManager.arcadeRegionTopLeftPos.y + option->position.y;
    option->vm.pos.z = 0.49f;
    g_AnmManager->Draw2D(&option->vm);
    return 0;
}
// FUNCTION: th08 0x44ea40
i32 __fastcall UpdateBombAnchorOption(Player *player, PlayerOptionState *option)
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
i32 __fastcall UpdateOrbitingOption(Player *player, PlayerOptionState *option)
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
i32 __fastcall UpdateModeSensitiveOrbitingOption(Player *player, PlayerOptionState *option)
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
i32 __fastcall UpdateFacingTrailOption(Player *player, PlayerOptionState *option)
{
    f32 targetAngle;
    f32 angleDifference;

    switch (option->state2C8)
    {
    case 1:
        player->anmFile->SetAndExecuteScriptIdx(&option->vm, 21);
        option->state2C8 = 2;
        option->target = player->positionHistory[15];
        option->orbitAngle = 0.0f;
        option->facingAngle = -ZUN_PI / 2.0f;
        // Fall through into the normal update.
    case 2:
        option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.052359879016876221f);
        option->position.FromAngleMagnitude(option->orbitAngle, 8.0f);
        option->target = (player->positionHistory[15] - option->target) * 0.05f + option->target;
        option->position += option->target;
        option->position.z = 0.0f;
        g_EffectManager.SpawnEffect(
            47, reinterpret_cast<D3DXVECTOR3 *>(&option->position), 1, 0x80405080);

        switch (player->movementDirection)
        {
        case PLAYER_DIRECTION_NONE:
            goto optionUpdateDone;
        case PLAYER_DIRECTION_UP:
            targetAngle = ZUN_PI / 2.0f;
            break;
        case PLAYER_DIRECTION_DOWN:
            targetAngle = -ZUN_PI / 2.0f;
            break;
        case PLAYER_DIRECTION_LEFT:
            targetAngle = 0.0f;
            break;
        case PLAYER_DIRECTION_RIGHT:
            targetAngle = ZUN_PI;
            break;
        case PLAYER_DIRECTION_UP_LEFT:
            targetAngle = ZUN_PI / 4.0f;
            break;
        case PLAYER_DIRECTION_UP_RIGHT:
            targetAngle = 3.0f * ZUN_PI / 4.0f;
            break;
        case PLAYER_DIRECTION_DOWN_LEFT:
            targetAngle = -ZUN_PI / 4.0f;
            break;
        case PLAYER_DIRECTION_DOWN_RIGHT:
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
i32 __fastcall UpdateModeSensitiveFacingOption(Player *player, PlayerOptionState *option)
{
    f32 targetAngle;
    f32 angleDifference;

    switch (option->state2C8)
    {
    case 1:
        player->anmFile->SetAndExecuteScriptIdx(&option->vm, 21);
        option->state2C8 = 2;
        option->target = player->positionHistory[15];
        option->orbitAngle = 0.0f;
        option->facingAngle = -ZUN_PI / 2.0f;
        // Fall through into the normal update.
    case 2:
        option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.052359879016876221f);
        option->position.FromAngleMagnitude(option->orbitAngle, 8.0f);
        option->target = (player->positionHistory[15] - option->target) * 0.05f + option->target;
        option->position += option->target;
        option->position.z = 0.0f;
        option->vm.color1.d3dColor = 0xFFFF8080;

        if (player->optionModeFlag == 0)
        {
            option->vm.color1.d3dColor = 0xFFFFFFFF;
            switch (player->movementDirection)
            {
            case PLAYER_DIRECTION_NONE:
                goto optionUpdateDone;
            case PLAYER_DIRECTION_UP:
                targetAngle = ZUN_PI / 2.0f;
                break;
            case PLAYER_DIRECTION_DOWN:
                targetAngle = -ZUN_PI / 2.0f;
                break;
            case PLAYER_DIRECTION_LEFT:
                targetAngle = 0.0f;
                break;
            case PLAYER_DIRECTION_RIGHT:
                targetAngle = ZUN_PI;
                break;
            case PLAYER_DIRECTION_UP_LEFT:
                targetAngle = ZUN_PI / 4.0f;
                break;
            case PLAYER_DIRECTION_UP_RIGHT:
                targetAngle = 3.0f * ZUN_PI / 4.0f;
                break;
            case PLAYER_DIRECTION_DOWN_LEFT:
                targetAngle = -ZUN_PI / 4.0f;
                break;
            case PLAYER_DIRECTION_DOWN_RIGHT:
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
i32 __fastcall UpdateTwinOrbitingOption(Player *player, PlayerOptionState *option)
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
void __fastcall Player::InitializeShot(PlayerShot *slot, PlayerShotDescriptor *entry)
{
    if (entry->sourceOptionIndex == 0)
    {
        slot->position = this->position;
    }
    else
    {
        slot->position = this->optionStates[entry->sourceOptionIndex - 1].position;
    }

    slot->position.operator float *()[0] += entry->positionOffset.x;
    slot->position.operator float *()[1] += entry->positionOffset.y;
    slot->position.operator float *()[2] = 0.495f;

    slot->hitboxSize.x = entry->hitboxSize.x;
    slot->hitboxSize.y = entry->hitboxSize.y;
    slot->hitboxSize.z = 1.0f;
    slot->angle = entry->angle;
    slot->speed = entry->speed;
    slot->velocity.x = cosf(entry->angle) * entry->speed;
    slot->velocity.y = sinf(entry->angle) * entry->speed;

    slot->timer = 0;
    slot->optionModeFlag = this->optionModeFlag;
    slot->shotType = entry->shotType;
    slot->damage = entry->damage;
    slot->animationIndex = entry->animationIndex;

    if (entry->soundIndex >= 0)
    {
        g_SoundPlayer.PlaySoundPositionedByIdx(static_cast<SoundIdx>(entry->soundIndex),
                                               this->position.x);
    }

    this->anmFile->SetAndExecuteScriptIdx(&slot->vm, entry->animationIndex + 10);

    slot->tintInExtremeYoukai = 0;
    if (g_GameManager.GaugeIsExtremelyYoukai())
    {
        if (entry->extremeGaugeBehavior > 0)
        {
            slot->tintInExtremeYoukai = 1;
        }
    }
}

// FUNCTION: th08 0x44fd80
#pragma var_order(slot, this)
i32 __fastcall Player::SpawnShotOnSchedule(PlayerShot *slot, i32 value,
                                           PlayerShotDescriptor *entry)
{
    if (value % entry->fireInterval == entry->fireFrame)
    {
        this->InitializeShot(slot, entry);
        return 1;
    }

    return 0;
}


// FUNCTION: th08 0x44fdd0
#pragma var_order(slot, this)
i32 __fastcall Player::SpawnShotOnScheduleUnlessBombing(PlayerShot *slot, i32 value,
                                                        PlayerShotDescriptor *entry)
{
    if (this->bombState.isInUse == 0 &&
        value % entry->fireInterval == entry->fireFrame)
    {
        this->InitializeShot(slot, entry);
        return 1;
    }

    return 0;
}

// FUNCTION: th08 0x44fe20
#pragma var_order(index, i, this, slot)
i32 __fastcall Player::SpawnPersistentShot(PlayerShot *slot, i32 value,
                                           PlayerShotDescriptor *entry)
{
    i32 index;
    i32 i;

    index = entry->fireFrame;
    if (this->bombState.isInUse != 0)
    {
        return 0;
    }
    if (g_GameManager.flags.suppressPlayerShots)
    {
        return 0;
    }

    if (this->timelines[index].instruction != NULL)
    {
        if (this->persistentShotDescriptors[index] != entry)
        {
            reinterpret_cast<PlayerShot *>(this->timelines[index].instruction)->vm.pendingInterrupt = 1;
            this->timelines[index].instruction = NULL;
        }
        return 0;
    }

    this->timelines[index].timer = 999;
    this->timelines[index].instruction = reinterpret_cast<EclTimelineInstruction *>(slot);
    slot->timelineIndex = static_cast<i16>(index);
    slot->sourceOptionIndex = entry->sourceOptionIndex;
    slot->velocity.z = entry->positionOffset.x;
    slot->auxiliaryValue = entry->positionOffset.y;
    slot->trailSegmentCount = entry->fireInterval;
    this->InitializeShot(slot, entry);

    for (i = 31; i >= 0; i--)
    {
        *reinterpret_cast<u32 *>(&slot->positionHistory[i].x) = 0xC479C000;
    }
    *reinterpret_cast<u32 *>(&slot->position.x) = 0xC479C000;
    this->persistentShotDescriptors[index] = entry;

    return 1;
}

// FUNCTION: th08 0x44ffa0
#pragma var_order(magnitude, angle, this, slot)
i32 __fastcall Player::SpawnShotAimedAtTrackedPoint(PlayerShot *slot, i32 value,
                                                    PlayerShotDescriptor *entry)
{
    f32 angle;
    f32 magnitude;

    if (value % entry->fireInterval == entry->fireFrame)
    {
        this->InitializeShot(slot, entry);
        if (this->tailPosition1.x > -100.0f)
        {
            angle = AddNormalizeAngle(
                VectorAngle(this->tailPosition1.y - slot->position.y,
                            this->tailPosition1.x - slot->position.x),
                entry->angle + ZUN_PI / 2.0f);
            magnitude = entry->speed * 1.5f;
            reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, magnitude);
            *reinterpret_cast<u32 *>(&slot->angle) = *reinterpret_cast<u32 *>(&angle);
        }
        return 1;
    }

    return 0;
}

// FUNCTION: th08 0x00450080
#pragma var_order(magnitude, angle)
i32 __fastcall SpawnShotAlongPlayerAngle(Player *player, PlayerShot *slot, i32 value,
                                         PlayerShotDescriptor *entry)
{
    f32 angle;
    f32 magnitude;

    if (value % entry->fireInterval == entry->fireFrame)
    {
        player->InitializeShot(slot, entry);
        angle = AddNormalizeAngle(
            player->baseShotAngle, entry->angle + ZUN_PI / 2.0f);
        magnitude = entry->speed;
        reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, magnitude);
        slot->angle = angle;
        return 1;
    }
    return 0;
}

// FUNCTION: th08 0x00450110
#pragma var_order(magnitude, angle)
i32 __fastcall SpawnShotAlongOptionAngle(Player *player, PlayerShot *slot, i32 value,
                                         PlayerShotDescriptor *entry)
{
    f32 angle;
    f32 magnitude;

    if (player->bombState.isInUse == 0 &&
        value % entry->fireInterval == entry->fireFrame)
    {
        player->InitializeShot(slot, entry);
        angle = AddNormalizeAngle(player->optionStates[2].facingAngle, entry->angle);
        magnitude = entry->speed;
        reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, magnitude);
        slot->angle = angle;
        return 1;
    }
    return 0;
}

// FUNCTION: th08 0x004501b0
i32 __fastcall SpawnRandomizedShot(Player *player, PlayerShot *slot, i32 value,
                                   PlayerShotDescriptor *entry)
{
    if (value % entry->fireInterval == entry->fireFrame)
    {
        player->InitializeShot(slot, entry);
        slot->angle = g_Rng.GetRandomF32() * ZUN_PI / 48.0f - ZUN_PI / 2.0f;
        reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(slot->angle, entry->speed);
        return 1;
    }
    return 0;
}

// FUNCTION: th08 0x00450240
#pragma var_order(magnitude, angle)
i32 __fastcall SpawnHomingShot(Player *player, PlayerShot *slot, i32 value,
                               PlayerShotDescriptor *entry)
{
    f32 angle;
    f32 magnitude;

    if (value % entry->fireInterval == entry->fireFrame)
    {
        player->InitializeShot(slot, entry);
        if (player->optionHomingTarget != NULL)
        {
            angle = AddNormalizeAngle(
                VectorAngle(player->optionHomingTarget->position.y - slot->position.y,
                            player->optionHomingTarget->position.x - slot->position.x),
                entry->angle + ZUN_PI / 2.0f);
            magnitude = entry->speed * 1.5f;
            reinterpret_cast<Float3 *>(&slot->velocity)->FromAngleMagnitude(angle, magnitude);
            slot->angle = angle;
        }
        return 1;
    }
    return 0;
}

// FUNCTION: th08 0x00450320
#pragma var_order(yDelta, xDelta, magnitude)
i32 __fastcall UpdateHomingShot(Player *player, PlayerShot *slot)
{
    f32 xDelta;
    f32 yDelta;
    f32 magnitude;
    if (slot->state == 1)
    {
        if (player->tailPosition0.x > -100.0f && (i32)slot->timer < 40 && slot->timer.HasTicked())
        {
            xDelta = player->tailPosition0.x - slot->position.operator float *()[0];
            yDelta = player->tailPosition0.y - slot->position.operator float *()[1];
            magnitude = sqrtf(xDelta * xDelta + yDelta * yDelta) / (slot->speed / 4.0f);
            if (magnitude < 1.0f) magnitude = 1.0f;
            xDelta = xDelta / magnitude + slot->velocity.x;
            yDelta = yDelta / magnitude + slot->velocity.y;
            magnitude = sqrtf(xDelta * xDelta + yDelta * yDelta);
            slot->speed = magnitude > 10.0f ? 10.0f : magnitude;
            if (slot->speed < 1.0f) slot->speed = 1.0f;
            slot->velocity.x = xDelta * slot->speed / magnitude;
            slot->velocity.y = yDelta * slot->speed / magnitude;
        }
        else if (slot->speed < 10.0f)
        {
            slot->speed += 1.0f / 3.0f;
            xDelta = slot->velocity.x;
            yDelta = slot->velocity.y;
            magnitude = sqrtf(xDelta * xDelta + yDelta * yDelta);
            slot->velocity.x = xDelta * slot->speed / magnitude;
            slot->velocity.y = yDelta * slot->speed / magnitude;
        }
    }
    slot->angle = VectorAngle(slot->velocity.y, slot->velocity.x);
    return 0;
}

// FUNCTION: th08 0x00450580
i32 __fastcall UpdateFallingShot(Player *player, PlayerShot *slot)
{
    if (slot->state == 1)
        slot->velocity.y -= g_Rng.GetRandomF32InRange(0.1f) + 0.27f;
    return 0;
}

// FUNCTION: th08 0x004505d0
i32 __fastcall UpdatePersistentShot(Player *player, PlayerShot *slot)
{
    if (player->timelines[slot->timelineIndex].instruction !=
        reinterpret_cast<EclTimelineInstruction *>(slot))
    {
        if (slot->vm.FUN_004396f8()) slot->vm.pendingInterrupt = 1;
    }
    if (g_Gui.IsDialoguePresent() || player->bombState.isInUse != 0 || g_GameManager.flags.suppressPlayerShots)
    {
        if ((i32)player->timelines[slot->timelineIndex].timer > 20)
            player->timelines[slot->timelineIndex].timer = 20;
    }
    if (player->timelines[slot->timelineIndex].timer <= 0)
    {
        player->timelines[slot->timelineIndex].timer = 0;
        player->timelines[slot->timelineIndex].instruction = NULL;
        slot->state = 0;
        return 1;
    }
    if (player->timelines[slot->timelineIndex].timer <= 70)
    {
        if (slot->vm.FUN_004396f8()) slot->vm.pendingInterrupt = 1;
    }
    slot->position.x += slot->velocity.z;
    slot->position.z = 0.44f;
    if (player->playerState == PLAYER_STATE_DYING) return 1;
    slot->vm.scale.y = slot->position.y / 14.0f;
    slot->hitboxSize.y = slot->position.y;
    slot->position.y /= 2.0f;
    if (player->timelines[slot->timelineIndex].timer < 100)
        player->timelines[slot->timelineIndex].timer--;
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
i32 __fastcall UpdateShotTrail(Player *player, PlayerShot *slot)
{
    PlayerCollisionRegion *damageSlot;
    i32 i;
    if (player->timelines[slot->timelineIndex].instruction !=
            reinterpret_cast<EclTimelineInstruction *>(slot) ||
        g_Gui.IsDialoguePresent() ||
        (i32)player->shotTimer < 0 ||
        player->playerState == PLAYER_STATE_DYING ||
        player->bombState.isInUse != 0 ||
        g_GameManager.flags.suppressPlayerShots)
    {
        slot->vm.pendingInterrupt = 1;
        player->timelines[slot->timelineIndex].instruction = NULL;
        slot->updateCallback = NULL;
    }
    if (player->optionStates[0].state2C8 == 0)
    {
        player->timelines[slot->timelineIndex].instruction = NULL;
        return 1;
    }
    for (i = 0; i < slot->trailSegmentCount; i++)
    {
        if (slot->positionHistory[i * 2].x >= -900.0f)
        {
            damageSlot = player->CreateRectDamageRegion(&slot->positionHistory[i * 2], 16.0f, 448.0f, 1, 0);
            damageSlot->mode = 1;
        }
    }
    for (i = 31; i > 0; i--)
    {
        slot->positionHistory[i] = slot->positionHistory[i - 1];
        slot->positionHistory[i].y -= 1.0f;
    }
    slot->positionHistory[0] = slot->position;
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
i32 __fastcall DrawShotTrail(Player *player, PlayerShot *slot)
{
    i32 color;
    i32 i;
    i32 originalColor;

    color = slot->vm.color1.a;
    originalColor = color;
    color = color * 3 / 4;
    for (i = 0; i < slot->trailSegmentCount * 2; i += 2)
    {
        if (slot->positionHistory[i].x == -999.0f)
            break;
        slot->vm.pos.x = slot->positionHistory[i].x;
        slot->vm.pos.y = slot->positionHistory[i].y;
        slot->vm.pos.z = slot->positionHistory[i].z;
        if (i != 0)
            slot->vm.color1.a = color - ((color / 2) * i) / slot->trailSegmentCount;
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
i32 __fastcall ApplyShotHitBehavior(Player *player, PlayerShot *slot, Float3 *effectPosition)
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
i32 __fastcall SpawnPeriodicShotHitEffect(Player *player, PlayerShot *slot,
                                          Float3 *effectPosition)
{
    player->shotHitEffectCounter++;
    if (player->shotHitEffectCounter % 8 == 0)
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
void __fastcall Player::SpawnShots(i32 value)
{
    PlayerShotPowerLevel *table;
    PlayerShotDescriptor *entry;
    PlayerShot *slot;
    i32 result;
    i32 i;

    table = (this->optionModeFlag == 0)
                ? this->primaryShtFile->shotPowerLevels
                : this->secondaryShtFile->shotPowerLevels;

    if (this->bombState.isInUse != 0 &&
        ((g_GameManager.shotType == 2 &&
          (this->bombState.callbackVariant & 1) != 0) ||
         g_GameManager.shotType == 9) &&
        this->bombState.timer >= 60)
    {
        table += ((this->bombState.callbackVariant & 2) ? 7 : 6);
    }
    else
    {
        while (g_GameManager.GetPower() >= table->minimumPower)
        {
            table++;
        }
    }

    entry = table->descriptors;
    slot = this->shots;
    for (i = 0; i < ARRAY_SIZE_SIGNED(this->shots); i++, slot++)
    {
        if (slot->state != PLAYER_SHOT_INACTIVE)
        {
            continue;
        }

processEntry:
        if (entry->spawnCallback != NULL)
        {
            result = entry->spawnCallback(this, slot, value, entry);
        }
        else
        {
            result = this->SpawnShotOnSchedule(slot, value, entry);
        }

        if (result == 1)
        {
            slot->vm.zWriteDisabled = 1;
            slot->state = PLAYER_SHOT_ACTIVE;
            slot->descriptor = entry;
            slot->updateCallback = slot->descriptor->updateCallback;
            slot->drawCallback = slot->descriptor->drawCallback;
            slot->collisionCallback = slot->descriptor->collisionCallback;
        }

        entry++;
        if (entry->fireInterval < 0)
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
void Player::UpdateShots()
{
    PlayerShot *slot;
    i32 i;

    if (g_GameManager.flags.deathbombFreezeActive)
    {
        return;
    }

    slot = this->shots;
    for (i = 0; i < ARRAY_SIZE_SIGNED(this->shots); i++, slot++)
    {
        if (slot->state == PLAYER_SHOT_INACTIVE)
        {
            continue;
        }

        if (slot->updateCallback != NULL)
        {
            if (slot->updateCallback(this, slot) != 0)
            {
                slot->state = PLAYER_SHOT_INACTIVE;
                continue;
            }
        }

        slot->position.operator float *()[0] +=
            g_Supervisor.framerateMultiplier * slot->velocity.x;
        slot->position.operator float *()[1] +=
            g_Supervisor.framerateMultiplier * slot->velocity.y;

        if (slot->shotType != 4 && slot->shotType != 5)
        {
            if (!g_GameManager.IsWithinPlayfield(
                    slot->position.operator float *()[0],
                    slot->position.operator float *()[1],
                    slot->vm.loadedSprite->widthPx,
                    slot->vm.loadedSprite->heightPx))
            {
                slot->state = PLAYER_SHOT_INACTIVE;
            }
        }

        if (g_AnmManager->ExecuteScript(&slot->vm) != ZUN_SUCCESS)
        {
            slot->state = PLAYER_SHOT_INACTIVE;
        }
        slot->timer++;
    }
}
// FUNCTION: th08 0x4512f0
#pragma var_order(i, slot, this)
void Player::DrawActiveShots()
{
    PlayerShot *slot;
    i32 i;

    slot = this->shots;
    for (i = 0; i < ARRAY_SIZE_SIGNED(this->shots); i++, slot++)
    {
        if (slot->state != PLAYER_SHOT_ACTIVE)
        {
            continue;
        }
        if (slot->vm.type != 0)
        {
            slot->vm.SetZRotation(slot->angle);
        }
        slot->vm.pos.x = g_GameManager.arcadeRegionTopLeftPos.x + slot->position.x;
        slot->vm.pos.y = g_GameManager.arcadeRegionTopLeftPos.y + slot->position.y;
        slot->vm.pos.z = 0.4f;
        if (slot->tintInExtremeYoukai != 0)
        {
            slot->vm.color1.r = 0xff;
            slot->vm.color1.g = 0x40;
            slot->vm.color1.b = 0x40;
        }
        g_AnmManager->Draw2D(&slot->vm);
        if (slot->drawCallback != NULL)
        {
            slot->drawCallback(this, slot);
        }
    }
}

// FUNCTION: th08 0x451400
#pragma var_order(i, slot, this)
void Player::DrawHitShots()
{
    PlayerShot *slot;
    i32 i;

    slot = this->shots;
    for (i = 0; i < ARRAY_SIZE_SIGNED(this->shots); i++, slot++)
    {
        if (slot->state != PLAYER_SHOT_HIT)
        {
            continue;
        }
        if (slot->vm.type != 0)
        {
            slot->vm.SetZRotation(slot->angle);
        }
        slot->vm.pos.x = g_GameManager.arcadeRegionTopLeftPos.x + slot->position.x;
        slot->vm.pos.y = g_GameManager.arcadeRegionTopLeftPos.y + slot->position.y;
        slot->vm.pos.z = 0.2f;
        if (slot->tintInExtremeYoukai != 0)
        {
            slot->vm.color1.r = 0xff;
            slot->vm.color1.g = 0x40;
            slot->vm.color1.b = 0x40;
        }
        g_AnmManager->DrawPlayerBullet(&slot->vm);
    }
}
// FUNCTION: th08 0x451500
i32 Player::UpdateShooting()
{
    if (*reinterpret_cast<i32 *>(0x164D2C8) < 20)
    {
        return 0;
    }

    if ((i32)this->shotTimer < 0)
    {
        return 0;
    }

    if (this->IsBombShotSuppressed())
    {
        return 0;
    }

    if (this->shotTimer.HasTicked())
    {
        if (*reinterpret_cast<i32 *>(0x17D6ED4) == 0 ||
            (g_GameManager.shotType != 1 && g_GameManager.shotType != 7 &&
             g_GameManager.shotType != 6))
        {
            this->SpawnShots((i32)this->shotTimer);
        }
    }

    this->shotTimer++;

    if ((i32)this->shotTimer >= 20)
    {
        this->shotTimer = -1;
    }

    if ((*reinterpret_cast<u16 *>(0x164D52C) & 1) != 0)
    {
        if ((i32)this->shotTimer < 0)
        {
            if (!g_Gui.IsDialoguePresent())
            {
                this->shotTimer = 0;
            }
        }
    }

    if (this->playerState == PLAYER_STATE_DYING || this->playerState == PLAYER_STATE_SPAWNING)
    {
        this->shotTimer = -1;
    }

    return 0;
}

// FUNCTION: th08 0x451640
void Player::StartShooting()
{
    if ((i32)this->shotTimer < 0)
        this->shotTimer = 0;
}

// FUNCTION: th08 0x451670
#pragma var_order(bullet, i, enemyBottomRight, savedRotation, bulletBottomRight, enemyTopLeft, damage, region, bulletTopLeft)
i32 Player::CalcDamageToEnemy(Float3 *enemyPosition, Float3 *enemySize, i32 *hitAccumulator,
                              i32 *bombHit)
{
    Float3 enemyTopLeft;
    Float3 enemyBottomRight;
    Float3 bulletTopLeft;
    Float3 bulletBottomRight;
    i32 damage;
    i32 i;
    i32 savedRotation;
    PlayerCollisionRegion *region;
    PlayerShot *bullet;

    damage = 0;
    if (!this->timer.HasTicked())
        return 0;

    PlayerBuildAabb(&enemyTopLeft, &enemyBottomRight, enemyPosition, enemySize);
    bullet = this->shots;
    if (bombHit != NULL)
        *bombHit = 0;

    for (i = 0; i < 128; i++, bullet++)
    {
        if (bullet->state == PLAYER_SHOT_INACTIVE ||
            (bullet->state != PLAYER_SHOT_ACTIVE && bullet->shotType != 3))
            continue;

        PlayerBuildAabb(&bulletTopLeft, &bulletBottomRight, &bullet->position, &bullet->hitboxSize);
        if (bulletTopLeft.y > enemyBottomRight.y || bulletTopLeft.x > enemyBottomRight.x ||
            bulletBottomRight.y < enemyTopLeft.y || bulletBottomRight.x < enemyTopLeft.x)
            continue;

        if ((bullet->shotType == 4 || bullet->shotType == 5) && (bullet->timer % 2) != 0)
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
                if (bullet->descriptor->extremeGaugeBehavior < 0)
                    g_ItemManager.SpawnItem(&bullet->position, static_cast<ItemType>(7), 3);
            }
            *hitAccumulator -= g_Player.damageAccumulatorThreshold;
        }

        if (bullet->shotType != 4 && bullet->shotType != 5 && bullet->shotType != 6)
        {
            if (bullet->state == PLAYER_SHOT_ACTIVE)
            {
                savedRotation = *reinterpret_cast<i32 *>(&bullet->vm.rotation.z);
                this->anmFile->SetAndExecuteScriptIdx(&bullet->vm, bullet->animationIndex + 11);
                *reinterpret_cast<i32 *>(&bullet->vm.rotation.z) = savedRotation;
                g_EffectManager.SpawnEffect(5, reinterpret_cast<D3DXVECTOR3 *>(&bullet->position), 1, -1);
                bullet->position.operator float *()[2] = 0.1f;
            }
            bullet->state = PLAYER_SHOT_HIT;
            if (bullet->shotType != 3)
            {
                bullet->velocity.x /= 8.0f;
                bullet->velocity.y /= 8.0f;
            }
        }
    }

    *hitAccumulator += damage > 50 ? 50 : damage;

    {
        region = this->damageRegions;
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

            if (region->mode == 0 && (++this->shotHitEffectCounter % 4) == 0)
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
i32 Player::IsBombShotSuppressed()
{
    return this->bombState.isInUse != 0 && this->bombState.callbackVariant == PLAYER_BOMB_CALLBACK_SPECIAL;
}
} /* namespace th08 */
