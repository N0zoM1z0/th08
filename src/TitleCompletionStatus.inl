// Shared by the production TitleScreen owner and its strict VC7 probe.
// The probe remains the canonical byte oracle for this source-shaped body.
static inline void InitializeTitleCompletionVmAndSetSprite(
    AnmLoaded *anm, AnmVm *vm, i32 sprite)
{
    i32 inlineSlot;
    vm->Initialize();
    vm->anmFile = anm;
    anm->SetSprite(vm, sprite);
}

ChainCallbackResult TitleScreen::DrawCompletionStatusText()
{
    ZunBool showVm = FALSE;

    if (this->stateTimer2 > 8)
    {
        if (g_GameManager.StageClearedWithoutRetries(
                STAGE6B, this->cursor, g_Supervisor.cfg.defaultDifficulty) &&
            g_GameManager.StageClearedWithRetries(
                STAGE6A, this->cursor, g_Supervisor.cfg.defaultDifficulty))
        {
            showVm = TRUE;
            InitializeTitleCompletionVmAndSetSprite(
                this->titleAnm, &this->spellCardNameVms[0], 146);
        }
        else if (g_GameManager.StageClearedWithoutRetries(
                     STAGE6B, this->cursor,
                     g_Supervisor.cfg.defaultDifficulty))
        {
            showVm = TRUE;
            InitializeTitleCompletionVmAndSetSprite(
                this->titleAnm, &this->spellCardNameVms[0], 148);
        }
        else if (g_GameManager.StageClearedWithoutRetries(STAGE6B, this->cursor, EASY) ||
                 g_GameManager.StageClearedWithoutRetries(STAGE6B, this->cursor, NORMAL) ||
                 g_GameManager.StageClearedWithoutRetries(STAGE6B, this->cursor, HARD) ||
                 g_GameManager.StageClearedWithoutRetries(STAGE6B, this->cursor, LUNATIC) ||
                 this->cursor > 3)
        {
            showVm = TRUE;
            InitializeTitleCompletionVmAndSetSprite(
                this->titleAnm, &this->spellCardNameVms[0], 147);
        }
        else if (g_GameManager.StageClearedWithRetries(STAGE6A, this->cursor, EASY) ||
                 g_GameManager.StageClearedWithRetries(STAGE6A, this->cursor, NORMAL) ||
                 g_GameManager.StageClearedWithRetries(STAGE6A, this->cursor, HARD) ||
                 g_GameManager.StageClearedWithRetries(STAGE6A, this->cursor, LUNATIC))
        {
            showVm = TRUE;
            InitializeTitleCompletionVmAndSetSprite(
                this->titleAnm, &this->spellCardNameVms[0], 145);
        }
    }

    if (showVm)
    {
        this->spellCardNameVms[0].anchor = 3;
        this->spellCardNameVms[0].color1.a = 255;
        this->spellCardNameVms[0].color1.r = 255;
        this->spellCardNameVms[0].color1.g = 255;
        this->spellCardNameVms[0].color1.b = 255;
        this->spellCardNameVms[0].pos.x = 400.0f;
        this->spellCardNameVms[0].pos.y = 170.0f;
        this->spellCardNameVms[0].pos.z = 0.0f;
        g_AnmManager->DrawNoRotation(&this->spellCardNameVms[0]);
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
}
