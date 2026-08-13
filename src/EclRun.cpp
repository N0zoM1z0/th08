#include "EclManager.hpp"
#include "EclOperands.hpp"

#include "EclRunLow.inl"
#include "EclRunHigh.inl"

namespace th08
{

// The dispatcher is now source-complete, while target-address service binding
// remains intentionally separate: subsystem owners can replace provisional
// adapters without changing the recovered opcode/control-flow core.
extern EclRunLowProposal::Services &GetEclRunLowServices(EclManager *manager);
extern EclRunHighProposal::TargetApi &GetEclRunHighApi(EclManager *manager);
extern u8 g_TargetDifficulty0160F53C;

static __forceinline u8 *EnemyBytes(Enemy *enemy)
{
    return reinterpret_cast<u8 *>(enemy);
}

static __forceinline EnemyEclContext *CurrentContext(Enemy *enemy)
{
    return *reinterpret_cast<EnemyEclContext **>(EnemyBytes(enemy) + 0x2CA0);
}

static __forceinline void RefreshResolvedPosition(Enemy *enemy)
{
    u8 *bytes = EnemyBytes(enemy);
    D3DXVECTOR3 *resolved = reinterpret_cast<D3DXVECTOR3 *>(bytes + 0x2D88);
    D3DXVECTOR3 *base = reinterpret_cast<D3DXVECTOR3 *>(bytes + 0x2D34);
    D3DXVECTOR3 *offset = reinterpret_cast<D3DXVECTOR3 *>(bytes + 0x2D40);
    resolved->x = base->x + offset->x;
    resolved->y = base->y + offset->y;
    resolved->z = base->z + offset->z;
}

// FUNCTION: th08 0x004184B0
ZunResult EclManager::RunEcl(Enemy *enemy)
{
    using namespace EclRunLowProposal;
    using namespace EclRunHighProposal;

    u8 *bytes = EnemyBytes(enemy);
    EclOperands::EnemyOverlay *operandEnemy = reinterpret_cast<EclOperands::EnemyOverlay *>(enemy);
    Services &lowServices = GetEclRunLowServices(this);
    TargetApi &highApi = GetEclRunHighApi(this);
    i32 activeChildContext = -1;

    *reinterpret_cast<u8 **>(bytes + 0x2CA4) = bytes + 0xA20;
    *reinterpret_cast<u8 **>(bytes + 0x2CA0) = bytes + 0x7F8;
    *reinterpret_cast<i16 *>(bytes + 0x2CEA) = *reinterpret_cast<i16 *>(bytes + 0x2CE8);

restart_context:
    EnemyEclContext *context = CurrentContext(enemy);
    EclRawInstruction *instruction = context->currentInstr;

    for (;;)
    {
        RefreshResolvedPosition(enemy);

        Context highContext;
        highContext.enemy = bytes;
        highContext.instruction = reinterpret_cast<RawInstruction *>(instruction);
        highContext.api = &highApi;
        highContext.activeChildContext = activeChildContext;

        if (*reinterpret_cast<i16 *>(bytes + 0x2D30) >= 0)
        {
            EnterSubroutine(highContext);
            goto restart_context;
        }

        if (context->secondaryTime.current > 0)
        {
            context->secondaryTime--;
            context->time--;
            break;
        }

        if (instruction == NULL || context->time.current != instruction->time)
            break;

        const u8 activeDifficulty = g_TargetDifficulty0160F53C | *(u8 *)(bytes + 0x3330);
        if ((instruction->difficultyMask & activeDifficulty) != activeDifficulty)
        {
            instruction = reinterpret_cast<EclRawInstruction *>(
                reinterpret_cast<u8 *>(instruction) + instruction->nextOffset);
            continue;
        }

        if (instruction->opcode <= 92)
        {
            LowResult result = Dispatch(operandEnemy, context, instruction, lowServices);
            switch (result.control)
            {
            case LOW_RETURN_MINUS_ONE:
                return ZUN_ERROR;
            case LOW_REDISPATCH:
                instruction = result.nextInstruction;
                continue;
            case LOW_RESTART_RUN_LOOP:
                goto restart_context;
            case LOW_SELECT_NEXT_CONTEXT:
                break;
            case LOW_ADVANCE:
            case LOW_NOT_HANDLED:
                instruction = reinterpret_cast<EclRawInstruction *>(
                    reinterpret_cast<u8 *>(instruction) + instruction->nextOffset);
                continue;
            }
            break;
        }

        highContext.instruction = reinterpret_cast<RawInstruction *>(instruction);
        if (DispatchOpcode93To184(highContext) == DISPATCH_ENTER_SUBROUTINE)
        {
            EnterSubroutine(highContext);
            goto restart_context;
        }

        instruction = reinterpret_cast<EclRawInstruction *>(
            reinterpret_cast<u8 *>(instruction) + instruction->nextOffset);
    }

    Context tailContext;
    tailContext.enemy = bytes;
    tailContext.instruction = reinterpret_cast<RawInstruction *>(instruction);
    tailContext.api = &highApi;
    tailContext.activeChildContext = activeChildContext;
    if (!RunCommonTail(tailContext))
    {
        activeChildContext = tailContext.activeChildContext;
        goto restart_context;
    }

    return ZUN_SUCCESS;
}

} // namespace th08
