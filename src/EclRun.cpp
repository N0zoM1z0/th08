#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "AsciiManager.hpp"
#include "Gui.hpp"
#include "BulletManager.hpp"
#include "Background.hpp"
#include "GameManager.hpp"
#include "ItemManager.hpp"
#include "Player.hpp"
#include "Spellcard.hpp"

#include <string.h>

#define TH08_ECL_RUN_DECLARATIONS_ONLY
#include "EclRunLow.inl"
#include "EclRunHigh.inl"
#undef TH08_ECL_RUN_DECLARATIONS_ONLY

namespace th08
{

// The low/high opcode bodies are included lexically below so VC7 can reproduce
// RunEcl's target handler order, shared labels, locals, and stack frame.
#undef TH08_ECL_CONTEXT_ENEMY
#undef TH08_ECL_CONTEXT_INSTRUCTION
#undef TH08_ECL_CONTEXT_CHILD
#define TH08_ECL_CONTEXT_ENEMY(unusedContext) (enemy)
#define TH08_ECL_CONTEXT_INSTRUCTION(unusedContext) (instruction)
#define TH08_ECL_CONTEXT_CHILD(unusedContext) (activeChildContext)

// FUNCTION: th08 0x004184B0
ZunResult EclManager::RunEcl(Enemy *enemy)
{
    using namespace EclRunLow;
    using namespace EclRunHigh;

    EclRawInstruction *instruction;
    i32 activeChildContext = -1;
    i32 lhsInt;

    enemy->activeEclCallStack = enemy->mainEclCallStackStorage;
    enemy->activeEclContext = &enemy->mainEclContextStorage;
    enemy->activeEclCallStackDepth = enemy->mainEclCallStackDepth;

restart_context:
    instruction = enemy->activeEclContext->currentInstr;

    for (;;)
    {
        if (enemy->pendingEclSubroutineIndex >= 0)
            goto enter_subroutine;

low_redispatch_instruction:
        *D3DXVECTOR3_PTR(&enemy->worldPosition) =
            *D3DXVECTOR3_PTR(&enemy->position) +
            *D3DXVECTOR3_PTR(&enemy->positionOffset);

        if ((int)enemy->activeEclContext->secondaryTime > 0)
        {
            enemy->activeEclContext->secondaryTime--;
            enemy->activeEclContext->time--;
            break;
        }

        if (enemy->activeEclContext->time == instruction->time)
        {
            if ((instruction->difficultyMask &
                 (static_cast<u32>(g_GameManager.difficultyMask) |
                  enemy->eclDifficultyMaskOverride)) !=
                (static_cast<u32>(g_GameManager.difficultyMask) |
                 enemy->eclDifficultyMaskOverride))
            {
                goto low_advance_instruction;
            }

            {
#define context (enemy->activeEclContext)
#define ctx unusedContext
#define TH08_ECL_RUN_LOW_BODY
#define TH08_ECL_RUN_HIGH_BODY
#define TH08_ECL_RUN_SHARED_SWITCH
            switch (instruction->opcode)
            {
                {
#include "EclRunLow.inl"
                }
                {
#include "EclRunHigh.inl"
                }
            default:
                break;
            }
#undef TH08_ECL_RUN_SHARED_SWITCH
#undef TH08_ECL_RUN_HIGH_BODY
#undef TH08_ECL_RUN_LOW_BODY
#undef ctx
#undef context
            }

low_advance_instruction:
            instruction = reinterpret_cast<EclRawInstruction *>(
                reinterpret_cast<u8 *>(instruction) + instruction->nextOffset);
            goto low_redispatch_instruction;

            break;
        }

        break;
    }

    // Target 0x0041E7F8..0x0041ECBD is part of RunEcl itself.  Keep the
    // complete frame tail lexical so VC7 can share RunEcl's locals and emit
    // the observed in-function easing switch and child-context back edge.
    if (enemy->life > 0)
    {
#pragma var_order(i, progress, restorePosition, entry, savedPosition)
        i32 i;
        f32 progress;
        i32 restorePosition = 0;
        EnemyEclInterpolationSlot *entry =
            enemy->activeEclContext->interpolationSlots;
        Float3 savedPosition = enemy->position;

        if (enemy->activeEclContext->perFrameCallback)
            enemy->activeEclContext->perFrameCallback(
                enemy, enemy->activeEclContext->perFrameInstruction);

        for (i = 0; i < 8; ++i, ++entry)
        {
            if (entry->callback)
            {
                entry->timer++;
                if (entry->timer >= entry->duration)
                    entry->timer = entry->duration;

                progress = static_cast<f32>(entry->timer) / entry->duration;
                switch (entry->easing)
                {
                case 1: progress = progress * progress; break;
                case 2: progress = progress * progress * progress; break;
                case 3: progress = progress * progress * progress * progress; break;
                case 4:
                    progress = 1.0f - progress;
                    progress = progress * progress;
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

                entry->callback(enemy, entry, progress);
                if (entry->timer >= entry->duration)
                    entry->callback = 0;

                if (entry->affectedVariable == 10042.0f ||
                    entry->affectedVariable == 10043.0f ||
                    entry->affectedVariable == 10044.0f)
                    restorePosition = 1;
            }
        }

        if (restorePosition)
        {
            enemy->velocity.x = enemy->position.x - savedPosition.x;
            enemy->velocity.y = enemy->position.y - savedPosition.y;
            enemy->movementAngle = VectorAngle(enemy->velocity.y, enemy->velocity.x);
            enemy->position = savedPosition;
        }
    }

    if (activeChildContext == -1)
        enemy->mainEclCallStackDepth = enemy->activeEclCallStackDepth;
    else
        enemy->childEclBlocks[activeChildContext]->callStackDepth =
            enemy->activeEclCallStackDepth;

    enemy->activeEclContext->currentInstr = instruction;
    enemy->activeEclContext->time.operator++(0);

low_select_next_context:
    for (i32 next = activeChildContext + 1; next < 4; ++next)
    {
        if (enemy->childEclBlocks[next])
        {
            EnemyChildEclBlock *childContext = enemy->childEclBlocks[next];
            enemy->activeEclCallStack = childContext->callStack;
            enemy->activeEclContext = &childContext->eclContext;
            instruction = enemy->activeEclContext->currentInstr;
            enemy->activeEclContext->childContextSlot = next + 1;
            enemy->activeEclCallStackDepth = childContext->callStackDepth;
            activeChildContext = next;
            goto low_redispatch_instruction;
        }
    }

    enemy->activeEclCallStack = enemy->mainEclCallStackStorage;
    enemy->activeEclContext = &enemy->mainEclContextStorage;
    enemy->UpdateMovement();
    enemy->UpdateShotAndAnm();

    return ZUN_SUCCESS;
}

#undef TH08_ECL_CONTEXT_ENEMY
#undef TH08_ECL_CONTEXT_INSTRUCTION
#undef TH08_ECL_CONTEXT_CHILD

} // namespace th08
