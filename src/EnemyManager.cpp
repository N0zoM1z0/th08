#include "th_pch.h"

#include "EnemyManager.hpp"
#include "AnmManager.hpp"
#include "AsciiManager.hpp"
#include "Background.hpp"
#include "BulletManager.hpp"
#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "ItemManager.hpp"
#include "Gui.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "ReplayManager.hpp"

namespace th08
{

ZunBool IsDisableResourceReload();
i32 IsResourceReloadEnabled();
f32 __stdcall InterpolateWrappedAngle(f32 angle1, f32 angle2, f32 factor);

DIFFABLE_STATIC(EnemyManager, g_EnemyManager);
DIFFABLE_STATIC_ARRAY_ASSIGN(u8, 32, g_EnemyDropSchedule) = {
    0, 0, 1, 0, 1, 0, 0, 0,
    1, 1, 0, 0, 1, 1, 1, 0,
    1, 0, 1, 0, 1, 0, 1, 0,
    1, 0, 0, 1, 1, 1, 0, 0,
};

namespace EclOperands
{




} // namespace EclOperands

// FUNCTION: th08 0x422c40
void Enemy::UpdateMovement()
{
    switch ((this->flags1 >> ENEMY_FLAG_MOVEMENT_MODE_SHIFT) & 3)
    {
    case 3:
    {
        {
            Float3 legacyWork;
        }
        Float3 polarVelocity;

        this->orbitAngle =
            AddNormalizeAngle(
                this->orbitAngle,
                g_Supervisor.framerateMultiplier * this->orbitAngularVelocity);
        this->orbitRadius =
            g_Supervisor.framerateMultiplier * this->radialVelocity + this->orbitRadius;
        polarVelocity.FromAngleMagnitude(
            this->orbitAngle, this->orbitRadius);
        this->velocity.x = polarVelocity.x + this->movementInterpolationOrigin.x - this->position.x;
        this->velocity.y = polarVelocity.y + this->movementInterpolationOrigin.y - this->position.y;
        this->movementAngle = VectorAngle(this->velocity.y, this->velocity.x);
        if (this->movementDuration > 0)
        {
            this->movementTimer--;
            if (this->movementTimer <= 0)
                this->flags1 &= ~ENEMY_FLAG_MOVEMENT_MODE_MASK;
        }
        break;
    }

    case 1:
        this->movementAngle =
            AddNormalizeAngle(
                this->movementAngle,
                g_Supervisor.framerateMultiplier * this->angularVelocity);
        this->speed = g_Supervisor.framerateMultiplier * this->acceleration + this->speed;
        this->velocity.FromAngleMagnitude(
            this->movementAngle, this->speed);
        this->velocity.operator float *()[2] = 0.0f;
        if (this->movementDuration > 0)
        {
            this->movementTimer--;
            if (this->movementTimer <= 0)
                this->flags1 &= ~ENEMY_FLAG_MOVEMENT_MODE_MASK;
        }
        break;

    case 2:
    {
        f32 progress;

        this->movementTimer--;
        progress = 1.0f - (f32)this->movementTimer / this->movementDuration;
        if (progress < 0.0f)
            progress = 0.0f;
        switch ((this->flags1 >> ENEMY_FLAG_MOVEMENT_EASING_SHIFT) & 7)
        {
        case 1: progress *= progress; break;
        case 2: progress = progress * progress * progress; break;
        case 3: progress = progress * progress * progress * progress; break;
        case 4:
            progress = 1.0f - progress;
            progress *= progress;
            progress = 1.0f - progress;
            break;
        case 5:
            progress = 1.0f - progress;
            progress = progress * progress * progress;
            progress = 1.0f - progress;
            break;
        case 6:
            progress = 1.0f - progress;
            progress = progress * progress * progress * progress;
            progress = 1.0f - progress;
            break;
        }

        this->velocity = this->movementInterpolationOrigin + this->movementInterpolationDelta * progress - this->position;
        if (((this->flags1 >> ENEMY_FLAG_MIRROR_MOVEMENT_X_SHIFT) & 1) != 0)
            this->velocity.x = -this->velocity.x;
        this->movementAngle = VectorAngle(this->velocity.y, this->velocity.x);
        if (this->movementTimer <= 0)
        {
            this->flags1 &= ~ENEMY_FLAG_MOVEMENT_MODE_MASK;
            this->position = this->movementInterpolationOrigin + this->movementInterpolationDelta;
            this->velocity = Float3(0.0f, 0.0f, 0.0f);
        }
        break;
    }
    }
}

// FUNCTION: th08 0x429e00
#pragma var_order(i, enemy, this)
void EnemyManager::Initialize()
{
    u8 *enemy = reinterpret_cast<u8 *>(&this->enemies[0]);
    i32 i;

    memset(this, 0, 0x9DCF10);
    for (i = 0; (u32)i < 4; i++)
        this->timelineEventSlots[i] = -1;

    enemy = reinterpret_cast<u8 *>(&this->spawnTemplate);
    memset(enemy, 0, 0x53D0);
    for (i = 0; i < 2; i++)
        reinterpret_cast<Enemy *>(enemy)->secondaryVms[i].scriptIndex = -1;
    for (i = 0; i < 0x60; i++)
        reinterpret_cast<Enemy *>(enemy)->trailSamples[i].position.x = -999.0f;

    reinterpret_cast<Enemy *>(enemy)->flags1 |= ENEMY_FLAG_ACTIVE;
    reinterpret_cast<Enemy *>(enemy)->bossTimer = 0;
    reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_HAS_BEEN_IN_BOUNDS;

    *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->hitboxDimensions) = D3DXVECTOR3(24.0f, 24.0f, 24.0f);
    *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->velocity) = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    *reinterpret_cast<i32 *>(&reinterpret_cast<Enemy *>(enemy)->angularVelocity) = 0;
    *reinterpret_cast<i32 *>(&reinterpret_cast<Enemy *>(enemy)->movementAngle) = 0;
    *reinterpret_cast<i32 *>(&reinterpret_cast<Enemy *>(enemy)->acceleration) = 0;
    *reinterpret_cast<i32 *>(&reinterpret_cast<Enemy *>(enemy)->speed) = 0;
    reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_MOVEMENT_MODE_MASK;
    reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_DEFER_BULLET_PATTERN;
    reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_MIRROR_MOVEMENT_X;
    reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_BOSS;
    reinterpret_cast<Enemy *>(enemy)->activeEclCallStackDepth = 0;
    reinterpret_cast<Enemy *>(enemy)->life = 1;
    reinterpret_cast<Enemy *>(enemy)->score = 100;
    reinterpret_cast<Enemy *>(enemy)->deathAnm1 = 0;
    reinterpret_cast<Enemy *>(enemy)->deathAnm2 = 0;
    reinterpret_cast<Enemy *>(enemy)->deathAnm3 = 0;
    reinterpret_cast<Enemy *>(enemy)->shootIntervalFrames = 0;
    reinterpret_cast<Enemy *>(enemy)->shootIntervalTimer = 0;
    *reinterpret_cast<D3DXVECTOR3 *>(&reinterpret_cast<Enemy *>(enemy)->shootOffset) = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    reinterpret_cast<Enemy *>(enemy)->anmScripts.moveLeft = -1;
    reinterpret_cast<Enemy *>(enemy)->anmScripts.moveRight = -1;
    reinterpret_cast<Enemy *>(enemy)->anmScripts.idleInitial = -1;
    reinterpret_cast<Enemy *>(enemy)->flags1 |= ENEMY_FLAG_COLLISION;
    reinterpret_cast<Enemy *>(enemy)->flags1 |= ENEMY_FLAG_DAMAGEABLE;
    reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_NO_SPRITE;
    reinterpret_cast<Enemy *>(enemy)->flags1 |= ENEMY_FLAG_ACCEPTS_DAMAGE;
    reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_SPECIAL_INTERACTION;
    reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_DEATH_MODE_MASK;
    reinterpret_cast<Enemy *>(enemy)->deathCallbackSubId = -1;
    reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_CLAMP_POSITION;
    reinterpret_cast<Enemy *>(enemy)->attachedEffectCount = 0;
    reinterpret_cast<Enemy *>(enemy)->pendingEclSubroutineIndex = -1;
    for (i = 0; i < 4; i++)
        reinterpret_cast<Enemy *>(enemy)->lifeCallbackThresholds[i] = -1;
    reinterpret_cast<Enemy *>(enemy)->timerCallbackThresholdFrames = -1;
    reinterpret_cast<Enemy *>(enemy)->selectedLaserSlot = 0;
    reinterpret_cast<Enemy *>(enemy)->damageFlashTimer = 0;
    reinterpret_cast<Enemy *>(enemy)->flags1 &= ~ENEMY_FLAG_ROTATE_ANM_WITH_MOVEMENT;
    reinterpret_cast<Enemy *>(enemy)->bulletRankInfluence.speedLow = -0.15f;
    reinterpret_cast<Enemy *>(enemy)->bulletRankInfluence.speedHigh = 0.15f;
    reinterpret_cast<Enemy *>(enemy)->bulletSpawnDescriptor.spawnSound = 7;
    reinterpret_cast<Enemy *>(enemy)->bulletSpawnDescriptor.transformSound = 25;
    reinterpret_cast<Enemy *>(enemy)->minimumPlayerDistanceSquared = 1024.0f;
    reinterpret_cast<Enemy *>(enemy)->playerShotHitAccumulator = g_Player.damageAccumulatorThreshold;
}

