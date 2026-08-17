#include "th_pch.h"

#include "BulletManager.hpp"
#include "EclManager.hpp"
#include "GameManager.hpp"
#include "Gui.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"
#include "Spellcard.hpp"

namespace th08
{

DIFFABLE_STATIC(ItemManager, g_ItemManager);
DIFFABLE_STATIC(i32, g_ItemTimeOrbMode);
DIFFABLE_STATIC(ItemTimeOrbTimerStorage, g_ItemTimeOrbTimerStorage);
DIFFABLE_STATIC(ItemTimeOrbTimerStorage, g_ItemScatterTimerStorage);
DIFFABLE_STATIC(f32, g_ItemPlayfieldBottom);
DIFFABLE_STATIC(i32, g_MaxValuePointItemsCollected);
DIFFABLE_STATIC(Float2, g_ItemAnmManagerScreenShakeOffset);
DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 6, g_PowerUpThresholds) = {8, 24, 48, 80, 128, 999};

// FUNCTION: th08 0x441830
ZunBool ZunTimer::operator!=(int value)
{
    return this->current != value;
}

// FUNCTION: th08 0x440010
ItemManager::ItemManager()
{
}

// FUNCTION: th08 0x4337f0
void ItemManager::Initialize()
{
    memset(this, 0, sizeof(ItemManager));
    this->itemListTail = &this->itemListHead;
}

// FUNCTION: th08 0x440050
Item::Item()
{
}

#pragma var_order(i, item)
Item *ItemManager::SpawnItem(Float3 *position, ItemType itemType, i32 state)
{
    i32 i;
    Item *item = &this->items[this->nextIndex];

    if (position->x < -64.0f || position->x > 448.0f)
    {
        return &this->items[MAX_ITEMS];
    }

    if (g_GameManager.GetPower() >= 128 && (itemType == ITEM_POWER_SMALL || itemType == ITEM_POWER_BIG))
    {
        itemType = ITEM_POINT_SMALL;
    }
    if (itemType == ITEM_TIME)
    {
        state = ITEM_STATE_UNK3;
    }
    else if (itemType == ITEM_TIME2)
    {
        state = ITEM_STATE_UNK5;
        itemType = ITEM_TIME;
    }

    for (i = 0; i < MAX_ITEMS; i++)
    {
        this->nextIndex++;

        if (item->isInUse)
        {
            if (this->nextIndex >= MAX_ITEMS)
            {
                this->nextIndex = 0;
                item = &this->items[0];
            }
            else
            {
                item++;
            }

            if (itemType == ITEM_TIME)
            {
                return &this->items[MAX_ITEMS];
            }

            continue;
        }

        if (this->nextIndex >= MAX_ITEMS)
        {
            this->nextIndex = 0;
        }

        item->isInUse = true;
        item->currentPosition = *position;
        item->startPositionOrVelocity.x = 0.0f;
        item->startPositionOrVelocity.y = -2.2f;
        item->startPositionOrVelocity.z = 0.0f;
        item->itemType = itemType;
        item->state = state;
        item->timer = 0;

        if (state == ITEM_STATE_UNK2)
        {
            item->targetPosition.x = g_Rng.GetRandomF32InRange(288.0f) + 48.0f;
            item->targetPosition.y = g_Rng.GetRandomF32InRange(192.0f) - 64.0f;
            item->targetPosition.z = 0.0f;
            item->startPositionOrVelocity = item->currentPosition;
        }
        else if (state == ITEM_STATE_UNK3)
        {
            item->startPositionOrVelocity.y = -2.0f - g_Rng.GetRandomF32InRange(0.2f);
            item->startPositionOrVelocity.x = g_Rng.GetRandomF32SignedInRange(0.6f);

            if (g_Player.playerState == PLAYER_STATE_DYING)
            {
                item->state = ITEM_STATE_DEFAULT;
                item->startPositionOrVelocity.x = 0.0f;
                item->startPositionOrVelocity.y = -0.9f;
                item->startPositionOrVelocity.z = 0.0f;
            }
        }
        // ZUN bloat: This is just a duplicate of the above state!
        else if (state == ITEM_STATE_UNK5)
        {
            item->startPositionOrVelocity.y = -2.0f - g_Rng.GetRandomF32InRange(0.2f);
            item->startPositionOrVelocity.x = g_Rng.GetRandomF32SignedInRange(0.6f);

            if (g_Player.playerState == PLAYER_STATE_DYING)
            {
                item->state = ITEM_STATE_DEFAULT;
                item->startPositionOrVelocity.x = 0.0f;
                item->startPositionOrVelocity.y = -0.9f;
                item->startPositionOrVelocity.z = 0.0f;
            }
        }

        g_BulletManager.bulletAnm->SetAndExecuteScriptIdx(&item->sprite, itemType + 61);

        item->sprite.color1.d3dColor = 0xFFFFFFFF;
        item->sprite.zWriteDisabled = true;
        item->isMaxValue = false;
        item->isOnscreen = true;
        this->itemListTail->next = item;
        item->prev = this->itemListTail;
        item->next = NULL;
        this->itemListTail = item;

        break;
    }

    return i < MAX_ITEMS ? item : &this->items[MAX_ITEMS];
}

DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 6, g_PointItemExtendThresholds) = {100, 250, 500, 800, 1100, 9999};
DIFFABLE_STATIC_ARRAY_ASSIGN(i32, 4, g_ExPointItemExtendThresholds) = {200, 666, 9999, 1};

void ItemManager::UpdatePointItemExtendThreshold()
{
    if (g_GameManager.difficulty < 4)
    {
        if (g_GameManager.globals->pointItemExtendsSoFar < 6)
        {
            g_GameManager.globals->nextPointItemExtendThreshold =
                g_PointItemExtendThresholds[g_GameManager.globals->pointItemExtendsSoFar];
        }
        else
        {
            g_GameManager.globals->nextPointItemExtendThreshold =
                (g_GameManager.globals->pointItemExtendsSoFar - 5) * 500 + g_PointItemExtendThresholds[5];
        }
    }
    else
    {
        if (g_GameManager.globals->pointItemExtendsSoFar < 3)
        {
            g_GameManager.globals->nextPointItemExtendThreshold =
                g_ExPointItemExtendThresholds[g_GameManager.globals->pointItemExtendsSoFar];
        }
        else
        {
            g_GameManager.globals->nextPointItemExtendThreshold = 99999;
        }
    }
}

// FUNCTION: th08 0x440500
#pragma var_order(speed, interp, pickupScore, angle, itemBox, soundIndex, item)
void ItemManager::OnUpdate()
{
    f32 speed;
    f32 interp;
    i32 pickupScore;
    f32 angle;
    i32 soundIndex = 0;
    Item *item = this->itemListHead.next;
    Float3 itemBox(*reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_PlayerPrimaryShtFile) + 0x18),
                   *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_PlayerPrimaryShtFile) + 0x18), 16.0f);

    this->itemCount = 0;
    speed = *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(&g_Player) + 3)
                ? *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_PlayerSecondaryShtFile) + 0x34)
                : *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_PlayerPrimaryShtFile) + 0x34);
    speed *= g_EclGameTimeScale;

    while (item != NULL)
    {
        this->itemCount++;

        if (item->state == ITEM_STATE_UNK2)
        {
            if (item->timer < 60)
            {
                interp = (f32)item->timer / 60.0f;
                item->currentPosition =
                    item->targetPosition * interp + item->startPositionOrVelocity * (1.0f - interp);
                goto pickup;
            }
            if (item->timer == 60)
            {
                item->startPositionOrVelocity = Float3(0.0f, 0.0f, 0.0f);
                item->state = ITEM_STATE_DEFAULT;
            }
            goto moveItem;
        }
        else if (item->state == ITEM_STATE_UNK3)
        {
            item->startPositionOrVelocity.y += 0.05f * g_EclGameTimeScale;
            if (item->startPositionOrVelocity.y > 0.0f ||
                *reinterpret_cast<ZunTimer *>(&g_ItemScatterTimerStorage) < 0)
            {
                item->state = ITEM_STATE_AUTOCOLLECT;
            }
            if (g_Player.playerState == PLAYER_STATE_DYING)
            {
                item->state = ITEM_STATE_DEFAULT;
                item->startPositionOrVelocity.x = 0.0f;
                item->startPositionOrVelocity.y = -0.7f;
                item->startPositionOrVelocity.z = 0.0f;
            }
            goto moveItem;
        }
        else if (item->state == ITEM_STATE_UNK5)
        {
            item->startPositionOrVelocity.y += 0.05f * g_EclGameTimeScale;
            item->currentPosition += item->startPositionOrVelocity * speed;
            if (item->startPositionOrVelocity.y > 0.0f)
            {
                item->state = ITEM_STATE_AUTOCOLLECT;
            }
            else
            {
                goto executeOnly;
            }
            if (g_Player.playerState == PLAYER_STATE_DYING)
            {
                item->state = ITEM_STATE_DEFAULT;
                item->startPositionOrVelocity.x = 0.0f;
                item->startPositionOrVelocity.y = -0.7f;
                item->startPositionOrVelocity.z = 0.0f;
            }
            goto moveItem;
        }
        else
        {
            if (item->state == ITEM_STATE_AUTOCOLLECT ||
                (g_Player.position.y < g_PlayerPrimaryShtFile->pointItemValueLine &&
                 (g_GameManager.GetPower() >= 0.0 ||
                  *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(&g_Player) + 3) != 0 ||
                  g_TargetByte0164D0B1 == 1 || g_TargetByte0164D0B1 == 6)))
            {
                if (g_Player.playerState != PLAYER_STATE_DYING && g_Player.playerState != PLAYER_STATE_SPAWNING)
                {
                    angle = g_Player.FUN_0044c1b0(&item->currentPosition);
                    item->startPositionOrVelocity.FromAngleMagnitude(
                        angle, *reinterpret_cast<f32 *>(reinterpret_cast<u8 *>(g_PlayerPrimaryShtFile) + 0x14));
                    item->state = ITEM_STATE_AUTOCOLLECT;
                    item->currentPosition += item->startPositionOrVelocity * g_EclGameTimeScale;
                    goto pickup;
                }
                item->startPositionOrVelocity.y = -0.7f;
                item->state = ITEM_STATE_DEFAULT;
            }
            else
            {
                item->startPositionOrVelocity.x = 0.0f;
                item->startPositionOrVelocity.z = 0.0f;
                if (item->startPositionOrVelocity.y < -2.2f)
                    item->startPositionOrVelocity.y = -2.2f;
            }
        }

