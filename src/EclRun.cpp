#include "EclManager.hpp"
#include "EclOperands.hpp"
#include "AsciiManager.hpp"
#include "Gui.hpp"
#include "BulletManager.hpp"
#include "GameManager.hpp"
#include "Spellcard.hpp"

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
        i32 restorePosition = 0;
        Interpolator *entry = reinterpret_cast<Interpolator *>(
            TH08_ECL_CURRENT_CONTEXT(unusedContext) + 0x9C);
        Vec3 savedPosition = TH08_ECL_AT(unusedContext, Vec3, 0x2D34);

        if (*reinterpret_cast<EclContextCallback *>(
                TH08_ECL_CURRENT_CONTEXT(unusedContext) + 0x10))
            (*reinterpret_cast<EclContextCallback *>(
                TH08_ECL_CURRENT_CONTEXT(unusedContext) + 0x10))(
                enemy, *reinterpret_cast<void **>(
                    TH08_ECL_CURRENT_CONTEXT(unusedContext) + 0x14));

        for (i32 i = 0; i < 8; ++i, ++entry)
        {
            if (!entry->callback)
                continue;

            entry->timer++;
            if (entry->timer >= entry->duration)
                entry->timer = entry->duration;

            f32 progress = static_cast<f32>(entry->timer) / entry->duration;
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

            (enemy->*entry->callback)(progress);
            if (entry->timer >= entry->duration)
                entry->callback = 0;

            if (entry->affectedVariable == 10042.0f ||
                entry->affectedVariable == 10043.0f ||
                entry->affectedVariable == 10044.0f)
                restorePosition = 1;
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
    reinterpret_cast<ZunTimer *>(TH08_ECL_CURRENT_CONTEXT(unusedContext) + 4)->operator++(0);

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
    enemy->FUN_00422c40();
    enemy->FUN_00423150();

    return ZUN_SUCCESS;
}

#undef TH08_ECL_CONTEXT_ENEMY
#undef TH08_ECL_CONTEXT_INSTRUCTION
#undef TH08_ECL_CONTEXT_API
#undef TH08_ECL_CONTEXT_CHILD

} // namespace th08
