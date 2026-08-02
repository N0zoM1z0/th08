#include "th_pch.h"

#include "AsciiManager.hpp"
#include "GameManager.hpp"
#include "MusicRoom.hpp"
#include "SoundPlayer.hpp"
#include "i18n.hpp"

namespace th08
{

const char *g_BgmNotUnlockedWarning[] = {
    TH_WARN_BGM_NOT_UNLOCKED0, TH_WARN_BGM_NOT_UNLOCKED1, TH_WARN_BGM_NOT_UNLOCKED2, TH_WARN_BGM_NOT_UNLOCKED3,
    TH_WARN_BGM_NOT_UNLOCKED4, TH_WARN_BGM_NOT_UNLOCKED5, TH_WARN_BGM_NOT_UNLOCKED6, TH_WARN_BGM_NOT_UNLOCKED4};

ZunResult MusicRoom::CheckInputEnable()
{
    i32 i;

    if (this->frameCount == 0)
    {
        for (i = 0; i < ARRAY_SIZE_SIGNED(this->songNameVms); i++)
        {
            if (this->cursor == i)
            {
                this->songNameVms[i].SetInterrupt(1);
            }
            else
            {
                this->songNameVms[i].SetInterrupt(2);
            }
        }

        for (i = 0; i < ARRAY_SIZE_SIGNED(this->descriptionVms) - 1; i++)
        {
            this->descriptionVms[i].SetInterrupt(1);
        }
    }

    if (this->frameCount >= 8)
    {
        this->inputState = 1;
    }

    return ZUN_SUCCESS;
}

#pragma var_order(oldListingOffset, i, buf1, buf2, pos)
i32 MusicRoom::ProcessInput()
{
    i32 oldListingOffset = this->listingOffset;
    i32 i;

    if (WAS_PRESSED_SCROLLING(TH_BUTTON_UP))
    {
        this->cursor--;
        if (this->cursor < 0)
        {
            this->cursor = this->numDescriptors - 1;
            this->listingOffset = this->numDescriptors - 10;
            if (this->listingOffset < 0)
            {
                this->listingOffset = 0;
            }
        }
        else if (this->listingOffset > this->cursor)
        {
            this->listingOffset = this->cursor;
        }

        for (i = 0; i < ARRAY_SIZE_SIGNED(this->songNameVms); i++)
        {
            if (this->cursor == i)
            {
                this->songNameVms[i].SetInterrupt(1);
            }
            else
            {
                this->songNameVms[i].SetInterrupt(2);
            }
        }

        for (i = 0; i < ARRAY_SIZE_SIGNED(this->descriptionVms) - 1; i++)
        {
            this->descriptionVms[i].SetInterrupt(2);
        }

        this->frameCount = 0;
    }

    if (WAS_PRESSED_SCROLLING(TH_BUTTON_DOWN))
    {
        this->cursor++;

        if (this->cursor >= this->numDescriptors)
        {
            this->cursor = 0;
            this->listingOffset = 0;
        }
        else
        {
            if (this->listingOffset <= this->cursor - 10)
            {
                this->listingOffset = this->cursor - 9;
            }
        }

        for (i = 0; i < ARRAY_SIZE_SIGNED(this->songNameVms); i++)
        {
            if (this->cursor == i)
            {
                this->songNameVms[i].SetInterrupt(1);
            }
            else
            {
                this->songNameVms[i].SetInterrupt(2);
            }
        }

        for (i = 0; i < ARRAY_SIZE_SIGNED(this->descriptionVms) - 1; i++)
        {
            this->descriptionVms[i].SetInterrupt(2);
        }

        this->frameCount = 0;
    }

    char buf1[66];
    char buf2[66];

    if (this->frameCount < 30)
    {
        i = -1;

        if (this->frameCount == 10)
        {
            i = 0;
        }
        else if (this->frameCount == 12)
        {
            i = 1;
        }
        else if (this->frameCount == 14)
        {
            i = 2;
        }
        else if (this->frameCount == 16)
        {
            i = 3;
        }
        else if (this->frameCount == 18)
        {
            i = 4;
        }
        else if (this->frameCount == 20)
        {
            i = 5;
        }
        else if (this->frameCount == 22)
        {
            i = 6;
        }

        if (i >= 0)
        {
            g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->descriptionVms[i], 10 + i);
            this->descriptionVms[i].SetInterrupt(1);

            memset(buf1, 0, sizeof(buf1));

            if (this->selectedSongIndex == this->cursor || this->bgmUnlocked[this->cursor])
            {
                memcpy(buf1, this->trackDescriptors[this->cursor].descriptors[i], 64);
            }
            else
            {
                memcpy(buf1, g_BgmNotUnlockedWarning[i], 64);
            }

            if (buf1[0] != '\0')
            {
                this->descriptionVms[i].prefix.flag1 = true;
                g_AnmManager->DrawTextLeft(&this->descriptionVms[i], 0xffe0c0, 0x300000, buf1);
            }
            else
            {
                this->descriptionVms[i].prefix.flag1 = false;
            }
        }
    }

