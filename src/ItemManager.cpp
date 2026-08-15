#include "th_pch.h"

#include "BulletManager.hpp"
#include "GameManager.hpp"
#include "Gui.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"

namespace th08
{

DIFFABLE_STATIC(ItemManager, g_ItemManager);
DIFFABLE_STATIC(i32, g_MaxValuePointItemsCollected);

// FUNCTION: th08 0x440010
ItemManager::ItemManager()
{
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

            if (g_Player.playerState == PLAYER_STATE_DEAD)
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

            if (g_Player.playerState == PLAYER_STATE_DEAD)
            {
                item->state = ITEM_STATE_DEFAULT;
                item->startPositionOrVelocity.x = 0.0f;
                item->startPositionOrVelocity.y = -0.9f;
                item->startPositionOrVelocity.z = 0.0f;
            }
        }

        // TODO: Uncomment this when BulletManager is actually done
        // g_BulletManager.bulletAnm->SetAndExecuteScriptIdx(&item->sprite, itemType + 61);

        item->sprite.color1.d3dColor = 0xFFFFFFFF;
        item->sprite.zWriteDisabled = true;
        item->isMaxValue = false;
        item->isOnscreen = true;
        this->itemListTail->next = item;
        item->prev = this->itemListTail;
        item->next = NULL;
        this->itemListTail = item;

        return i < MAX_ITEMS ? item : &this->items[MAX_ITEMS];
    }
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

// STUB: th08 0x440500
void ItemManager::OnUpdate()
{
    // TODO: NEEDS WORK ON Gui
}

// STUB: th08 0x440cf0
void Item::CollectPowerSmall()
{
}

// FUNCTION: th08 0x440e40
void Item::CollectPoint()
{
    i32 currentPointItemValue;
    i32 isAbovePointLine;
    i32 pointItemValue;
    i32 pointItemValueBase = g_GameManager.globals->pointItemValue;

    if (this->currentPosition.y < g_PlayerPrimaryShtFile->pointItemValueLine)
    {
        isAbovePointLine = true;
    }
    else
    {
        isAbovePointLine = false;
    }

    if (isAbovePointLine)
    {
        pointItemValue = pointItemValueBase;
    }
    else
    {
        pointItemValue =
            pointItemValueBase / 2 -
            (i32)(this->currentPosition.y - g_PlayerPrimaryShtFile->pointItemValueLine) *
                (g_GameManager.globals->pointItemValue / 1000);
    }

    currentPointItemValue = pointItemValue;
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
        for (;;)
        {
            ItemManager::UpdatePointItemExtendThreshold();
            if (g_GameManager.globals->pointItemsCollected < g_GameManager.globals->nextPointItemExtendThreshold)
            {
                break;
            }
            g_GameManager.CollectExtend();
            g_GameManager.globals->pointItemExtendsSoFar++;
        }
    }

    g_MaxValuePointItemsCollected++;
    g_GameManager.UpdateAntiTamper();
}

// FUNCTION: th08 0x441020
#pragma var_order(pointItemValueBase, currentPointItemValue, pointItemValue, isAbovePointLine, this)
void Item::CollectPointSmall()
{
    i32 pointItemValueBase = g_GameManager.globals->pointItemValue;
    i32 pointItemValue;
    i32 currentPointItemValue;
    i32 isAbovePointLine;

    if (this->currentPosition.y < g_PlayerPrimaryShtFile->pointItemValueLine)
    {
        isAbovePointLine = true;
    }
    else
    {
        isAbovePointLine = false;
    }

    if (isAbovePointLine)
    {
        pointItemValue = pointItemValueBase;
    }
    else
    {
        pointItemValue =
            pointItemValueBase / 2 -
            (i32)(this->currentPosition.y - g_PlayerPrimaryShtFile->pointItemValueLine) *
                (g_GameManager.globals->pointItemValue / 1000);
    }

    currentPointItemValue = pointItemValue;
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

// STUB: th08 0x441170
void Item::CollectPowerBig()
{
}

// STUB: th08 0x4412b0
void Item::CollectTimeOrb()
{
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

// STUB: th08 0x441450
void ItemManager::ConvertAllPowerItemsToTimeOrbs(Item *item)
{
    // TODO: NEEDS WORK ON EffectManager
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

// STUB: th08 0x4415a0
void ItemManager::OnDraw()
{
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
