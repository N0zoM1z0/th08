#include "EclManager.hpp"
#include "EclOperands.hpp"

#include <string.h>

#define TH08_ECL_RUN_DECLARATIONS_ONLY
#include "EclRunLow.inl"
#include "EclRunHigh.inl"
#undef TH08_ECL_RUN_DECLARATIONS_ONLY

namespace th08
{

// The dispatcher is now source-complete, while target-address service binding
// remains intentionally separate: subsystem owners can replace provisional
// adapters without changing the recovered opcode/control-flow core.
extern u8 g_TargetDifficulty0160F53C;

#undef TH08_ECL_CONTEXT_ENEMY
#undef TH08_ECL_CONTEXT_INSTRUCTION
#undef TH08_ECL_CONTEXT_API
#undef TH08_ECL_CONTEXT_CHILD
#define TH08_ECL_CONTEXT_ENEMY(unusedContext) (reinterpret_cast<u8 *>(enemy))
#define TH08_ECL_CONTEXT_INSTRUCTION(unusedContext) \
    (reinterpret_cast<RawInstruction *>(instruction))
#define TH08_ECL_CONTEXT_API(unusedContext) \
    (reinterpret_cast<TargetApi *>(this))
#define TH08_ECL_CONTEXT_CHILD(unusedContext) (activeChildContext)

// FUNCTION: th08 0x004184B0
ZunResult EclManager::RunEcl(Enemy *enemy)
{
    using namespace EclRunLowProposal;
    using namespace EclRunHighProposal;

    EclRawInstruction *instruction;
    i32 activeChildContext = -1;
    i32 lhsInt;

    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(enemy) + 0x2CA4) =
        reinterpret_cast<u8 *>(enemy) + 0xA20;
    *reinterpret_cast<u8 **>(reinterpret_cast<u8 *>(enemy) + 0x2CA0) =
        reinterpret_cast<u8 *>(enemy) + 0x7F8;
    *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(enemy) + 0x2CEA) =
        *reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(enemy) + 0x2CE8);

restart_context:
    instruction = (*reinterpret_cast<EnemyEclContext **>(
        reinterpret_cast<u8 *>(enemy) + 0x2CA0))->currentInstr;

    for (;;)
    {
        if (*reinterpret_cast<i16 *>(reinterpret_cast<u8 *>(enemy) + 0x2D30) >= 0)
            goto enter_subroutine;

low_redispatch_instruction:
        *reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(enemy) + 0x2D88) =
            *reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(enemy) + 0x2D34) +
            *reinterpret_cast<D3DXVECTOR3 *>(reinterpret_cast<u8 *>(enemy) + 0x2D40);

        if ((int)(*reinterpret_cast<EnemyEclContext **>(
                reinterpret_cast<u8 *>(enemy) + 0x2CA0))->secondaryTime > 0)
        {
            (*reinterpret_cast<EnemyEclContext **>(
                reinterpret_cast<u8 *>(enemy) + 0x2CA0))->secondaryTime--;
            (*reinterpret_cast<EnemyEclContext **>(
                reinterpret_cast<u8 *>(enemy) + 0x2CA0))->time--;
            break;
        }

        if ((*reinterpret_cast<EnemyEclContext **>(
                reinterpret_cast<u8 *>(enemy) + 0x2CA0))->time == instruction->time)
        {
            if ((instruction->difficultyMask &
                 (*reinterpret_cast<u32 *>(&g_TargetDifficulty0160F53C) |
                  *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(enemy) + 0x3330))) !=
                (*reinterpret_cast<u32 *>(&g_TargetDifficulty0160F53C) |
                 *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(enemy) + 0x3330)))
            {
                goto low_advance_instruction;
            }

            {
#define enemy reinterpret_cast<EclOperands::EnemyOverlay *>(enemy)
#define context (*reinterpret_cast<EnemyEclContext **>( \
    reinterpret_cast<u8 *>(enemy) + 0x2CA0))
#define services (*reinterpret_cast<Services *>(this))
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
#undef services
#undef context
#undef enemy
            }

low_advance_instruction:
            instruction = reinterpret_cast<EclRawInstruction *>(
                reinterpret_cast<u8 *>(instruction) + instruction->nextOffset);
            goto low_redispatch_instruction;