    if (WAS_PRESSED(TH_BUTTON_SHOOT | TH_BUTTON_ENTER))
    {
        this->selectedSongIndex = this->cursor;

        if (g_Supervisor.IsMusicPreloadEnabled())
        {
            g_SoundPlayer.StartBGM("thbgm.dat");
        }

        g_Supervisor.PlayAudio(this->trackDescriptors[this->selectedSongIndex].path, 0);

        this->frameCount = 0;

        g_Supervisor.textAnm->SetAndExecuteScriptIdx(&this->descriptionVms[7], 17);
        this->descriptionVms[7].SetInterrupt(1);

        memset(buf2, 0, sizeof(buf2));

        memcpy(buf2, this->trackDescriptors[this->cursor].descriptors[0], 64);

        g_AnmManager->DrawTextLeft(&this->descriptionVms[7], 0xffe0c0, 0x300000, buf2);
    }

    if (WAS_PRESSED(TH_BUTTON_BOMB | TH_BUTTON_MENU))
    {
        g_Supervisor.curState = SupervisorState_TitleScreen;

        g_Supervisor.SetupLoadingVmsAndInitCapture(&Float3(500.0, 440.0f, 0.0f));

        return 1;
    }

    if (WAS_PRESSED(TH_BUTTON_SKIP))
    {
        g_Supervisor.FadeOutMusic(8.0f);
    }
    if (WAS_PRESSED(TH_BUTTON_RESET))
    {
        if (g_Supervisor.IsMusicPreloadEnabled())
        {
            g_SoundPlayer.StartBGM("thbgm.dat");
        }

        g_Supervisor.PlayAudio(this->trackDescriptors[this->selectedSongIndex].path, 0);
    }

    this->frameCount++;

    return ZUN_SUCCESS;
}