// FUNCTION: th08 0x42a210
EnemyManager::EnemyManager()
{
    this->Initialize();
}

// FUNCTION: th08 0x42a280
Enemy::Enemy() {}

// FUNCTION: th08 0x42a450
EnemyEclContext::EnemyEclContext() {}

// FUNCTION: th08 0x42a490
EnemyTrailSample::EnemyTrailSample() {}

// FUNCTION: th08 0x42a4c0
EnemyEclInterpolationSlot::EnemyEclInterpolationSlot() {}

// FUNCTION: th08 0x42a820
#pragma var_order(i, this)
void Enemy::ReleaseAttachedEffects()
{
    i32 i;

    for (i = 0; i < this->attachedEffectCount; i++)
    {
        if (this->attachedEffects[i] == NULL)
            continue;
        this->attachedEffects[i]->releaseRequested = 1;
        this->attachedEffects[i] = NULL;
    }
    this->attachedEffectCount = 0;
}

namespace EclOperands
{

// FUNCTION: th08 0x42adb0
#pragma var_order(j, nextEnemy, enemy, popupColor, chainIndex, position, dropLocals, itemType, attachedPosition)
void EnemyOverlay::DetachEnemyChain(i32 awardRewards)
{
    i32 j;
    EnemyOverlay *nextEnemy;
    EnemyOverlay *enemy;
    i32 popupColor;
    i32 chainIndex;

    j = reinterpret_cast<Enemy *>(this)->CountParentChain();
    if (j != 0)
    {
        chainIndex = 0;
        Float3 position;
        struct DropLocals { i32 itemCount; i32 i; } dropLocals;
        i32 itemType;
        enemy = reinterpret_cast<EnemyOverlay *>(
            reinterpret_cast<Enemy *>(this)->nextInAttachmentChain);
        popupColor = j < 2 ? -1 : (j < 6 ? -48 : (j < 10 ? -80 : -128));

        while (enemy != NULL)
        {
            if (((reinterpret_cast<Enemy *>(enemy)->flags1 >>
                  ENEMY_FLAG_INHERIT_PARENT_POSITION_SHIFT) & 1) != 0)
                reinterpret_cast<Enemy *>(enemy)->positionOffset =
                    reinterpret_cast<Enemy *>(this)->position;

            nextEnemy = reinterpret_cast<EnemyOverlay *>(
                reinterpret_cast<Enemy *>(enemy)->nextInAttachmentChain);
            reinterpret_cast<Enemy *>(enemy)->flags1 |= ENEMY_FLAG_SUPPRESS_DEATH_EFFECTS;
            reinterpret_cast<Enemy *>(enemy)->parentEnemy = NULL;
            reinterpret_cast<Enemy *>(enemy)->nextInAttachmentChain = NULL;
            reinterpret_cast<Enemy *>(enemy)->previousInAttachmentChain = NULL;

            if (awardRewards != 0)
            {
                itemType = (((reinterpret_cast<Enemy *>(this)->flags1 >>
                              ENEMY_FLAG_BOSS_SHIFT) & 1) != 0) ? 7 : 9;
                if (g_GameManager.IsSoloYoukai())
                    dropLocals.itemCount = j >= 10 ? 26 : j * 2 + 6;
                else if (g_GameManager.IsSoloHuman())
                    dropLocals.itemCount = j >= 4 ? 40 : j * 6 + 16;
                else
                    dropLocals.itemCount = j >= 8 ? 26 : j * 2 + 10;

                if (g_Player.bombState.isInUse != 0)
                    j /= 3;

                g_AsciiManager.CreateTimePopup(
                    &reinterpret_cast<Enemy *>(enemy)->worldPosition, j, 0, popupColor);
                reinterpret_cast<Enemy *>(enemy)->worldPosition =
                    reinterpret_cast<Enemy *>(enemy)->position +
                    reinterpret_cast<Enemy *>(enemy)->positionOffset;
                g_Player.CreateCircleCancelRegion(&reinterpret_cast<Enemy *>(enemy)->worldPosition,
                                      32.0f, 2.0f, 8, itemType);

                for (dropLocals.i = 0; dropLocals.i < dropLocals.itemCount; dropLocals.i++)
                {
                    position.FromAngleMagnitude(
                        g_Rng.GetRandomF32SignedInRange(ZUN_PI),
                        g_Rng.GetRandomF32InRange((f32)dropLocals.itemCount * 2.0f));
                    position.z = 0.0f;
                    position += reinterpret_cast<Enemy *>(enemy)->worldPosition;
                    g_ItemManager.SpawnItem(&position, ITEM_TIME, 3);
                }

                if (!g_EnemyManager.HasBoss() || g_Spellcard.IsActive())
                {
                    reinterpret_cast<Enemy *>(enemy)->itemDropType = 8;
                    reinterpret_cast<Enemy *>(enemy)->DropItems(0);
                }
                g_SoundPlayer.PlaySoundPositionedByIdx(
                    static_cast<SoundIdx>(chainIndex % 2 + 2),
                    reinterpret_cast<Enemy *>(enemy)->worldPosition.x);
            }

            reinterpret_cast<Enemy *>(enemy)->powerOrPointItemDropCount = 0;
            reinterpret_cast<Enemy *>(enemy)->pointItemDropCount = 0;
            reinterpret_cast<Enemy *>(enemy)->itemDropType = -2;
            enemy = nextEnemy;
            ++chainIndex;
        }

        if (awardRewards != 0)
        {
            g_AsciiManager.SetScale(2.0f, 2.0f);
            g_AsciiManager.CreateTimePopup(
                &reinterpret_cast<Enemy *>(this)->worldPosition,
                reinterpret_cast<Enemy *>(this)->linkedChildCount, 0, 0xFFF0F00F);
            g_AsciiManager.SetScale(1.0f, 1.0f);

            for (j = 0; j < 2 * reinterpret_cast<Enemy *>(this)->linkedChildCount; j++)
            {
                position.FromAngleMagnitude(
                    g_Rng.GetRandomF32SignedInRange(ZUN_PI),
                    g_Rng.GetRandomF32InRange(128.0f));
                position.z = 0.0f;
                position += reinterpret_cast<Enemy *>(this)->worldPosition;
                g_ItemManager.SpawnItem(&position, ITEM_TIME, 1);
            }
            g_Player.CreateCircleCancelRegion(&reinterpret_cast<Enemy *>(this)->worldPosition,
                                  32.0f, 1.0f, 16, 7);
            *reinterpret_cast<ZunTimer *>(&g_Player.timerE2ADC) = 0;
        }
    }

    if (reinterpret_cast<Enemy *>(this)->HasAttachedEnemy() && awardRewards != 0)
    {
        Float3 attachedPosition;
        g_GameManager.AddToYoukaiGauge(-g_GameManager.GetYoukaiGauge() / 12, 0);
        *reinterpret_cast<ZunTimer *>(&g_Player.timerE2AE8) = 0;
        *reinterpret_cast<ZunTimer *>(&g_Player.timerE2AD0) = 30;
        *reinterpret_cast<ZunTimer *>(&g_Player.timerE2ADC) = 50;
        reinterpret_cast<Enemy *>(this)->worldPosition =
            reinterpret_cast<Enemy *>(this)->position +
            reinterpret_cast<Enemy *>(this)->positionOffset;
        g_AsciiManager.CreateTimePopup(
            &reinterpret_cast<Enemy *>(this)->worldPosition, 1, 0, 0xFFFFFFFF);
        g_ItemManager.SpawnItem(&reinterpret_cast<Enemy *>(this)->worldPosition, ITEM_TIME, 1);
        reinterpret_cast<Enemy *>(this)->powerOrPointItemDropCount = 0;
        reinterpret_cast<Enemy *>(this)->pointItemDropCount = 0;
        reinterpret_cast<Enemy *>(this)->itemDropType = -2;
    }

    reinterpret_cast<Enemy *>(this)->DetachFromParentChain();
}

} // namespace EclOperands