low_select_next_context:
            break;
        }

        break;
    }

    // Target 0x0041E7F8..0x0041ECBD is part of RunEcl itself.  Keep the
    // complete frame tail lexical so VC7 can share RunEcl's locals and emit
    // the observed in-function easing switch and child-context back edge.
    if (TH08_ECL_AT(unusedContext, i32, 0x2DFC) > 0)
    {
        bool restorePosition = false;
        Vec3 savedPosition = TH08_ECL_AT(unusedContext, Vec3, 0x2D34);
        u8 *eclContext = TH08_ECL_CURRENT_CONTEXT(unusedContext);

        if (*(void **)(eclContext + 0x10))
            TH08_ECL_CONTEXT_API(unusedContext)->RunContextCallback(
                *(void **)(eclContext + 0x10), reinterpret_cast<u8 *>(enemy),
                *(void **)(eclContext + 0x14));

        Interpolator *entry = (Interpolator *)(eclContext + 0x9C);
        for (i32 i = 0; i < 8; ++i, ++entry)
        {
            if (!entry->callback)
                continue;

            TH08_ECL_CONTEXT_API(unusedContext)->ResetTimer(entry->timer, 0);
            if (TH08_ECL_CONTEXT_API(unusedContext)->TimerDone(
                    entry->timer, entry->duration))
                TH08_ECL_CONTEXT_API(unusedContext)->SetTimer(
                    entry->timer, entry->duration);

            f32 progress =
                TH08_ECL_CONTEXT_API(unusedContext)->TimerValue(entry->timer) /
                entry->duration;
            f32 inverse;
            switch (entry->easing)
            {
            case 1: progress = progress * progress; break;
            case 2: progress = progress * progress * progress; break;
            case 3: progress = progress * progress * progress * progress; break;
            case 4:
                inverse = 1.0f - progress;
                progress = 1.0f - inverse * inverse;
                break;
            case 5:
                inverse = 1.0f - progress;
                progress = 1.0f - inverse * inverse * inverse;
                break;
            case 6:
                inverse = 1.0f - progress;
                progress = 1.0f - inverse * inverse * inverse * inverse;
                break;
            }

            TH08_ECL_CONTEXT_API(unusedContext)->RunInterpolatorCallback(
                entry->callback, reinterpret_cast<u8 *>(enemy), entry, progress);
            if (TH08_ECL_CONTEXT_API(unusedContext)->TimerDone(
                    entry->timer, entry->duration))
                entry->callback = 0;

            if (entry->affectedVariable == 10042.0f ||
                entry->affectedVariable == 10043.0f ||
                entry->affectedVariable == 10044.0f)
                restorePosition = true;
        }

        if (restorePosition)
        {
            TH08_ECL_AT(unusedContext, f32, 0x2D4C) =
                TH08_ECL_AT(unusedContext, f32, 0x2D34) - savedPosition.x;
            TH08_ECL_AT(unusedContext, f32, 0x2D50) =
                TH08_ECL_AT(unusedContext, f32, 0x2D38) - savedPosition.y;
            TH08_ECL_AT(unusedContext, f32, 0x2D94) =
                TH08_ECL_CONTEXT_API(unusedContext)->VectorAngle(
                    TH08_ECL_AT(unusedContext, f32, 0x2D50),
                    TH08_ECL_AT(unusedContext, f32, 0x2D4C));
            TH08_ECL_AT(unusedContext, Vec3, 0x2D34) = savedPosition;
        }
    }

    if (activeChildContext == -1)
        TH08_ECL_AT(unusedContext, i16, 0x2CE8) = TH08_ECL_AT(unusedContext, i16, 0x2CEA);
    else
        *(i16 *)(TH08_ECL_AT(unusedContext, u8 *, 0x3384 + activeChildContext * 4) + 6) =
            TH08_ECL_AT(unusedContext, i16, 0x2CEA);

    *(RawInstruction **)TH08_ECL_CURRENT_CONTEXT(unusedContext) =
        reinterpret_cast<RawInstruction *>(instruction);
    TH08_ECL_CONTEXT_API(unusedContext)->ResetTimer(
        TH08_ECL_CURRENT_CONTEXT(unusedContext) + 4, 0);

    for (i32 next = activeChildContext + 1; next < 4; ++next)
    {
        u8 *child = TH08_ECL_AT(unusedContext, u8 *, 0x3384 + next * 4);
        if (!child)
            continue;

        TH08_ECL_AT(unusedContext, u8 *, 0x2CA4) = child + 0x230;
        TH08_ECL_AT(unusedContext, u8 *, 0x2CA0) = child + 8;
        instruction = reinterpret_cast<EclRawInstruction *>(*(RawInstruction **)(child + 8));
        *(i32 *)(child + 0x228) = next + 1;
        TH08_ECL_AT(unusedContext, i16, 0x2CEA) = *(i16 *)(child + 6);
        activeChildContext = next;
        goto restart_context;
    }

    TH08_ECL_AT(unusedContext, u8 *, 0x2CA4) =
        reinterpret_cast<u8 *>(enemy) + 0xA20;
    TH08_ECL_AT(unusedContext, u8 *, 0x2CA0) =
        reinterpret_cast<u8 *>(enemy) + 0x7F8;
    TH08_ECL_CONTEXT_API(unusedContext)->ResetEnemyAfterRun(
        reinterpret_cast<u8 *>(enemy));
    TH08_ECL_CONTEXT_API(unusedContext)->FinalizeEnemyAfterRun(
        reinterpret_cast<u8 *>(enemy));

    return ZUN_SUCCESS;
}

#undef TH08_ECL_CONTEXT_ENEMY
#undef TH08_ECL_CONTEXT_INSTRUCTION
#undef TH08_ECL_CONTEXT_API
#undef TH08_ECL_CONTEXT_CHILD

} // namespace th08