moveItem:
        item->currentPosition += item->startPositionOrVelocity * speed;
        if (item->state == ITEM_STATE_DEFAULT && g_ItemPlayfieldBottom + 16.0f <= item->currentPosition.y)
        {
            g_GameManager.DecreaseSubrank(3);
            item->Delete();
            item = item->next;
            continue;
        }

        if (item->startPositionOrVelocity.operator float *()[1] < 3.0f)
            item->startPositionOrVelocity.y += 0.03f * speed;
        else
            item->startPositionOrVelocity.y = 3.0f;

pickup:
        if (item->state != ITEM_STATE_UNK3 && g_Player.CalcItemBoxCollision(&item->currentPosition, &itemBox))
        {
            *reinterpret_cast<u16 *>(reinterpret_cast<u8 *>(g_ReplayManager) + 0xda) |= 0x40;
            switch (item->itemType)
            {
            case ITEM_POWER_SMALL:
                item->CollectPowerSmall();
                break;
            case ITEM_POINT:
                item->CollectPoint();
                break;
            case ITEM_POINT_SMALL:
                item->CollectPointSmall();
                break;
            case ITEM_POWER_BIG:
                item->CollectPowerBig();
                break;
            case ITEM_BOMB:
                if (g_GameManager.GetBombsRemaining() < 8)
                {
                    g_GameManager.AddToBombCount(1);
                    g_Gui.flags.bombDisplayUpdateFrames = 2;
                }
                g_GameManager.IncreaseSubrank(5);
                break;
            case ITEM_EXTEND:
                g_GameManager.CollectExtend();
                break;
            case ITEM_POWER_FULL:
                if (g_GameManager.GetPower() < 128)
                {
                    g_BulletManager.bulletmanager_fun_00415c60();
                    g_Gui.FUN_00437e5d(0, 1);
                    g_SoundPlayer.PlaySoundByIdx(SOUND_POWERUP, 0);
                    g_AsciiManager.CreatePlayerPointPopup(&item->currentPosition, -1, 0xffffc0a0);
                    this->ConvertAllPowerItemsToTimeOrbs(item);
                }
                g_GameManager.SetPower(128);
                g_GameManager.AddScore(1000);
                g_AsciiManager.CreatePlayerPointPopup(&item->currentPosition, 1000, 0xffffffff);
                g_Gui.flags.powerDisplayUpdateFrames = 2;
                break;
            case ITEM_POINT_STAR:
                if (g_ItemTimeOrbMode == 0)
                {
                    pickupScore = (g_GameManager.globals->graze / 40) * 10 + 300;
                    if (pickupScore <= 0)
                        pickupScore = 10;
                }
                else
                {
                    pickupScore = 100;
                }
                g_AsciiManager.CreateScorePopup(&item->currentPosition, pickupScore, 0xffffffff);
                g_GameManager.AddScore(pickupScore);
                break;
            case ITEM_TIME:
                item->CollectTimeOrb();
                break;
            default:
                break;
            }

            if (soundIndex <= SOUND_ITEM)
                soundIndex = item->isMaxValue ? SOUND_2C : SOUND_ITEM;
            item->Delete();
            item = item->next;
            continue;
        }