// FUNCTION: th08 0x42b2f0
void Enemy::DetachFromParentChain()
{
    if (this->HasAttachedEnemy())
    {
        this->previousInAttachmentChain->nextInAttachmentChain =
            this->nextInAttachmentChain;
        if (this->nextInAttachmentChain != NULL)
        {
            this->nextInAttachmentChain->previousInAttachmentChain =
                this->previousInAttachmentChain;
        }
        this->parentEnemy = NULL;
        this->nextInAttachmentChain = NULL;
        this->previousInAttachmentChain = NULL;
    }
    else
    {
        this->previousInAttachmentChain = NULL;
        this->nextInAttachmentChain = NULL;
    }
}

DIFFABLE_STATIC(ChainElem, g_EnemyManagerCalcChain);
DIFFABLE_STATIC(ChainElem, g_EnemyManagerDrawChainHighPrio);
DIFFABLE_STATIC(ChainElem, g_EnemyManagerDrawChainLowPrio);

// FUNCTION: th08 0x42b370
#pragma var_order(damage, i, maxHp, this)
void Enemy::ApplyDamageToParent(i32 amount)
{
    i32 damage;
    i32 i;
    i32 maxHp;

    if (!this->HasAttachedEnemy())
        return;
    if (g_Player.bombState.isInUse != 0)
        return;

    maxHp = 0;
    for (i = 0; i < 4; ++i)
    {
        if (maxHp < this->parentEnemy->lifeCallbackThresholds[i])
        {
            maxHp = this->parentEnemy->lifeCallbackThresholds[i];
        }
    }

    damage = amount / 2;
    if (this->parentEnemy->damageReductionTimer > 0)
    {
        if (((this->parentEnemy->flags1 >> ENEMY_FLAG_BOSS_SHIFT) & 1) != 0)
            damage /= 9;
        else
            damage = 0;
    }

    if (damage == 0)
        return;

    this->parentEnemy->life -= damage;
    if (this->parentEnemy->life <= maxHp)
    {
        this->parentEnemy->life = maxHp;
    }
}

// FUNCTION: th08 0x42b490
#pragma var_order(state, phaseCount, i, work, enemyCursor, k, this)
i32 Enemy::HandleLifeCallback()
{
    u32 state;
    i32 phaseCount;
    i32 i;
    i32 work;
    Enemy *enemyCursor;
    i32 k;

    phaseCount = 0;
    state = 0;
    this->flags2 &= ~ENEMY_FLAG2_DAMAGE_FEEDBACK_MASK;
    for (i = 0; i < 4; i++)
    {
        if (this->lifeCallbackThresholds[i] < 0)
            continue;

        phaseCount++;
        if (this->life < this->lifeCallbackThresholds[i])
        {
            this->life = this->lifeCallbackThresholds[i];
            this->phaseStartingLife = this->life;
            g_EclManager.CallEclSub(
                reinterpret_cast<EnemyEclContext *>(&this->mainEclContextStorage),
                *reinterpret_cast<i16 *>(&this->lifeCallbackSubIds[i]));
            this->lifeCallbackThresholds[i] = -1;
            this->phaseEndTimeRemainingSeconds =
                (this->timerCallbackThresholdFrames - (i32)this->bossTimer) / 60;
            this->timerCallbackThresholdFrames = -1;

            for (work = 0; work < 4; work++)
            {
                if (this->childEclBlocks[work] != NULL)
                {
                    g_ZunMemory.Free(this->childEclBlocks[work]);
                    this->childEclBlocks[work] = NULL;
                }
            }

            this->ResetBulletRankInfluence();
            this->activeEclCallStackDepth = 0;
            this->flags2 &= ~ENEMY_FLAG2_DAMAGE_FEEDBACK_MASK;
            this->bulletSpawnDescriptor = g_EnemyManager.spawnTemplate.bulletSpawnDescriptor;
            this->shootIntervalFrames = 0;
            reinterpret_cast<EclOperands::EnemyOverlay *>(this)->DetachEnemyChain(1);

            enemyCursor = &g_EnemyManager.enemies[0];
            for (k = 0; k < 480; k++, enemyCursor++)
            {
                if ((enemyCursor->flags1 & ENEMY_FLAG_ACTIVE) == 0)
                    continue;
                if (((enemyCursor->flags1 >> ENEMY_FLAG_BOSS_SHIFT) & 1) != 0)
                    continue;

                enemyCursor->life = 0;
                if (enemyCursor->deathCallbackSubId >= 0)
                {
                    g_EclManager.CallEclSub(
                        reinterpret_cast<EnemyEclContext *>(&enemyCursor->mainEclContextStorage),
                        enemyCursor->deathCallbackSubId);
                    enemyCursor->deathCallbackSubId = -1;
                }
            }

            if (((this->flags1 >> ENEMY_FLAG_BOSS_SHIFT) & 1) != 0 &&
                g_Player.playerState == 0)
            {
                g_Player.timer = 70;
                g_Player.playerState = 3;
            }
            return 1;
        }

        work = this->life - this->lifeCallbackThresholds[i];
        if (g_Spellcard.IsActive())
        {
            if (work < 120)
                state = 3;
            else if (work < 200)
                state = 2;
            else if (work < 300)
                state = 1;
            else
                state = 0;
        }
        else
        {
            if (work < 500)
                state = 3;
            else if (work < 1500)
                state = 2;
            else if (work < 2200)
                state = 1;
            else
                state = 0;
        }

        if (((this->flags2 >> ENEMY_FLAG2_DAMAGE_FEEDBACK_SHIFT) & 3) < state)
        {
            reinterpret_cast<EnemyFlag2Bits *>(&this->flags2)->damageFeedbackLevel = state;
        }
    }

    if (phaseCount == 0)
    {
        work = this->life;
        if (((this->flags1 >> ENEMY_FLAG_BOSS_SHIFT) & 1) != 0)
        {
            if (g_Spellcard.IsActive())
            {
                if (work < 120)
                    state = 3;
                else if (work < 300)
                    state = 2;
                else if (work < 400)
                    state = 1;
                else
                    state = 0;
            }
            else
            {
                if (work < 600)
                    state = 3;
                else if (work < 1600)
                    state = 2;
                else if (work < 2400)
                    state = 1;
                else
                    state = 0;
            }
        }
        else if (g_Spellcard.IsActive())
        {
            if (work < 10)
                state = 3;
            else
                state = 0;
        }
        else
        {
            if (work < 50)
                state = 3;
            else
                state = 0;
        }

        if (((this->flags2 >> ENEMY_FLAG2_DAMAGE_FEEDBACK_SHIFT) & 3) < state)
        {
            reinterpret_cast<EnemyFlag2Bits *>(&this->flags2)->damageFeedbackLevel = state;
        }
    }
    return 0;
}