ZunResult MusicRoom::RegisterChain()
{
    static MusicRoom g_MusicRoom;

    MusicRoom *musicRoom = &g_MusicRoom;

    memset(musicRoom, 0, sizeof(MusicRoom));

    musicRoom->calcChain = g_Chain.CreateElem((ChainCallback)MusicRoom::OnUpdate);
    musicRoom->calcChain->arg = musicRoom;
    musicRoom->calcChain->addedCallback = (ChainLifetimeCallback)MusicRoom::AddedCallback;
    musicRoom->calcChain->deletedCallback = (ChainLifetimeCallback)MusicRoom::DeletedCallback;

    if (g_Chain.AddToCalcChain(musicRoom->calcChain, 4) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    musicRoom->drawChain = g_Chain.CreateElem((ChainCallback)MusicRoom::OnDraw);
    musicRoom->drawChain->arg = musicRoom;
    g_Chain.AddToDrawChain(musicRoom->drawChain, 3);

    return ZUN_SUCCESS;
}

ChainCallbackResult MusicRoom::OnUpdate(MusicRoom *musicRoom)
{
    i32 i;

    i32 oldInputSetting = musicRoom->inputState;

start:
    switch (musicRoom->inputState)
    {
    case 0:
        if (musicRoom->CheckInputEnable() == ZUN_SUCCESS)
        {
            break;
        }

        goto start;
    case 1:
        if (musicRoom->ProcessInput())
        {
            return CHAIN_CALLBACK_RESULT_CONTINUE_AND_REMOVE_JOB;
        }
    }

    if (oldInputSetting != musicRoom->inputState)
    {
        musicRoom->frameCount = 0;
    }
    else
    {
        musicRoom->frameCount++;
    }

    g_AnmManager->ExecuteScript(&musicRoom->mainVms[0]);

    for (i = 0; i < ARRAY_SIZE_SIGNED(musicRoom->songNameVms); i++)
    {
        g_AnmManager->ExecuteScript(&musicRoom->songNameVms[i]);
    }

    for (i = 0; i < ARRAY_SIZE_SIGNED(musicRoom->descriptionVms); i++)
    {
        g_AnmManager->ExecuteScript(&musicRoom->descriptionVms[i]);
    }

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(color, i, arrowText, position)
ChainCallbackResult MusicRoom::OnDraw(MusicRoom *musicRoom)
{
    int i;
    Float3 position;
    D3DCOLOR color;
    char arrowText[4];

    arrowText[0] = 0x7f;
    arrowText[1] = 0x0;

    g_AnmManager->ClearTexture();
    g_AnmManager->CopySurfaceToBackbuffer(0, 0, 0, 0, 0);
    g_AnmManager->DrawNoRotation(&musicRoom->mainVms[0]);

    for (i = musicRoom->listingOffset; i < musicRoom->listingOffset + 10; i++)
    {
        if (i >= musicRoom->numDescriptors)
        {
            break;
        }

        g_AsciiManager.SetColor(musicRoom->songNameVms[i].prefix.color1.d3dColor);

        musicRoom->songNameVms[i].pos.x = 93.0f;
        musicRoom->songNameVms[i].pos.y = (((i + 1 - musicRoom->listingOffset) * 18) + 104.0f) - 20.0f;
        musicRoom->songNameVms[i].pos.z = 0.0f;

        g_AnmManager->DrawNoRotation(&musicRoom->songNameVms[i]);

        position = musicRoom->songNameVms[i].pos;
        position.x -= 60.0f;

        if (musicRoom->cursor == i)
        {
            g_AsciiManager.AddString(&position, arrowText);
        }

        position.x += 15.0f;
        g_AsciiManager.AddFormatText(&position, "%2d.", i + 1);
    }

    /* again, ???? */
    i++;

    for (i = 0; i < ARRAY_SIZE_SIGNED(musicRoom->descriptionVms) - 1; i++)
    {
        g_AnmManager->DrawNoRotation(&musicRoom->descriptionVms[i]);
    }

    g_AsciiManager.SetColor(0xffffffff);

    position.x = 320.0f;
    position.y = 32.0f;
    position.z = 0.0f;

    g_AsciiManager.AddFormatText(&position, "Now Playing");

    position = musicRoom->descriptionVms[7].pos;
    color = musicRoom->descriptionVms[7].prefix.color1.d3dColor;

    musicRoom->descriptionVms[7].pos.x = 320.0f;
    musicRoom->descriptionVms[7].pos.y = 52.0f;
    musicRoom->descriptionVms[7].pos.z = 0.0f;
    musicRoom->descriptionVms[7].prefix.color1.d3dColor = 0xffffffff;

    g_AnmManager->DrawNoRotation(&musicRoom->descriptionVms[7]);

    musicRoom->descriptionVms[7].pos = position;
    musicRoom->descriptionVms[7].prefix.color1.d3dColor = color;

    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

#pragma var_order(i, currentLine, fileSize, ptr, charIdx, musicCmtFile)
ZunResult MusicRoom::AddedCallback(MusicRoom *musicRoom)
{
    if (g_AnmManager->LoadSurface(0, "result/music.jpg") != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    musicRoom->musicAnm = g_AnmManager->LoadAnm(23, "music00.anm");
    if (musicRoom->musicAnm == NULL)
    {
        return ZUN_ERROR;
    }

    musicRoom->selectedSongIndex = 0;
    musicRoom->musicAnm->SetAndExecuteScriptIdx(&musicRoom->mainVms[0], 0);
    musicRoom->frameCount = 0;

    u32 fileSize;
    char *musicCmtFile;
    char *ptr;

    musicCmtFile = ptr = (char *)FileSystem::OpenFile("sprt/musiccmt.txt", (i32 *)&fileSize, FALSE);
    if (ptr == NULL)
    {
        return ZUN_ERROR;
    }

    musicRoom->trackDescriptors = ZUN_NEW_ARRAY(TrackDescriptor, 32, "MusicCmtInf");

    int i = -1;
    u32 charIdx;
    int currentLine;

    while ((ptr - musicCmtFile) < fileSize)
    {
        if (*ptr == '@')
        {
            ptr++;
            i++;
            charIdx = 0;

            while (*ptr != '\n' && *ptr != '\r')
            {
                musicRoom->trackDescriptors[i].path[charIdx] = *ptr;

                ptr++;
                charIdx++;

                if ((ptr - musicCmtFile) >= fileSize)
                {
                    goto out;
                }
            }

            while (*ptr == '\n' || *ptr == '\r')
            {
                ptr++;

                if ((ptr - musicCmtFile) >= fileSize)
                {
                    goto out;
                }
            }

            charIdx = 0;

            while (*ptr != '\n' && *ptr != '\r')
            {
                musicRoom->trackDescriptors[i].title[charIdx] = *ptr;

                ptr++;
                charIdx++;

                if ((ptr - musicCmtFile) >= fileSize)
                {
                    goto out;
                }
            }

            while (*ptr == '\n' || *ptr == '\r')
            {
                ptr++;

                if ((ptr - musicCmtFile) >= fileSize)
                {
                    goto out;
                }
            }

            for (currentLine = 0; currentLine < 7; currentLine++)
            {
                if (*ptr == '@')
                {
                    break;
                }

                memset(&musicRoom->trackDescriptors[i].descriptors[currentLine], 0,
                       sizeof(musicRoom->trackDescriptors[i].descriptors[0]));

                charIdx = 0;

                while (*ptr != '\n' && *ptr != '\r')
                {
                    musicRoom->trackDescriptors[i].descriptors[currentLine][charIdx] = *ptr;

                    ptr++;
                    charIdx++;

                    if ((ptr - musicCmtFile) >= fileSize)
                    {
                        goto out;
                    }
                }

                while (*ptr == '\n' || *ptr == '\r')
                {
                    ptr++;

                    if ((ptr - musicCmtFile) >= fileSize)
                    {
                        goto out;
                    }
                }
            }
        }
        else
        {
            *ptr++;
        }
    }

out:
    musicRoom->numDescriptors = i + 1;

    for (i = 0; i < musicRoom->numDescriptors; i++)
    {
        musicRoom->bgmUnlocked[i] = g_GameManager.plst.bgmUnlocked[i];
    }

    for (i = 0; i < musicRoom->numDescriptors; i++)
    {
        musicRoom->musicAnm->SetAndExecuteScriptIdx(&musicRoom->songNameVms[i], 1 + i);
        if (musicRoom->bgmUnlocked[i])
        {
            g_AnmManager->DrawTextLeft(&musicRoom->songNameVms[i], 0xc0e0ff, 0x302080,
                                       musicRoom->trackDescriptors[i].title);
        }
        else
        {
            g_AnmManager->DrawTextLeft(&musicRoom->songNameVms[i], 0x80a0c0, 0x100040, TH_SONG_NAME_NOT_UNLOCKED);
        }

        musicRoom->songNameVms[i].pos.x = 93.0f;
        musicRoom->songNameVms[i].pos.y = (((i + 1) * 18) + 104.0f) - 20.0f;
        musicRoom->songNameVms[i].pos.z = 0.0f;
        musicRoom->songNameVms[i].prefix.anchor = 3;
    }

    g_ZunMemory.Free(musicCmtFile);

    return ZUN_SUCCESS;
}

ZunResult MusicRoom::DeletedCallback(MusicRoom *musicRoom)
{
    ZUN_DELETE(musicRoom->trackDescriptors);

    g_AnmManager->ReleaseSurface(0);
    g_AnmManager->ReleaseAnm(23);

    g_Chain.Cut(musicRoom->drawChain);
    musicRoom->drawChain = NULL;

    return ZUN_SUCCESS;
}

} /* namespace th08 */