executeOnly:
        item->timer++;
        if (item->sprite.currentInstruction != NULL)
            g_AnmManager->ExecuteScript(&item->sprite);
        item = item->next;
    }

    if (soundIndex != 0)
        g_SoundPlayer.PlaySoundByIdx((SoundIdx)soundIndex, 0);

    if (*reinterpret_cast<ZunTimer *>(&g_ItemTimeOrbTimerStorage) != 0)
    {
        (*reinterpret_cast<ZunTimer *>(&g_ItemTimeOrbTimerStorage))--;
        if (*reinterpret_cast<ZunTimer *>(&g_ItemTimeOrbTimerStorage) <= 0)
            *reinterpret_cast<ZunTimer *>(&g_ItemTimeOrbTimerStorage) = 0;
    }
}

// FUNCTION: th08 0x440cf0
#pragma var_order(powerLevel, oldPowerLevel)
void Item::CollectPowerSmall()
{
    i32 powerLevel;
    i32 oldPowerLevel;

    if (g_GameManager.GetPower() >= 0x80)
    {
        goto increaseSubrank;
    }

    powerLevel = 0;
    while (g_GameManager.GetPower() >= g_PowerUpThresholds[powerLevel])
    {
        powerLevel++;
    }
    oldPowerLevel = powerLevel;

    *(u8 *)((u8 *)&g_GameManager + 0x3DBA8) = 0;
    g_GameManager.AddPower(1);

    if (g_GameManager.GetPower() >= 0x80)
    {
        g_GameManager.SetPower(0x80);
        if (!g_Spellcard.IsActive())
        {
            g_BulletManager.bulletmanager_fun_00415c60();
        }
        g_Gui.FUN_00437e5d(0, 1);
        g_ItemManager.ConvertAllPowerItemsToTimeOrbs(this);
    }

    g_GameManager.AddScore(10);
    g_Gui.flags.powerDisplayUpdateFrames = 2;

    while (g_GameManager.GetPower() >= g_PowerUpThresholds[powerLevel])
    {
        powerLevel++;
    }

    if (powerLevel != oldPowerLevel)
    {
        g_AsciiManager.CreateScorePopup(&this->currentPosition, -1, 0xffffc0a0);
        g_SoundPlayer.PlaySoundByIdx(SOUND_POWERUP, 0);
    }
    else
    {
        g_AsciiManager.CreateScorePopup(&this->currentPosition, 10, 0xffffffff);
    }

increaseSubrank:
    g_GameManager.IncreaseSubrank(1);
}

