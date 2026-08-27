#pragma once

#include "AnmManager.hpp"
#include "Supervisor.hpp"

namespace th08
{
enum ItemType
{
    ITEM_POWER_SMALL,
    ITEM_POINT,
    ITEM_POWER_BIG,
    ITEM_BOMB,
    ITEM_POWER_FULL,
    ITEM_EXTEND,
    ITEM_POINT_STAR,
    ITEM_TIME,
    ITEM_POINT_SMALL,
    ITEM_RESERVED_9,
    // Spawn request converted immediately to ITEM_TIME with apex-gated collection.
    ITEM_TIME_APEX_AUTOCOLLECT_REQUEST,
};

struct ItemTimeOrbTimerStorage
{
    i32 current;
    f32 subFrame;
    i32 previous;
};

enum ItemState
{
    ITEM_STATE_DEFAULT,
    ITEM_STATE_AUTOCOLLECT,
    ITEM_STATE_DEATH_DROP_SPREAD,
    ITEM_STATE_TIME_RISING,
    ITEM_STATE_RESERVED_4,
    ITEM_STATE_TIME_RISING_TO_APEX,
};

struct Item
{
    Item();

    AnmVm sprite;

    Float3 currentPosition;
    Float3 startPositionOrVelocity;
    Float3 targetPosition;

    ZunTimer timer;

    i8 itemType;
    i8 isInUse;
    i8 isOnscreen;
    i8 state;
    i8 isMaxValue;

    Item *next;
    Item *prev;

    void CollectPowerSmall();
    void CollectPoint();
    void CollectPointSmall();
    void CollectPowerBig();
    void CollectTimeOrb();

    void Delete();
};
C_ASSERT(sizeof(Item) == 0x2e4);

#define MAX_ITEMS 2096

struct ItemManager
{
    ItemManager();
    void Initialize();

    Item items[MAX_ITEMS + 1];

    i32 nextIndex;
    u32 itemCount;

    Item itemListHead;
    Item *itemListTail;

    Item *SpawnItem(Float3 *position, ItemType itemType, int state);
    static void UpdatePointItemExtendThreshold();
    void OnUpdate();
    void AutoCollectAllItems();
    void ConvertAllPowerItemsToTimeOrbs(Item *item);
    void CancelAutoCollect();
    void OnDraw();
    i32 GetTimeOrbCount();
};
C_ASSERT(sizeof(ItemManager) == 0x17b094);

DIFFABLE_EXTERN(ItemManager, g_ItemManager);

}; // namespace th08
