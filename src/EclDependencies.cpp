#include "th_pch.h"

#include "Gui.hpp"
#include "Spellcard.hpp"

namespace th08
{

struct EclSpellCardInstructionArgs
{
    u8 header[0xC];
    i16 enemyFace;
    u16 spellCardNumber;
    i32 bonus;
    u8 encodedName[0x30];
    u8 encodedOwner[0x30];
    char commentLine1[0x40];
    char commentLine2[0x40];
};

C_ASSERT(offsetof(EclSpellCardInstructionArgs, enemyFace) == 0x0C);
C_ASSERT(offsetof(EclSpellCardInstructionArgs, spellCardNumber) == 0x0E);
C_ASSERT(offsetof(EclSpellCardInstructionArgs, bonus) == 0x10);
C_ASSERT(offsetof(EclSpellCardInstructionArgs, encodedName) == 0x14);
C_ASSERT(offsetof(EclSpellCardInstructionArgs, encodedOwner) == 0x44);
C_ASSERT(offsetof(EclSpellCardInstructionArgs, commentLine1) == 0x74);
C_ASSERT(offsetof(EclSpellCardInstructionArgs, commentLine2) == 0xB4);

// FUNCTION: th08 0x421280
void __fastcall StartEnemySpell(u8 *enemy, void *instruction)
{
    g_Spellcard.StartSpell(static_cast<EclSpellCardInstructionArgs *>(instruction)->spellCardNumber,
                           static_cast<EclSpellCardInstructionArgs *>(instruction)->encodedName,
                           static_cast<EclSpellCardInstructionArgs *>(instruction)->enemyFace,
                           static_cast<EclSpellCardInstructionArgs *>(instruction)->bonus, enemy,
                           static_cast<EclSpellCardInstructionArgs *>(instruction)->encodedOwner,
                           static_cast<EclSpellCardInstructionArgs *>(instruction)->commentLine1,
                           static_cast<EclSpellCardInstructionArgs *>(instruction)->commentLine2);
}

// FUNCTION: th08 0x4212e0
void __fastcall EndEnemySpell(u8 *enemy, void *instruction)
{
    g_Spellcard.EndSpell();
}

// FUNCTION: th08 0x4230c0
void Gui::FUN_004230c0(f32 value)
{
    this->bossLifeBarSize = value;
}

// FUNCTION: th08 0x423130
void Gui::FUN_00423130(i32 value)
{
    this->eclSetLives = value;
}

} // namespace th08