// FUNCTION: th08 0x440e40
#pragma var_order(pointItemValueBase, currentPointItemValue)
void Item::CollectPoint()
{
    i32 pointItemValueBase = g_GameManager.globals->pointItemValue;
    i32 currentPointItemValue;

    currentPointItemValue = static_cast<ZunBool>(this->currentPosition.y < g_PlayerPrimaryShtFile->pointItemValueLine)
                                ? pointItemValueBase
                                : pointItemValueBase / 2 -
                                      (i32)(this->currentPosition.y - g_PlayerPrimaryShtFile->pointItemValueLine) *
                                          (g_GameManager.globals->pointItemValue / 1000);
    if (this->isMaxValue == 1)
    {
        currentPointItemValue = pointItemValueBase;
    }

    currentPointItemValue -= currentPointItemValue % 10;
    if (g_GameManager.GaugeIsExtremelyHuman())
    {
        currentPointItemValue += currentPointItemValue;
    }

    g_AsciiManager.CreateScorePopup(&this->currentPosition, currentPointItemValue,
                                    currentPointItemValue >= pointItemValueBase ? 0xffffff00 : 0xffffffff);
    if (currentPointItemValue >= pointItemValueBase)
    {
        this->isMaxValue = true;
    }

    g_GameManager.AddScore(currentPointItemValue);
    g_GameManager.globals->pointItemsCollectedInStage++;
    g_GameManager.globals->pointItemsCollected++;
    g_Gui.flags.pointDisplayUpdateFrames = 2;

    if (currentPointItemValue >= pointItemValueBase)
    {
        g_GameManager.IncreaseSubrank(10);
    }
    else
    {
        g_GameManager.IncreaseSubrank(3);
    }

    if ((i32)g_GameManager.globals->pointItemExtendsSoFar >= 0)
    {
        while ((ItemManager::UpdatePointItemExtendThreshold(),
                g_GameManager.globals->pointItemsCollected >= g_GameManager.globals->nextPointItemExtendThreshold))
        {
            g_GameManager.CollectExtend();
            g_GameManager.globals->pointItemExtendsSoFar++;
        }
    }

    g_MaxValuePointItemsCollected++;
    g_GameManager.UpdateAntiTamper();
}

