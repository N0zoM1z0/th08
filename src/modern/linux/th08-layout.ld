/*
 * Several exact reconstruction units still materialize original 1.00d data
 * addresses.  Keep the target data arena present and bind the corresponding
 * reconstructed owners to their original virtual addresses.  The regular ELF
 * image remains at the toolchain's native base; only target-owned data lives
 * here.  NOLOAD prevents this zero-initialized arena from inflating the file.
 */
SECTIONS
{
    .th08_target_data 0x004b4000 (NOLOAD) :
    {
        . = 0x018da000 - 0x004b4000;
    }

    _ZN4th0816g_LastSpellCountE = 0x004c6c3c;
    _ZN4th0817g_EffectTemplatesE = 0x004c6d30;
    _ZN4th0822g_GuiStageClearBonusesE = 0x004c7158;
    _ZN4th0822g_GuiMessageTextColorsE = 0x004c7180;
    _ZN4th0823g_GuiStageMusicContextsE = 0x004c7240;
    _ZN4th0830g_AsciiManagerDrawChainLowPrioE = 0x004cce00;
    _ZN4th0814g_AsciiManagerE = 0x004cce20;
    _ZN4th0823g_AsciiManagerCalcChainE = 0x004e3fd0;
    _ZN4th0831g_AsciiManagerDrawChainHighPrioE = 0x004e3ff0;
    _ZN4th0829g_BackgroundDrawChainHighPrioE = 0x004e4010;
    _ZN4th0812g_BackgroundE = 0x004e4030;
    _ZN4th0821g_GuiMessageStageModeE = 0x004e4b64;
    _ZN4th0828g_BackgroundDrawChainLowPrioE = 0x004ea630;
    _ZN4th0821g_BackgroundCalcChainE = 0x004ea650;

    /* ECL opcode 147 names Background::unk6260 as a standalone global. */

    _ZN4th0811g_SpellcardE = 0x004ea670;
    _ZN4th0820g_SpellcardCalcChainE = 0x004eccac;

    /* ECL opcode 155 names Spellcard::scoreLimit as a standalone global. */
    _ZN4th0819g_EclGlobal004ECCA8E = 0x004ecca8;

    _ZN4th0812g_EclManagerE = 0x004eccb8;
    _ZN4th0817EclRunLowProposal19g_EclCallParametersE = 0x004ece20;
    _ZN4th0815g_EffectManagerE = 0x004ece60;

    /*
     * The original linker placed EffectManager::stageEffectAnm at the
     * aggregate tail directly before its chain elements.  Spellcard.cpp
     * names that pointer as a separate global, so keep both views aliased.
     */
    _ZN4th0824g_SpellcardBackgroundAnmE = 0x00577eb8;
    _ZN4th0824g_EffectManagerCalcChainE = 0x00577ebc;
    _ZN4th0824g_EffectManagerDrawChainE = 0x00577edc;

    _ZN4th0831g_EnemyManagerDrawChainHighPrioE = 0x00577f00;
    _ZN4th0814g_EnemyManagerE = 0x00577f20;
    _ZN4th0823g_EnemyManagerCalcChainE = 0x00f54e30;
    _ZN4th0830g_EnemyManagerDrawChainLowPrioE = 0x00f54e50;
    _ZN4th0824g_BulletManagerDrawChainE = 0x00f54e70;
    _ZN4th0815g_BulletManagerE = 0x00f54e90;
    _ZN4th0824g_BulletManagerCalcChainE = 0x0160f408;
    _ZN4th085g_GuiE = 0x0160f428;
    _ZN4th0814g_GuiDrawChainE = 0x0160f4c4;
    _ZN4th0814g_GuiCalcChainE = 0x0160f4e4;
    _ZN4th0813g_GameManagerE = 0x0160f508;
    _ZN4th0823g_PlayerNormalBombCountE = 0x0164cfa8;
    _ZN4th0822g_PlayerDeathbombCountE = 0x0164cfac;
    _ZN4th0829g_MaxValuePointItemsCollectedE = 0x0164cf9c;
    _ZN4th0819g_PlayerGaugeBoundsE = 0x0164d300;
    _ZN4th0822g_GameManagerCalcChainE = 0x0164d344;
    _ZN4th0822g_GameManagerDrawChainE = 0x0164d364;
    _ZN4th0814g_JoystickCapsE = 0x0164d388;
    _ZN4th0826g_FocusButtonConflictStateE = 0x0164d51c;
    _ZN4th085g_RngE = 0x0164d520;
    _ZN4th0815g_CurFrameInputE = 0x0164d528;
    _ZN4th0824g_GuiMessageInputCurrentE = 0x0164d52c;
    _ZN4th0816g_LastFrameInputE = 0x0164d530;
    _ZN4th0825g_GuiMessageInputPreviousE = 0x0164d534;
    _ZN4th0826g_IsEighthFrameOfHeldInputE = 0x0164d538;
    _ZN4th0827g_NumOfFramesInputsWereHeldE = 0x0164d53c;
    _ZN4th0818g_GameErrorContextE = 0x0164d540;
    _ZN4th087g_ChainE = 0x0164f548;
    _ZN4th0812g_PbgArchiveE = 0x0164f588;
    _ZN4th0811g_ZunMemoryE = 0x0164f598;
    _ZN4th0816g_ControllerDataE = 0x016535a0;
    _ZN4th0819g_TextBufferSurfaceE = 0x01653620;
    _ZN4th0813g_ItemManagerE = 0x01653648;
    _ZN4th0816g_ExclusiveMutexE = 0x017ce6fc;
    _ZN4th0812g_GameWindowE = 0x017ce700;
    _ZN4th0834g_DummyMidiTimerPerformanceCounterE = 0x017ce744;
    _ZN4th0824g_SupervisorScreenEffectE = 0x017ce750;
    _ZN4th0812g_SupervisorE = 0x017ce758;
    _ZN4th0824g_GuiFullPowerModeFramesE = 0x017ce8cc;
    _ZN4th0832g_GuiMessageScreenEffectDurationE = 0x017ce8cc;

    /* These target globals are overlapping views into aggregate owners. */
    _ZN4th0823g_EclGameTimeScaleFlagsE = 0x017ce8fc;
    _ZN4th088g_PlayerE = 0x017d5ef8;
    _ZN4th0811EclOperands30g_TargetPlayerPosition017D61ACE = 0x017d61ac;

    _ZN4th0822g_SupervisorLoadingVmsE = 0x017ceac0;
    _ZN4th0829g_SupervisorFpsQpcSampleCountE = 0x017cf2ac;
    _ZN4th0822g_SupervisorFpsLastQpcE = 0x017cf2b0;
    _ZN4th0826g_SupervisorFpsDebugBufferE = 0x017cf2b8;
    _ZN4th0821g_SupervisorFpsBufferE = 0x017cf3b8;
    _ZN4th0823g_SupervisorFpsLastTimeE = 0x017cf4b8;
    _ZN4th0830g_SupervisorFpsTimeInitializedE = 0x017cf4bc;
    _ZN4th0825g_SupervisorFpsFrameCountE = 0x017cf4c0;
    _ZN4th0815g_ReplayManagerE = 0x018b8a28;
    _ZN4th0821g_ScreenEffectCounterE = 0x018b8a2c;
    _ZN4th0814g_ScreenEffectE = 0x018b8a30;
    _ZN4th0813g_SoundPlayerE = 0x018b8a68;
    _ZN4th0812g_AnmManagerE = 0x018bdc90;
    _ZN4th0814g_QuadVerticesE = 0x018bdc98;
    _ZN4th0834g_AnmManagerUntexturedQuadVerticesE = 0x018bdd08;
    _ZN4th0824g_BackgroundQuadVerticesE = 0x018bdd68;
    _ZN4th0813g_TitleScreenE = 0x018bde08;
    _ZN4th084Lzss6m_TreeE = 0x018bdf58;
    _ZN4th084Lzss6m_DictE = 0x018d5f68;
}
INSERT BEFORE .bss;