// FUNCTION: th08 0x42b930
#pragma var_order(i, maxThreshold, selectedOrK, enemyCursor, j, this)
i32 Enemy::HandleTimerCallback()
{
    i32 i;
    i32 maxThreshold;
    i32 selectedOrK;
    Enemy *enemyCursor;
    i32 j;

    if (((this->flags1 >> ENEMY_FLAG_BOSS_SHIFT) & 1) != 0 && this->bossSlot == 0)
    {
        g_Gui.SetBossTimerSeconds(
            (this->timerCallbackThresholdFrames - (i32)this->bossTimer) / 60);
    }

    if (this->bossTimer >= this->timerCallbackThresholdFrames)
    {
    this->phaseEndTimeRemainingSeconds = 0;
    maxThreshold = 0;
    for (i = 0; i < 4; i++)
    {
        if (this->lifeCallbackThresholds[i] < 0)
            continue;
        if (maxThreshold < this->lifeCallbackThresholds[i])
        {
            maxThreshold = this->lifeCallbackThresholds[i];
            selectedOrK = i;
        }
    }

    if (maxThreshold > 0)
    {
        this->life = this->lifeCallbackThresholds[selectedOrK];
        this->phaseStartingLife = this->life;
        this->lifeCallbackThresholds[selectedOrK] = -1;
    }

    g_EclManager.CallEclSub(
        reinterpret_cast<EnemyEclContext *>(&this->mainEclContextStorage),
        *reinterpret_cast<i16 *>(&this->timerCallbackSubId));
    this->timerCallbackThresholdFrames = -1;
    this->timerCallbackSubId =
        this->deathCallbackSubId;
    this->bossTimer = 0;

    if (((this->flags1 >> ENEMY_FLAG_TIMEOUT_SPELL_SHIFT) & 1) == 0)
    {
        PrepareSpellcardForTimerCallback(&g_Spellcard);
        g_BulletManager.RemoveAllBullets(4);
    }

    if (((this->flags1 >> ENEMY_FLAG_BOSS_SHIFT) & 1) != 0 &&
        g_Player.playerState == 0)
    {
        g_Player.timer = 70;
        g_Player.playerState = 3;
    }

    reinterpret_cast<EclOperands::EnemyOverlay *>(this)->DetachEnemyChain(0);
    enemyCursor = &g_EnemyManager.enemies[0];
    for (j = 0; j < 480; j++, enemyCursor++)
    {
        if ((enemyCursor->flags1 & ENEMY_FLAG_ACTIVE) == 0)
            continue;
        if (((enemyCursor->flags1 >> ENEMY_FLAG_BOSS_SHIFT) & 1) != 0)
            continue;
        enemyCursor->life = 0;
        if (enemyCursor->deathCallbackSubId >= 0)
        {
            g_EclManager.CallEclSub(
                reinterpret_cast<EnemyEclContext *>(&enemyCursor->mainEclContextStorage),
                enemyCursor->deathCallbackSubId);
            enemyCursor->deathCallbackSubId = -1;
        }
    }

    for (selectedOrK = 0; selectedOrK < 4; selectedOrK++)
    {
        if (this->childEclBlocks[selectedOrK] != NULL)
        {
            g_ZunMemory.Free(this->childEclBlocks[selectedOrK]);
            this->childEclBlocks[selectedOrK] = NULL;
        }
    }

    this->bulletSpawnDescriptor = g_EnemyManager.spawnTemplate.bulletSpawnDescriptor;
    this->shootIntervalFrames = 0;
    this->ResetBulletRankInfluence();
    this->activeEclCallStackDepth = 0;
    this->flags2 &= ~ENEMY_FLAG2_DAMAGE_FEEDBACK_MASK;
    return 1;
    }
    return 0;
}

// FUNCTION: th08 0x42bc50
void __fastcall PrepareSpellcardForTimerCallback(Spellcard *spellcard)
{
    spellcard->flags &= ~SPELLCARD_FLAG_CAPTURE_VALID;
    spellcard->flags |= SPELLCARD_FLAG_TIMER_CALLBACK_TRANSITION;
    spellcard->bonusProgress = 0;
}

// FUNCTION: th08 0x42bc90
void Enemy::ReleaseChildEclBlocks()
{
    for (i32 i = 0; i < 4; i++)
    {
        if (this->childEclBlocks[i] != NULL)
        {
            g_ZunMemory.Free(this->childEclBlocks[i]);
            this->childEclBlocks[i] = NULL;
        }
    }
}

// FUNCTION: th08 0x42bcf0
#pragma var_order(i, this)
void Enemy::Despawn()
{
    i32 i;

    reinterpret_cast<EclOperands::EnemyOverlay *>(this)->DetachEnemyChain(0);

    if (((this->flags1 >> ENEMY_FLAG_DEATH_MODE_SHIFT) & 7) == 0)
        this->flags1 &= ~ENEMY_FLAG_ACTIVE;

    if (((this->flags1 >> ENEMY_FLAG_BOSS_SHIFT) & 1) != 0 && this->bossSlot < 4)
    {
        g_Gui.SetBossPresent(false);
        g_EnemyManager.bosses[this->bossSlot] = NULL;
        this->flags1 &= ~ENEMY_FLAG_BOSS;
        g_AsciiManager.SetBossMarkerInterrupt(
            this->bossSlot, 2);
        g_AsciiManager.SetBossMarkerPosition(
            this->bossSlot,
            reinterpret_cast<D3DXVECTOR3 *>(&Float3(-999.0f, -999.0f, 0.0f)));
    }

    if (this->attachedEffectCount != 0)
        this->ReleaseAttachedEffects();

    if (((this->flags1 >> ENEMY_FLAG_BOSS_SHIFT) & 1) != 0)
        g_EnemyManager.bosses[this->bossSlot] = NULL;

    g_ReplayManager->frameEventFlags |= 0x20;

    if (this->alignmentEffect != NULL)
    {
        this->alignmentEffect->vm.SetInterrupt(3);
        this->alignmentEffect = NULL;
    }

    for (i = 0; i < 4; ++i)
        this->lifeCallbackThresholds[i] = -1;
    this->timerCallbackThresholdFrames = -1;

    this->ReleaseChildEclBlocks();
    if (g_Player.optionHomingTarget == this)
        g_Player.optionHomingTarget = NULL;
}