// FUNCTION: th08 0x441020
#pragma var_order(pointItemValueBase, currentPointItemValue)
void Item::CollectPointSmall()
{
    i32 pointItemValueBase = g_GameManager.globals->pointItemValue;
    i32 currentPointItemValue;

    currentPointItemValue = static_cast<ZunBool>(this->currentPosition.y < g_PlayerPrimaryShtFile->pointItemValueLine)
                                ? pointItemValueBase
                                : pointItemValueBase / 2 -
                                      (i32)(this->currentPosition.y - g_PlayerPrimaryShtFile->pointItemValueLine) *
                                          (g_GameManager.globals->pointItemValue / 1000);
    if (this->isMaxValue == 1)
    {
        currentPointItemValue = pointItemValueBase;
    }

    pointItemValueBase /= 10;
    pointItemValueBase -= pointItemValueBase % 10;
    currentPointItemValue /= 10;
    currentPointItemValue -= currentPointItemValue % 10;
    if (g_GameManager.GaugeIsExtremelyHuman())
    {
        currentPointItemValue += currentPointItemValue;
    }

    g_AsciiManager.CreateScorePopup(&this->currentPosition, currentPointItemValue,
                                    currentPointItemValue >= pointItemValueBase ? 0xffffff00 : 0xffffffff);
    g_GameManager.AddScore(currentPointItemValue);
    if (currentPointItemValue >= pointItemValueBase)
    {
        this->isMaxValue = true;
    }
}

// FUNCTION: th08 0x441170
#pragma var_order(powerLevel, oldPowerLevel)
void Item::CollectPowerBig()
{
    i32 powerLevel;
    i32 oldPowerLevel;

    if (g_GameManager.GetPower() >= 0x80)
    {
        return;
    }

    powerLevel = 0;
    while (g_GameManager.GetPower() >= g_PowerUpThresholds[powerLevel])
    {
        powerLevel++;
    }
    oldPowerLevel = powerLevel;

    g_GameManager.AddPower(8);

    if (g_GameManager.GetPower() >= 0x80)
    {
        g_GameManager.SetPower(0x80);
        if (!g_Spellcard.IsActive())
        {
            g_BulletManager.bulletmanager_fun_00415c60();
        }
        g_Gui.FUN_00437e5d(0, 1);
        g_ItemManager.ConvertAllPowerItemsToTimeOrbs(this);
    }

    g_Gui.flags.powerDisplayUpdateFrames = 2;
    g_GameManager.AddScore(10);

    while (g_GameManager.GetPower() >= g_PowerUpThresholds[powerLevel])
    {
        powerLevel++;
    }

    if (powerLevel != oldPowerLevel)
    {
        g_AsciiManager.CreateScorePopup(&this->currentPosition, -1, 0xffffc0a0);
        g_SoundPlayer.PlaySoundByIdx(SOUND_POWERUP, 0);
    }
    else
    {
        g_AsciiManager.CreateScorePopup(&this->currentPosition, 10, 0xffffffff);
    }
}

// FUNCTION: th08 0x4412b0
#pragma var_order(score)
void Item::CollectTimeOrb()
{
    i32 score;

    if (g_ItemTimeOrbMode == 0)
    {
        if (g_GameManager.globals->pointItemsCollectedInStage >= 2000)
        {
            score = 10000;
        }
        else
        {
            score = (g_GameManager.globals->pointItemsCollected / 2) * 10;
            if (score < 100)
                score = 100;
        }
    }
    else
    {
        score = 100;
    }

    if (this != NULL)
    {
        g_AsciiManager.CreateScorePopup(
            &this->currentPosition, score,
            g_GameManager.GetTimeOrbs() < g_GameManager.GetLastSpellTimeOrbThreshold() ? -536870913 : -536875136);
    }

    g_Gui.flags.timeDisplayUpdateFrames = 2;
    g_GameManager.AddScore(score);
    g_GameManager.AddTimeOrbs(1);
    g_Spellcard.spellcard_fun_00416b10(8000);

    if (*reinterpret_cast<ZunTimer *>(&g_ItemTimeOrbTimerStorage) == 0)
    {
        score = 111;
        g_GameManager.AddToYoukaiGauge(
            *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(&g_Player) + 3) ? score : -score, 0);
    }
}

// FUNCTION: th08 0x4413e0
void ItemManager::AutoCollectAllItems()
{
    Item *item = this->itemListHead.next;
    while (item != NULL)
    {
        item->state = ITEM_STATE_AUTOCOLLECT;
        item->startPositionOrVelocity = Float3(0.0f, -0.5f, 0.0f);
        item = item->next;
    }
}

