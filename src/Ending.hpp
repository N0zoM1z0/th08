#pragma once

#include "AnmManager.hpp"
#include "Global.hpp"
#include "ScreenEffect.hpp"
#include "Supervisor.hpp"
#include "ZunResult.hpp"
#include "diffbuild.hpp"
#include "inttypes.hpp"
#include "utils.hpp"
#include <d3dx8.h>

namespace th08
{

enum EndingFadeType
{
    ENDING_FADE_TYPE_NO_FADE,
    ENDING_FADE_TYPE_FADE_IN_BLACK,
    ENDING_FADE_TYPE_FADE_OUT_BLACK,
    ENDING_FADE_TYPE_FADE_IN_WHITE,
    ENDING_FADE_TYPE_FADE_OUT_WHITE,
};

#define END_READ_OPCODE '@'
enum EndOpcode
{
    END_OPCODE_FADE_IN_BLACK = '0',
    END_OPCODE_FADE_OUT_BLACK = '1',
    END_OPCODE_FADE_IN = '2',
    END_OPCODE_FADE_OUT = '3',
    END_OPCODE_EXECUTE_ANM = 'a',
    END_OPCODE_BACKGROUND = 'b',
    END_OPCODE_COLOR = 'c',
    END_OPCODE_PLAY_MUSIC = 'm',
    END_OPCODE_WAIT_RESET = 'r',
    END_OPCODE_SET_DELAY = 's',
    END_OPCODE_SET_VERTICAL_SCROLL_POS = 'v',
    END_OPCODE_WAIT = 'w',
    END_OPCODE_END = 'z',
    END_OPCODE_EXEC_END_FILE = 'F',
    END_OPCODE_FADE_MUSIC = 'M',
    END_OPCODE_ROLL_STAFF = 'R',
    END_OPCODE_SCROLL_BACKGROUND = 'V',
};

struct Ending
{
    Ending();

    ZunResult ReadScriptParameter();
    void UpdateAndDrawFade();
    ZunResult RunEndingScript();
    ZunResult LoadEndingScript(const char *path);

    static ZunResult RegisterChain();
    static ChainCallbackResult OnUpdate(Ending *ending);
    static ChainCallbackResult OnDraw(Ending *ending);
    static ZunResult AddedCallback(Ending *ending);
    static ZunResult DeletedCallback(Ending *ending);

    ChainElem *calcChain;      // 0x0
    ChainElem *drawChain;      // 0x4
    Float2 backgroundPos;      // 0x08 + //0x0c
    f32 backgroundScrollSpeed; // 0x10
    AnmVm endingVms[16];                 // 0x14
    char *scriptData;                    // 0x2a54
    ZunBool hasSeenEnding;               // 0x2a58
    i32 canSkipChainedEnding;            // 0x2a5c
    AnmLoaded *endingAnm;                // 0x2a60
    ZunTimer elapsedTimer;               // 0x2a64
    ZunTimer lineWaitTimer;              // 0x2a70
    ZunTimer pageWaitTimer;              // 0x2a7c
    i32 pageSkipLockFrames;              // 0x2a88
    i32 lineSkipLockFrames;              // 0x2a8c
    i32 defaultLineWaitFrames;           // 0x2a90
    i32 minimumLineWaitFrames;           // 0x2a94
    u32 unconsumedDword2A98;             // 0x2a98
    i32 nextTextVmIndex;      // 0x2a9c
    i32 textColor;            // 0x2aa0
    D3DCOLOR fadeColor;       // 0x2aa4
    i32 fadeTimer;            // 0x2aa8
    i32 fadeDuration;         // 0x2aac
    EndingFadeType fadeMode;  // 0x2ab0
    char *scriptCursor;       // 0x2ab4
};

C_ASSERT(sizeof(Ending) == 0x2ab8);
C_ASSERT(offsetof(Ending, endingVms) == 0x14);
C_ASSERT(offsetof(Ending, scriptData) == 0x2a54);
C_ASSERT(offsetof(Ending, canSkipChainedEnding) == 0x2a5c);
C_ASSERT(offsetof(Ending, endingAnm) == 0x2a60);
C_ASSERT(offsetof(Ending, pageWaitTimer) == 0x2a7c);
C_ASSERT(offsetof(Ending, pageSkipLockFrames) == 0x2a88);
C_ASSERT(offsetof(Ending, defaultLineWaitFrames) == 0x2a90);
C_ASSERT(offsetof(Ending, minimumLineWaitFrames) == 0x2a94);
C_ASSERT(offsetof(Ending, unconsumedDword2A98) == 0x2a98);
C_ASSERT(offsetof(Ending, nextTextVmIndex) == 0x2a9c);
C_ASSERT(offsetof(Ending, scriptCursor) == 0x2ab4);

} // namespace th08