// FUNCTION: th08 0x42bea0
#pragma var_order(i, position)
void Enemy::DropItems(i32 mode)
{
    Float3 position;
    i32 i;
    if (this->itemDropType >= 0)
    {
        g_EffectManager.SpawnEffect(
            this->deathAnm2 + 4,
            reinterpret_cast<D3DXVECTOR3 *>(&this->worldPosition), 3, -1);
        g_ItemManager.SpawnItem(&this->worldPosition,
                                static_cast<ItemType>(this->itemDropType),
                                mode != 0);
    }
    else if (this->itemDropType == -1)
    {
        if ((g_EnemyManager.enemyDropCounter % 3) == 0)
        {
            g_EffectManager.SpawnEffect(
                this->deathAnm2 + 4,
                reinterpret_cast<D3DXVECTOR3 *>(&this->worldPosition), 6, -1);
            g_ItemManager.SpawnItem(&this->worldPosition,
                                    static_cast<ItemType>(g_EnemyDropSchedule[g_EnemyManager.enemyDropScheduleIndex]),
                                    mode != 0);
            ++g_EnemyManager.enemyDropScheduleIndex;
            if (g_EnemyManager.enemyDropScheduleIndex >= 32)
                g_EnemyManager.enemyDropScheduleIndex = 0;
        }
        ++g_EnemyManager.enemyDropCounter;
    }

    if (this->powerOrPointItemDropCount != 0)
    {
        for (i = 0; i < this->powerOrPointItemDropCount; i++)
        {
            position = this->worldPosition;
            position.operator float *()[0] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            position.operator float *()[1] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            if (g_GameManager.GetPower() < 128)
                g_ItemManager.SpawnItem(&position, ITEM_POWER_SMALL, 0);
            else
                g_ItemManager.SpawnItem(&position, ITEM_POINT, 0);
        }
        this->powerOrPointItemDropCount = 0;
    }

    if (this->pointItemDropCount != 0)
    {
        for (i = 0; i < this->pointItemDropCount; i++)
        {
            position = this->worldPosition;
            position.operator float *()[0] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            position.operator float *()[1] += g_Rng.GetRandomF32() * 128.0f - 64.0f;
            g_ItemManager.SpawnItem(&position, ITEM_POINT, 0);
        }
        this->pointItemDropCount = 0;
    }
}

// FUNCTION: th08 0x42c180
void Enemy::ClampPosition()
{
    if (((this->flags1 >> ENEMY_FLAG_CLAMP_POSITION_SHIFT) & 1) != 0)
    {
        if ((*reinterpret_cast<D3DXVECTOR3 *>(&this->position))[0] < this->movementBounds.lower.x)
            (*reinterpret_cast<D3DXVECTOR3 *>(&this->position))[0] = this->movementBounds.lower.x;
        else if ((*reinterpret_cast<D3DXVECTOR3 *>(&this->position))[0] > this->movementBounds.upper.x)
            (*reinterpret_cast<D3DXVECTOR3 *>(&this->position))[0] = this->movementBounds.upper.x;

        if ((*reinterpret_cast<D3DXVECTOR3 *>(&this->position))[1] < this->movementBounds.lower.y)
            (*reinterpret_cast<D3DXVECTOR3 *>(&this->position))[1] = this->movementBounds.lower.y;
        else if ((*reinterpret_cast<D3DXVECTOR3 *>(&this->position))[1] > this->movementBounds.upper.y)
            (*reinterpret_cast<D3DXVECTOR3 *>(&this->position))[1] = this->movementBounds.upper.y;
    }
}

// FUNCTION: th08 0x42c290
#pragma var_order(collisionSize)
void Enemy::CheckPlayerCollision(Float3 *position, Float3 *size)
{
    Float3 collisionSize;

    collisionSize = *size / 0.7f;
    if (((this->flags1 >> ENEMY_FLAG_SPECIAL_INTERACTION_SHIFT) & 1) != 0 &&
        this->bossTimer.HasTicked() && this->bossTimer % 6 == 0)
    {
        g_Player.CheckGrazeCollision(position, &collisionSize);
    }

    if (g_GameManager.shotType == 0 || g_GameManager.shotType == 4)
    {
        if (this->HasAttachedEnemy())
            return;
    }

    {
        collisionSize = *size / 1.5f;
        if (g_Player.CheckLethalCollision(position, &collisionSize) == 1)
        {
            if (((this->flags1 >> ENEMY_FLAG_BOSS_SHIFT) & 1) == 0 &&
                ((this->flags1 >> ENEMY_FLAG_SPECIAL_INTERACTION_SHIFT) & 1) == 0)
            {
                this->life -= 10;
            }
        }
    }
}

// FUNCTION: th08 0x42c3b0
#pragma var_order(interval, this)
void EnemyManager::UpdateSubrank()
{
    i32 interval;

    if (!g_Gui.IsDialoguePresent())
    {
        interval = 2400;
        interval -= g_GameManager.GetLives() * 4 * 60;
        if (this->timer.HasTicked())
        {
            if ((i32)this->timer % interval == 0)
                g_GameManager.IncreaseSubrank(100);
        }
    }
}

// FUNCTION: th08 0x42c420
void Enemy::UpdateYoukaiAlignment()
{
    if (((this->flags1 >> ENEMY_FLAG_YOUKAI_ALIGNED_SHIFT) & 1) == 0)
    {
        if (g_Player.IsYoukai())
        {
            g_EffectManager.SpawnEffect(31, reinterpret_cast<D3DXVECTOR3 *>(&this->worldPosition), 1, 0x80303080);
            if (this->alignmentEffect != NULL)
                this->alignmentEffect->vm.SetInterrupt(2);
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(40), 0);
            this->drawGroup = 0;
        }

        if (((this->flags2 >> ENEMY_FLAG2_FORM_EFFECT_SHIFT) & 1) != 0 &&
            this->bossTimer.IsPeriodic(2))
        {
            g_EffectManager.SpawnEffect(38, reinterpret_cast<D3DXVECTOR3 *>(&this->worldPosition), 1, -1);
        }
    }
    else
    {
        if (!g_Player.IsYoukai())
        {
            g_EffectManager.SpawnEffect(30, reinterpret_cast<D3DXVECTOR3 *>(&this->worldPosition), 1, 0x80803030);
            if (this->alignmentEffect != NULL)
                this->alignmentEffect->vm.SetInterrupt(1);
            g_SoundPlayer.PlaySoundByIdx(static_cast<SoundIdx>(39), 0);
            this->drawGroup = 2;
        }
    }

    reinterpret_cast<EnemyFlag1Bits *>(&this->flags1)->youkaiAligned = g_Player.IsYoukai();
    this->eclDifficultyMaskOverride = g_Player.IsYoukai() ? 64 : 32;
}