// FUNCTION: th08 0x441450
void ItemManager::ConvertAllPowerItemsToTimeOrbs(Item *item)
{
    Item *current = this->itemListHead.next;

    while (current != NULL)
    {
        if (current != item && (current->itemType == ITEM_POWER_SMALL || current->itemType == ITEM_POWER_BIG))
        {
            if (current->startPositionOrVelocity.y > -0.5f)
            {
                current->startPositionOrVelocity.x = 0.0f;
                current->startPositionOrVelocity.y = -0.5f;
                current->startPositionOrVelocity.z = 0.0f;
            }
            g_EffectManager.SpawnEffect(0, reinterpret_cast<D3DXVECTOR3 *>(&current->currentPosition), 1, -1);
            current->itemType = ITEM_POINT_SMALL;
            g_BulletManager.bulletAnm->SetAndExecuteScriptIdx(&current->sprite, ITEM_POINT_SMALL + 61);
        }
        current = current->next;
    }
}

// FUNCTION: th08 0x441530
void ItemManager::CancelAutoCollect()
{
    Item *item = this->itemListHead.next;
    while (item != NULL)
    {
        if (item->state == ITEM_STATE_AUTOCOLLECT)
        {
            item->state = ITEM_STATE_DEFAULT;
            item->startPositionOrVelocity.x = 0.0f;
            item->startPositionOrVelocity.y = -0.9f;
            item->startPositionOrVelocity.z = 0.0f;
        }
        item = item->next;
    }
}

// FUNCTION: th08 0x4415a0
#pragma var_order(alpha, item, this)
void ItemManager::OnDraw()
{
    i32 alpha;
    Item *item = this->itemListHead.next;

    while (item != NULL)
    {
        item->sprite.pos.x = g_ItemAnmManagerScreenShakeOffset.x + item->currentPosition.x;
        item->sprite.pos.y = g_ItemAnmManagerScreenShakeOffset.y + item->currentPosition.y;
        item->sprite.pos.z = 0.15f;

        if (((f32 *)item->currentPosition)[1] < -8.0f)
        {
            item->sprite.pos.y = 8.0f + g_ItemAnmManagerScreenShakeOffset.y;
            if (item->isOnscreen)
            {
                g_BulletManager.bulletAnm->SetSprite(&item->sprite, item->itemType + 0xb6);
                item->isOnscreen = false;
                *(u32 *)((u8 *)&item->sprite + 0x1f8) |= 0x2000;
            }

            alpha = 255 - (i32)(((8.0f - ((f32 *)item->currentPosition)[1]) * 255.0f) / 128.0f);
            if (alpha < 0x40)
            {
                alpha = 0x40;
            }
            item->sprite.color1.d3dColor = (item->sprite.color1.d3dColor & 0xffffff) | (alpha << 24);
        }
        else
        {
            if (!item->isOnscreen)
            {
                g_BulletManager.bulletAnm->SetSprite(&item->sprite, item->itemType + 0xac);
                item->isOnscreen = true;
                item->sprite.color1.d3dColor = 0xffffffff;
                *(u32 *)((u8 *)&item->sprite + 0x1f8) |= 0x2000;
            }
        }

        g_AnmManager->Draw2D(&item->sprite);
        item = item->next;
    }
}

void Item::Delete()
{
    this->isInUse = false;
    this->prev->next = this->next;
    if (this->next != NULL)
    {
        this->next->prev = this->prev;
    }
    if (g_ItemManager.itemListTail == this)
    {
        g_ItemManager.itemListTail = this->prev;
    }
}

i32 ItemManager::GetTimeOrbCount()
{
    Item *next = this->itemListHead.next;
    i32 count = 0;

    while (next != NULL)
    {
        if (next->itemType == ITEM_TIME)
        {
            count++;
        }
        next = next->next;
    }

    return count;
}

} /* namespace th08 */