// FUNCTION: th08 0x42c590
#pragma var_order(result, enemyManager)
ZunResult EnemyManager::RegisterChain()
{
    EnemyManager *enemyManager = &g_EnemyManager;
    i32 result = 0;

    enemyManager->Initialize();

    g_EnemyManagerCalcChain.SetCallback((ChainCallback)EnemyManager::OnUpdate);
    g_EnemyManagerCalcChain.addedCallback = (ChainLifetimeCallback)EnemyManager::AddedCallback;
    g_EnemyManagerCalcChain.deletedCallback = (ChainLifetimeCallback)EnemyManager::DeletedCallback;
    g_EnemyManagerCalcChain.arg = enemyManager;
    if (g_Chain.AddToCalcChain(&g_EnemyManagerCalcChain, CHAIN_PRIO_CALC_ENEMYMANAGER) != ZUN_SUCCESS)
        return ZUN_ERROR;

    g_EnemyManagerDrawChainHighPrio.SetCallback((ChainCallback)EnemyManager::OnDrawHighPrio);
    g_EnemyManagerDrawChainHighPrio.arg = enemyManager;
    if (g_Chain.AddToDrawChain(&g_EnemyManagerDrawChainHighPrio, CHAIN_PRIO_DRAW_ENEMYMANAGER_HIGH_PRIO) != ZUN_SUCCESS)
        return ZUN_ERROR;

    g_EnemyManagerDrawChainLowPrio.SetCallback((ChainCallback)EnemyManager::OnDrawLowPrio);
    g_EnemyManagerDrawChainLowPrio.arg = enemyManager;
    if (g_Chain.AddToDrawChain(&g_EnemyManagerDrawChainLowPrio, CHAIN_PRIO_DRAW_ENEMYMANAGER_LOW_PRIO) != ZUN_SUCCESS)
        return ZUN_ERROR;

    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x42deb0
void Enemy::IntegrateVelocity()
{
    this->lastFrameDisplacement = this->position - this->previousPosition;
    this->previousPosition = this->position;
    if (((this->flags1 >> ENEMY_FLAG_MIRROR_MOVEMENT_X_SHIFT) & 1) == 0)
        this->position.x += g_Supervisor.framerateMultiplier * this->velocity.x;
    else
        this->position.x -= g_Supervisor.framerateMultiplier * this->velocity.x;
    this->position.y += g_Supervisor.framerateMultiplier * this->velocity.y;
    this->position.z += g_Supervisor.framerateMultiplier * this->velocity.z;
}

// FUNCTION: th08 0x42e010
#pragma var_order(effect, i, this)
void Enemy::UpdateEffects()
{
    Effect *effect;
    i32 i;

    for (i = 0; i < this->attachedEffectCount; ++i)
    {
        effect = this->attachedEffects[i];
        if (effect == NULL)
            continue;

        effect->vm.flag1 =
            ((this->flags1 >> ENEMY_FLAG_NO_SPRITE_SHIFT) & 1) == 0;
        effect->vector5 = this->position;

        if (effect->radius < this->attachedEffectDistance)
        {
            effect->radius += 0.3f;
        }
        else
        {
            effect->radius = this->attachedEffectDistance;
        }

        effect->angle = AddNormalizeAngle(effect->angle, 0.031415928f);
    }
}

// FUNCTION: th08 0x42e120
ChainCallbackResult EnemyManager::OnDrawHighPrio(EnemyManager *enemyManager)
{
    return enemyManager->OnDrawImpl(0, 2);
}

// FUNCTION: th08 0x42e140
#pragma var_order(savedScaleY, savedScaleX, i, savedColor, vm, k, enemy, halfWidth, halfCenter, vertexCount, sinAngle, uv, previousAngle, vertices, uvStep, angle, cosAngle, uvSpan, this, drawGroup)
ChainCallbackResult __fastcall EnemyManager::OnDrawImpl(i32 drawGroup, i32 chainPriority)
{
    f32 savedScaleY;
    f32 savedScaleX;
    i32 i;
    u32 savedColor;
    AnmVm *vm;
    i32 k;
    Enemy *enemy;
    f32 halfWidth;
    f32 halfCenter;
    i32 vertexCount;
    f32 sinAngle;
    f32 uv;
    f32 previousAngle;
    VertexTex1DiffuseXyzrhw *vertices;
    f32 uvStep;
    f32 angle;
    f32 cosAngle;
    f32 uvSpan;

    for (i = drawGroup; i < chainPriority; ++i)
    {
        enemy = this->drawGroupHeads[i];
        while (enemy != NULL)
        {
            vm = &enemy->secondaryVms[0];
            for (k = 0; k < 1; ++k, ++vm)
            {
                if (vm->scriptIndex >= 0)
                {
                    if (vm->type)
                        vm->SetZRotation(enemy->movementAngle);

                    if (((enemy->flags2 >>
                          ENEMY_FLAG2_EXTRA_VM_FIXED_OFFSET_SHIFT) & 1) == 0)
                        vm->pos = enemy->worldPosition + vm->pos2;
                    else
                        vm->pos = enemy->worldPosition +
                                  enemy->vm.pos2;

                    vm->pos.z = 0.3f;
                    vm->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                    vm->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                    g_AnmManager->Draw2D(vm);
                }
            }

            if (((enemy->flags1 >>
                  ENEMY_FLAG_ROTATE_ANM_WITH_MOVEMENT_SHIFT) & 1) != 0)
                enemy->vm.SetZRotation(
                    enemy->movementAngle);

            enemy->vm.pos =
                enemy->worldPosition + enemy->vm.pos2;
            enemy->vm.pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
            enemy->vm.pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
            enemy->vm.pos.z = 0.25f;

            if (enemy->trailFlags)
            {
                *reinterpret_cast<Float2 *>(&savedScaleX) = enemy->vm.scale;
                savedColor = enemy->vm.color1.d3dColor;

                if ((enemy->trailFlags & ENEMY_TRAIL_RENDER_AS_STRIP) == 0)
                {
                    for (k = enemy->trailHistoryLength - 1; k > 0;
                         k -= enemy->trailSampleStride)
                    {
                        if (enemy->trailSamples[k].position.x < -990.0f)
                            continue;

                        if (((enemy->flags1 >>
                              ENEMY_FLAG_ROTATE_ANM_WITH_MOVEMENT_SHIFT) & 1) != 0)
                                enemy->vm.SetZRotation(
                                    enemy->trailSamples[k].angle);

                            if ((enemy->trailFlags & ENEMY_TRAIL_TAPER) != 0)
                                enemy->vm.scale.x =
                                    savedScaleX - (f32)k * savedScaleX /
                                                      (f32)enemy->trailHistoryLength;

                            if ((enemy->trailFlags & ENEMY_TRAIL_FADE) != 0)
                                enemy->vm.color1.a =
                                    reinterpret_cast<u8 *>(&savedColor)[3] -
                                    reinterpret_cast<u8 *>(&savedColor)[3] * k /
                                        enemy->trailHistoryLength;

                            enemy->vm.pos =
                                enemy->trailSamples[k].position +
                                enemy->vm.pos2;
                            enemy->vm.pos.z = 0.3f;
                            enemy->vm.pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                            enemy->vm.pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                        g_AnmManager->Draw2D(&enemy->vm);
                    }
                }
                else
                {
                    vertexCount = 0;
                    for (k = 0; k < enemy->trailHistoryLength;
                         k += enemy->trailSampleStride)
                    {
                        if (enemy->trailSamples[k].position.x < -990.0f)
                            break;
                        vertexCount += 2;
                    }

                    if (vertexCount > 2)
                    {
                        uvSpan = enemy->vm.loadedSprite->uvEnd.x -
                                 enemy->vm.loadedSprite->uvStart.x;
                        uvStep = uvSpan / ((vertexCount + 1) / 2 - 1);
                        uv = enemy->vm.loadedSprite->uvEnd.x +
                             enemy->vm.uvScrollPos.x;
                        vertices = enemy->trailVertices;

                        for (k = 0; k < enemy->trailHistoryLength;
                             k += enemy->trailSampleStride, uv -= uvStep)
                        {
                            if (enemy->trailSamples[k].position.x < -990.0f)
                                break;

                            if (k == 0)
                            {
                                angle = enemy->trailSamples[0].angle;
                            }
                            else
                            {
                                angle = InterpolateWrappedAngle(
                                    enemy->trailSamples[k - 1].angle,
                                    enemy->trailSamples[k].angle, 0.5f);
                            }

                            if ((enemy->trailFlags & ENEMY_TRAIL_TAPER) != 0 && k > 0 &&
                                k + enemy->trailSampleStride <
                                    enemy->trailHistoryLength)
                            {
                                sinAngle = InterpolateWrappedAngle(
                                    enemy->trailSamples[
                                        k + enemy->trailSampleStride - 1].angle,
                                    enemy->trailSamples[
                                        enemy->trailSampleStride].angle,
                                    0.5f);
                                if (fabsf(previousAngle - angle) < 0.00001f &&
                                    fabsf(angle - sinAngle) < 0.00001f)
                                {
                                    vertexCount -= 2;
                                    continue;
                                }
                            }

                            previousAngle = angle;
                            sinAngle = sinf(angle);
                            cosAngle = cosf(angle);
                            halfCenter = 0.0f;
                            halfWidth = savedScaleY *
                                        enemy->vm.loadedSprite->heightPx / 2.0f;
                            if ((enemy->trailFlags & ENEMY_TRAIL_TAPER) != 0)
                            {
                                angle = 1.0f - (f32)k / (f32)enemy->trailHistoryLength;
                                halfCenter *= angle;
                                halfWidth *= angle;
                            }

                            vertices[1].diffuse = enemy->vm.color1.d3dColor;
                            vertices[0].diffuse = vertices[1].diffuse;
                            if ((enemy->trailFlags & ENEMY_TRAIL_FADE) != 0)
                            {
                                reinterpret_cast<u8 *>(&vertices[1].diffuse)[3] =
                                    reinterpret_cast<u8 *>(&savedColor)[3] -
                                    reinterpret_cast<u8 *>(&savedColor)[3] * k /
                                        enemy->trailHistoryLength;
                                reinterpret_cast<u8 *>(&vertices[0].diffuse)[3] =
                                    reinterpret_cast<u8 *>(&vertices[1].diffuse)[3];
                            }

                            vertices[0].pos = enemy->trailSamples[k].position;
                            vertices[0].pos.x += cosAngle * halfCenter - sinAngle * halfWidth + 32.0f;
                            vertices[0].pos.y += sinAngle * halfCenter + cosAngle * halfWidth + 16.0f;
                            vertices[0].textureUV.x = uv;
                            vertices[0].textureUV.y =
                                enemy->vm.loadedSprite->uvStart.y +
                                enemy->vm.uvScrollPos.y;
                            ++vertices;

                            vertices[0].pos = enemy->trailSamples[k].position;
                            vertices[0].pos.x += cosAngle * halfCenter + sinAngle * halfWidth + 32.0f;
                            vertices[0].pos.y += sinAngle * halfCenter - cosAngle * halfWidth + 16.0f;
                            vertices[0].textureUV.x = uv;
                            vertices[0].textureUV.y =
                                enemy->vm.loadedSprite->uvEnd.y +
                                enemy->vm.uvScrollPos.y;
                            ++vertices;
                        }

                        if (vertexCount > 2)
                            g_AnmManager->DrawVertices(
                                &enemy->vm,
                                enemy->trailVertices, vertexCount);
                    }
                }

                enemy->vm.scale = *reinterpret_cast<Float2 *>(&savedScaleX);
                enemy->vm.color1.d3dColor = savedColor;
            }

            if ((enemy->trailFlags & ENEMY_TRAIL_HIDE_HEAD_ANM) == 0 &&
                ((enemy->flags1 >>
                  ENEMY_FLAG_HIDE_PRIMARY_ANM_SHIFT) & 1) == 0)
            {
                g_AnmManager->Draw2D(&enemy->vm);
            }

            for (k = 1; k < 2; ++k, ++vm)
            {
                if (vm->scriptIndex >= 0)
                {
                    if (vm->type)
                        vm->SetZRotation(-enemy->movementAngle);

                    if (((enemy->flags2 >>
                          ENEMY_FLAG2_EXTRA_VM_FIXED_OFFSET_SHIFT) & 1) == 0)
                        vm->pos = enemy->worldPosition + vm->pos2;
                    else
                        vm->pos = enemy->worldPosition +
                                  enemy->vm.pos2;

                    vm->pos.z = 0.3f;
                    vm->pos.x += g_GameManager.arcadeRegionTopLeftPos.x;
                    vm->pos.y += g_GameManager.arcadeRegionTopLeftPos.y;
                    g_AnmManager->Draw2D(vm);
                }
            }

            enemy = enemy->nextInDrawGroup;
        }
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

// FUNCTION: th08 0x42eb10
#pragma var_order(wrapDelta, shortDelta)
f32 __stdcall InterpolateWrappedAngle(f32 angle1, f32 angle2, f32 factor)
{
    f32 shortDelta;
    f32 wrapDelta;

    if (angle1 < angle2)
    {
        shortDelta = angle2 - angle1;
        wrapDelta = angle1 + ZUN_2PI - angle2;
    }
    else
    {
        shortDelta = angle1 - angle2;
        wrapDelta = angle2 + ZUN_2PI - angle1;
        angle1 = angle2;
    }

    if (shortDelta < wrapDelta)
        return shortDelta * factor + angle1;
    return wrapDelta * factor + angle1;
}

// FUNCTION: th08 0x42eb90
ChainCallbackResult EnemyManager::OnDrawLowPrio(EnemyManager *enemyManager)
{
    ChainCallbackResult result;

    if (g_GameManager.flags.deathbombFreezeActive)
    {
        g_AnmManager->SetMixColor(0xfff01010);
    }

    result = enemyManager->OnDrawImpl(2, 4);

    if (g_GameManager.flags.deathbombFreezeActive)
    {
        g_AnmManager->SetMixColorDefault();
    }

    return result;
}

// FUNCTION: th08 0x42ebf0
#pragma var_order(enemy, savedEcl0, savedEcl1, markerPosition, enemyManager)
ZunResult EnemyManager::AddedCallback(EnemyManager *enemyManager)
{
    Enemy *enemy = &enemyManager->enemies[0];
    i32 savedEcl0;
    i32 savedEcl1;

    if (IsResourceReloadEnabled())
    {
        enemyManager->enemyAnm = g_AnmManager->PreloadAnm(7, "enemy.anm");
        if (enemyManager->enemyAnm == NULL)
        {
            return ZUN_ERROR;
        }
    }
    else
    {
        enemyManager->enemyAnm = g_AnmManager->GetAnm(7);
    }

    if (!IsDisableResourceReload())
    {
        if (!g_GameManager.flags.isSpellPractice ||
            g_GameManager.currentSpellCardNumber < 0xCD)
        {
            enemyManager->alternateEnemyAnm =
                g_AnmManager->PreloadAnm(8, g_StageEnemyAnms[g_GameManager.currentStage]);
            if (enemyManager->alternateEnemyAnm == NULL)
            {
                return ZUN_ERROR;
            }
        }
        else
        {
            enemyManager->alternateEnemyAnm =
                g_AnmManager->PreloadAnm(
                    8, g_SpellEnemyAnms[g_GameManager.currentSpellCardNumber - 0xCD]);
            if (enemyManager->alternateEnemyAnm == NULL)
            {
                return ZUN_ERROR;
            }
        }
    }
    else
    {
        enemyManager->alternateEnemyAnm = g_AnmManager->GetAnm(8);
    }

    if (!IsDisableResourceReload())
    {
#ifdef TH08_MODERN_PORT
        memset(&g_EclManager, 0, sizeof(g_EclManager));
        memset(&EclRunLowProposal::g_EclCallParameters, 0,
               sizeof(EclRunLowProposal::g_EclCallParameters));
#else
        memset(&g_EclManager, 0,
               sizeof(g_EclManager) + sizeof(EclRunLowProposal::g_EclCallParameters));
#endif
        if (!g_GameManager.flags.isSpellPractice)
        {
            if (g_EclManager.Load(const_cast<char *>(g_StageEclFiles[g_GameManager.currentStage])) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
        }
        else if (g_GameManager.currentSpellCardNumber >= 0xCD)
        {
            if (g_EclManager.Load(const_cast<char *>(g_SpellEclFiles[
                    g_GameManager.currentSpellCardNumber - 0xCD])) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
        }
        else
        {
            if (g_EclManager.Load(const_cast<char *>(g_StageSpellEclFiles[g_GameManager.currentStage])) !=
                ZUN_SUCCESS)
            {
                return ZUN_ERROR;
            }
        }
    }
    else
    {
        savedEcl0 = reinterpret_cast<i32 *>(&g_EclManager)[0];
        savedEcl1 = reinterpret_cast<i32 *>(&g_EclManager)[1];
#ifdef TH08_MODERN_PORT
        memset(&g_EclManager, 0, sizeof(g_EclManager));
        memset(&EclRunLowProposal::g_EclCallParameters, 0,
               sizeof(EclRunLowProposal::g_EclCallParameters));
#else
        memset(&g_EclManager, 0,
               sizeof(g_EclManager) + sizeof(EclRunLowProposal::g_EclCallParameters));
#endif
        reinterpret_cast<i32 *>(&g_EclManager)[0] = savedEcl0;
        reinterpret_cast<i32 *>(&g_EclManager)[1] = savedEcl1;
    }

    enemyManager->enemyDropCounter = g_Rng.GetRandomU16InRange(3);
    enemyManager->enemyDropScheduleIndex = g_Rng.GetRandomU16InRange(8);
    D3DXVECTOR3 markerPosition(-999.0f, -999.0f, -999.0f);
    g_AsciiManager.SetBossMarkerPosition(0, &markerPosition);
    g_AsciiManager.SetBossMarkerPosition(1, &markerPosition);
    g_AsciiManager.SetBossMarkerPosition(2, &markerPosition);
    g_AsciiManager.SetBossMarkerPosition(3, &markerPosition);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x42ee80
#pragma var_order(i, enemy, markerPosition, enemyManager)
ZunResult EnemyManager::DeletedCallback(EnemyManager *enemyManager)
{
    Enemy *enemy = &enemyManager->enemies[0];
    i32 i = 0;

    for (; i < 0x1E0; ++i, enemy++)
    {
        enemy->ReleaseChildEclBlocks();
    }

    if (!IsDisableResourceReload())
    {
        g_AnmManager->ReleaseAnm(8);
    }
    if (IsBulletManagerAnmReleaseRequired())
    {
        g_AnmManager->ReleaseAnm(7);
    }
    if (!IsDisableResourceReload())
    {
        g_EclManager.Unload();
    }

    D3DXVECTOR3 markerPosition(-999.0f, -999.0f, -999.0f);
    g_AsciiManager.SetBossMarkerPosition(0, &markerPosition);
    g_AsciiManager.SetBossMarkerPosition(1, &markerPosition);
    g_AsciiManager.SetBossMarkerPosition(2, &markerPosition);
    g_AsciiManager.SetBossMarkerPosition(3, &markerPosition);
    return ZUN_SUCCESS;
}

// FUNCTION: th08 0x42ef70
void EnemyManager::CutChain()
{
    g_Chain.Cut(&g_EnemyManagerCalcChain);
    g_Chain.Cut(&g_EnemyManagerDrawChainHighPrio);
    g_Chain.Cut(&g_EnemyManagerDrawChainLowPrio);
}

// FUNCTION: th08 0x42efb0
#pragma var_order(score, totalScore, enemy, enemyIndex, itemIndex, this)
i32 EnemyManager::KillAllNonBossEnemies(i32 maxScore, i32 initialScore)
{
    i32 itemIndex;
    i32 enemyIndex;
    u8 *enemy;
    i32 totalScore;
    i32 score;

    enemy = reinterpret_cast<u8 *>(&this->enemies[0]);
    totalScore = initialScore;
    score = 2000;
    for (enemyIndex = 0; enemyIndex < 480; enemyIndex++, enemy += 0x53D0)
    {
        if ((reinterpret_cast<Enemy *>(enemy)->flags1 & ENEMY_FLAG_ACTIVE) == 0)
        {
            continue;
        }
        if (((reinterpret_cast<Enemy *>(enemy)->flags1 >> ENEMY_FLAG_BOSS_SHIFT) & 1) != 0)
        {
            continue;
        }
        if (((reinterpret_cast<Enemy *>(enemy)->flags2 >> ENEMY_FLAG2_NO_DEATH_SHIFT) & 1) != 0)
        {
            continue;
        }

        reinterpret_cast<Enemy *>(enemy)->life = 0;
        if (((reinterpret_cast<Enemy *>(enemy)->flags1 >>
              ENEMY_FLAG_SPECIAL_INTERACTION_SHIFT) & 1) != 0)
        {
            reinterpret_cast<Enemy *>(enemy)->worldPosition =
                reinterpret_cast<Enemy *>(enemy)->position +
                reinterpret_cast<Enemy *>(enemy)->positionOffset;
            g_ItemManager.SpawnItem(&reinterpret_cast<Enemy *>(enemy)->worldPosition, ITEM_POINT_STAR,
                                    ITEM_STATE_AUTOCOLLECT);
            g_AsciiManager.CreateScorePopup(&reinterpret_cast<Enemy *>(enemy)->worldPosition, score,
                                            score >= maxScore ? -256 : -1);
            totalScore += score;
            score += 30;
            if (score > maxScore)
            {
                score = maxScore;
            }

            if (reinterpret_cast<Enemy *>(enemy)->trailFlags != 0)
            {
                for (itemIndex = 0; itemIndex < reinterpret_cast<Enemy *>(enemy)->trailHistoryLength; itemIndex += 6)
                {
                    g_ItemManager.SpawnItem(
                        &reinterpret_cast<Enemy *>(enemy)->trailSamples[itemIndex].position, ITEM_POINT_STAR,
                        ITEM_STATE_AUTOCOLLECT);
                    g_AsciiManager.CreateScorePopup(
                        &reinterpret_cast<Enemy *>(enemy)->trailSamples[itemIndex].position, score,
                        score >= maxScore ? -256 : -1);
                    totalScore += score;
                    score += 30;
                    if (score > maxScore)
                    {
                        score = maxScore;
                    }
                }
            }
        }

        reinterpret_cast<Enemy *>(enemy)->DetachFromParentChain();
        if (reinterpret_cast<Enemy *>(enemy)->deathCallbackSubId >= 0)
        {
            g_EclManager.CallEclSub(
                reinterpret_cast<EnemyEclContext *>(
                    &reinterpret_cast<Enemy *>(enemy)->mainEclContextStorage),
                reinterpret_cast<Enemy *>(enemy)->deathCallbackSubId);
            reinterpret_cast<Enemy *>(enemy)->deathCallbackSubId = -1;
        }
    }

    return totalScore;
}

// FUNCTION: th08 0x42f1f0
i32 EnemyManager::HasBoss()
{
    i32 i;
    for (i = 0; i < 8; i++)
    {
        if (this->bosses[i] != NULL)
            return 1;
    }
    return 0;
}

// FUNCTION: th08 0x449f50
EclTimeline::EclTimeline() {}

} /* namespace th08 */
