#include "th_pch.h"

#include "inttypes.hpp"
#include <windows.h>
#include <mmreg.h>
#include <mmsystem.h>

#include "Global.hpp"
#include "Midi.hpp"
#include "Supervisor.hpp"
#include "i18n.hpp"
#include "utils.hpp"

namespace th08
{

DIFFABLE_STATIC(LARGE_INTEGER, g_DummyMidiTimerPerformanceCounter);

void MidiTimer::OnTimerElapsed()
{
}

MidiDevice::MidiDevice()
{
    handle = NULL;
    this->deviceId = 0;
}

MidiDevice::~MidiDevice()
{
    Close();
}

BOOL MidiDevice::OpenDevice(UINT uDeviceId)
{
    if (handle != NULL)
    {
        if (this->deviceId != uDeviceId)
        {
            Close();
        }
        else
        {
            return FALSE;
        }
    }

    this->deviceId = uDeviceId;

    return midiOutOpen(&handle, uDeviceId, (DWORD_PTR)g_Supervisor.hwndGameWindow, NULL, CALLBACK_WINDOW) !=
           MMSYSERR_NOERROR;
}

ZunResult MidiDevice::Close()
{
    if (handle == NULL)
    {
        return ZUN_ERROR;
    }

    midiOutReset(handle);
    midiOutClose(handle);
    handle = NULL;

    return ZUN_SUCCESS;
}

BOOL MidiDevice::SendLongMsg(LPMIDIHDR pmh)
{
    if (handle == NULL)
    {
        return FALSE;
    }
    else
    {
        if (midiOutPrepareHeader(handle, pmh, sizeof(*pmh)) != MMSYSERR_NOERROR)
        {
            return TRUE;
        }

        return midiOutLongMsg(handle, pmh, sizeof(*pmh)) != MMSYSERR_NOERROR;
    }
}

union MidiShortMsg {
    struct
    {
        u8 midiStatus;
        i8 firstByte;
        i8 secondByte;
        i8 unused;
    } msg;
    DWORD dwMsg;
};

BOOL MidiDevice::SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte)
{
    MidiShortMsg pkt;

    if (handle == NULL)
    {
        return FALSE;
    }
    else
    {
        pkt.msg.midiStatus = midiStatus;
        pkt.msg.firstByte = firstByte;
        pkt.msg.secondByte = secondByte;
        return midiOutShortMsg(handle, pkt.dwMsg) != MMSYSERR_NOERROR;
    }
}

MidiTimer::MidiTimer()
{
    timeGetDevCaps(&this->timeCaps, sizeof(TIMECAPS));
    this->timerId = 0;
}

MidiTimer::~MidiTimer()
{
    StopTimerImpl();
    timeEndPeriod(this->timeCaps.wPeriodMin);
}

UINT MidiTimer::StartTimerImpl(u32 delay, LPTIMECALLBACK cb, DWORD_PTR data)
{
    StopTimerImpl();
    timeBeginPeriod(this->timeCaps.wPeriodMin);

    if (cb != NULL)
    {
        this->timerId = timeSetEvent(delay, this->timeCaps.wPeriodMin, cb, data, TIME_PERIODIC);
    }
    else
    {
        this->timerId = timeSetEvent(delay, this->timeCaps.wPeriodMin, (LPTIMECALLBACK)MidiTimer::DefaultTimerCallback,
                                     (DWORD_PTR)this, TIME_PERIODIC);
    }

    return this->timerId;
}

BOOL MidiTimer::StopTimerImpl()
{
    if (this->timerId != 0)
    {
        timeKillEvent(this->timerId);
    }
    timeEndPeriod(this->timeCaps.wPeriodMin);
    this->timerId = 0;
    return TRUE;
}

void CALLBACK MidiTimer::DefaultTimerCallback(u32 uTimerID, u32 uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    MidiTimer *timer = (MidiTimer *)dwUser;
    timer->OnTimerElapsed();
}

u16 MidiOutput::Ntohs(u16 val)
{
    u8 tmp[2];

    tmp[0] = ((u8 *)&val)[1];
    tmp[1] = ((u8 *)&val)[0];

    return *(const u16 *)(&tmp);
}

u32 MidiOutput::SkipVariableLength(LPBYTE *curTrackDataCursor)
{
    u32 length;
    u8 tmp;

    length = 0;
    do
    {
        tmp = **curTrackDataCursor;
        *curTrackDataCursor = *curTrackDataCursor + 1;
        length = length * 0x80 + (tmp & 0x7f);
    } while ((tmp & 0x80) != 0);

    return length;
}

MidiOutput::MidiOutput()
{
    this->tracks = NULL;
    this->divisions = 0;
    this->tempo = 0;
    this->numTracks = 0;
    this->unk2c4 = 0;
    this->fadeOutVolumeMultiplier = 0;
    this->fadeOutLastSetVolume = 0;
    this->unk2d0 = 0;
    this->unk2d4 = 0;
    this->unk2d8 = 0;
    this->unk2dc = 0;
    this->fadeOutFlag = FALSE;

    for (int i = 0; i < ARRAY_SIZE_SIGNED(this->midiFileData); i++)
    {
        this->midiFileData[i] = NULL;
    }

    for (int i = 0; i < ARRAY_SIZE_SIGNED(this->midiHeaders); i++)
    {
        this->midiHeaders[i] = NULL;
    }

    this->midiFileIndex = -1;
    this->midiHeadersCursor = 0;
}

MidiOutput::~MidiOutput()
{
    StopPlayback();
    ClearTracks();
    for (i32 i = 0; i < 32; i++)
    {
        ReleaseFileData(i);
    }
}

ZunResult MidiOutput::ReadFileData(int idx, LPCSTR path)
{
    if (this->midiFileIndex == idx)
    {
        StopPlayback();
    }

    ReleaseFileData(idx);

    this->midiFileData[idx] = FileSystem::OpenFile((LPSTR)path, NULL, false);
    if (this->midiFileData[idx] == NULL)
    {
        g_GameErrorContext.Log(TH_ERR_MIDI_FAILED_TO_READ_FILE, path);
        return ZUN_ERROR;
    }

    return ZUN_SUCCESS;
}

void MidiOutput::ReleaseFileData(int idx)
{
    g_ZunMemory.Free(this->midiFileData[idx]);
    this->midiFileData[idx] = NULL;
}

void MidiOutput::ClearTracks()
{
    for (i32 trackIndex = 0; trackIndex < this->numTracks; trackIndex++)
    {
        g_ZunMemory.Free(this->tracks[trackIndex].trackData);
    }

    g_ZunMemory.Free(this->tracks);
    this->tracks = NULL;
    this->numTracks = 0;
}

#pragma var_order(trackIdx, currentCursor, currentCursorTrack, fileData, hdrLength, hdrRaw, trackLength,               \
                  endOfHeaderPointer)
ZunResult MidiOutput::ParseFile(int fileIdx)
{
    u8 hdrRaw[8];
    u32 trackLength;
    LPBYTE currentCursor, currentCursorTrack, endOfHeaderPointer;
    i32 trackIdx;
    LPBYTE fileData;
    u32 hdrLength;

    ClearTracks();
    currentCursor = this->midiFileData[fileIdx];
    fileData = currentCursor;
    if (currentCursor == NULL)
    {
        utils::DebugPrint(TH_ERR_MIDI_NOT_LOADED);
        return ZUN_ERROR;
    }

    // Read midi header chunk
    // First, read the header len
    memcpy(&hdrRaw, currentCursor, 8);

    // Get a pointer to the end of the header chunk
    currentCursor += sizeof(hdrRaw);
    hdrLength = MidiOutput::Ntohl(*(u32 *)(&hdrRaw[4]));

    endOfHeaderPointer = currentCursor;
    currentCursor += hdrLength;

    // Read the format. Only three values of format are specified:
    //  0: the file contains a single multi-channel track
    //  1: the file contains one or more simultaneous tracks (or MIDI outputs) of a
    //  sequence
    //  2: the file contains one or more sequentially independent single-track
    //  patterns
    this->format = MidiOutput::Ntohs(*(u16 *)endOfHeaderPointer);

    // Read the divisions in this track. Note that this doesn't appear to support
    // "negative SMPTE format", which happens when the MSB is set.
    this->divisions = MidiOutput::Ntohs(*(u16 *)(endOfHeaderPointer + 4));
    // Read the number of tracks in this midi file.
    this->numTracks = MidiOutput::Ntohs(*(u16 *)(endOfHeaderPointer + 2));

    // Allocate this->divisions * 32 bytes.
    this->tracks = (MidiTrack *)g_ZunMemory.Alloc(sizeof(MidiTrack) * this->numTracks, "midi");
    memset(this->tracks, 0, sizeof(MidiTrack) * this->numTracks);
    for (trackIdx = 0; trackIdx < this->numTracks; trackIdx += 1)
    {
        currentCursorTrack = currentCursor;
        currentCursor += 8;

        // Read a track (MTrk) chunk.
        //
        // First, read the length of the chunk
        trackLength = MidiOutput::Ntohl(*(u32 *)&currentCursorTrack[4]);
        this->tracks[trackIdx].trackLength = trackLength;
        this->tracks[trackIdx].trackData = (LPBYTE)g_ZunMemory.Alloc(trackLength, "midi");
        this->tracks[trackIdx].trackPlaying = TRUE;
        memcpy(this->tracks[trackIdx].trackData, currentCursor, trackLength);
        currentCursor += trackLength;
    }
    this->tempo = 1000000;
    this->midiFileIndex = fileIdx;
    utils::DebugPrint(" midi open %d\n", fileIdx);
    return ZUN_SUCCESS;
}

u32 MidiOutput::Ntohl(u32 val)
{
    u8 tmp[4];

    tmp[0] = ((u8 *)&val)[3];
    tmp[1] = ((u8 *)&val)[2];
    tmp[2] = ((u8 *)&val)[1];
    tmp[3] = ((u8 *)&val)[0];

    return *(const u32 *)tmp;
}

ZunResult MidiOutput::LoadFile(LPCSTR midiPath)
{
    if (ReadFileData(0x1f, midiPath) != ZUN_SUCCESS)
    {
        return ZUN_ERROR;
    }

    ParseFile(0x1f);
    ReleaseFileData(0x1f);

    return ZUN_SUCCESS;
}

void MidiOutput::LoadTracks()
{
    i32 trackIndex;
    MidiTrack *track = this->tracks;

    this->fadeOutVolumeMultiplier = 1.0;
    this->unk2dc = 0;
    this->fadeOutFlag = FALSE;
    this->volume = 0;
    this->unk130 = 0;

    for (trackIndex = 0; trackIndex < this->numTracks; trackIndex++, track++)
    {
        track->curTrackDataCursor = track->trackData;
        track->startTrackDataMaybe = track->curTrackDataCursor;
        track->trackPlaying = TRUE;
        track->trackLengthOther = MidiOutput::SkipVariableLength(&track->curTrackDataCursor);
    }
}

ZunResult MidiOutput::Play()
{
    if (this->tracks == NULL)
    {
        return ZUN_ERROR;
    }

    LoadTracks();
    this->midiOutDev.OpenDevice(MIDI_MAPPER);
    StartTimerImpl(1, NULL, NULL);
    utils::DebugPrint(" midi play\n");

    return ZUN_SUCCESS;
}

ZunResult MidiOutput::StopPlayback()
{
    if (this->tracks == NULL)
    {
        return ZUN_ERROR;
    }

    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->midiHeaders); i++)
    {
        if (this->midiHeaders[i] != NULL)
        {
            UnprepareHeader(this->midiHeaders[i]);
        }
    }

    StopTimerImpl();
    this->midiOutDev.Close();
    this->midiFileIndex = -1;

    return ZUN_SUCCESS;
}

ZunResult MidiOutput::UnprepareHeader(LPMIDIHDR pmh)
{
    if (pmh == NULL)
    {
        utils::DebugPrint("error :\r\n");
    }

    if (this->midiOutDev.handle == NULL)
    {
        utils::DebugPrint("error :\r\n");
    }

    for (i32 i = 0; i < ARRAY_SIZE_SIGNED(this->midiHeaders); i++)
    {
        if (this->midiHeaders[i] == pmh)
        {
            this->midiHeaders[i] = NULL;
            goto success;
        }
    }

    return ZUN_ERROR;

success:
    MMRESULT res = midiOutUnprepareHeader(this->midiOutDev.handle, pmh, sizeof(*pmh));
    if (res != MMSYSERR_NOERROR)
    {
        utils::DebugPrint("error :\r\n");
    }

    g_ZunMemory.Free(pmh->lpData);
    g_ZunMemory.Free(pmh);
    return ZUN_SUCCESS;
}

ZunResult MidiOutput::SetFadeOut(u32 ms)
{
    this->fadeOutVolumeMultiplier = 0.0;
    this->fadeOutInterval = ms;
    this->fadeOutElapsedMS = 0;
    this->unk2dc = 0;
    this->fadeOutFlag = TRUE;

    return ZUN_SUCCESS;
}

#pragma var_order(trackIndex, local_14, trackLoaded)
void MidiOutput::OnTimerElapsed()
{
    BOOL trackLoaded = FALSE;
    ULONGLONG local_14 = this->unk130 + (this->volume * this->divisions * 1000) / this->tempo;
    if (this->fadeOutFlag != FALSE)
    {
        if (this->fadeOutElapsedMS < this->fadeOutInterval)
        {
            this->fadeOutVolumeMultiplier = 1.0f - (f32)this->fadeOutElapsedMS / (f32)this->fadeOutInterval;
            if ((u32)(this->fadeOutVolumeMultiplier * 128.0f) != this->fadeOutLastSetVolume)
            {
                FadeOutSetVolume(0);
            }
            this->fadeOutLastSetVolume = this->fadeOutVolumeMultiplier * 128.0f;
            this->fadeOutElapsedMS++;
        }
        else
        {
            this->fadeOutVolumeMultiplier = 0.0;
            return;
        }
    }
    i32 trackIndex;
    for (trackIndex = 0; trackIndex < this->numTracks; trackIndex++)
    {
        if (this->tracks[trackIndex].trackPlaying)
        {
            trackLoaded = TRUE;
            while (this->tracks[trackIndex].trackPlaying)
            {
                if (this->tracks[trackIndex].trackLengthOther <= local_14)
                {
                    ProcessMsg(&this->tracks[trackIndex]);
                    local_14 = this->unk130 + (this->volume * this->divisions * 1000 / this->tempo);
                    continue;
                }
                break;
            }
        }
    }
    this->volume++;
    if (!trackLoaded)
    {
        LoadTracks();
    }
}

#pragma var_order(nextTrackLength, idx, arg2, lVar5, opcodeLow, opcodeHigh, opcode, arg1, curTrackLength, midiHdr,     \
                  cVar1, unk24, local_2c, local_30)
void MidiOutput::ProcessMsg(MidiTrack *track)
{
    i32 lVar5;
    i32 curTrackLength, nextTrackLength;
    MidiTrack *local_30;
    MidiTrack *local_2c;
    u8 arg1, arg2;
    u8 opcode, opcodeHigh, opcodeLow;
    u8 cVar1;
    LPMIDIHDR midiHdr;
    i32 idx;
    i32 unk24;

    opcode = *track->curTrackDataCursor;
    if (opcode < MIDI_OPCODE_NOTE_OFF)
    {
        opcode = track->opcode;
    }
    else
    {
        track->curTrackDataCursor += 1;
    }
    // we AND the opcode to filter out the channel
    opcodeHigh = opcode & 0xf0;
    opcodeLow = opcode & 0x0f;

    switch (opcodeHigh)
    {
    case MIDI_OPCODE_SYSTEM_EXCLUSIVE:
        if (opcode == MIDI_OPCODE_SYSTEM_EXCLUSIVE)
        {
            if (this->midiHeaders[this->midiHeadersCursor] != NULL)
            {
                UnprepareHeader(this->midiHeaders[this->midiHeadersCursor]);
            }

            midiHdr = this->midiHeaders[this->midiHeadersCursor] =
                (LPMIDIHDR)g_ZunMemory.Alloc(sizeof(MIDIHDR), "midiHDR");
            curTrackLength = MidiOutput::SkipVariableLength(&track->curTrackDataCursor);
            memset(midiHdr, 0, sizeof(MIDIHDR));
            midiHdr->lpData = (LPSTR)g_ZunMemory.Alloc(curTrackLength + 1, "midiHDR->lpData");
            midiHdr->lpData[0] = MIDI_OPCODE_SYSTEM_EXCLUSIVE;
            midiHdr->dwFlags = 0;
            midiHdr->dwBufferLength = curTrackLength + 1;
            for (idx = 0; idx < curTrackLength; idx++)
            {
                midiHdr->lpData[idx + 1] = *track->curTrackDataCursor;
                track->curTrackDataCursor++;
            }
            if (this->midiOutDev.SendLongMsg(midiHdr))
            {
                g_ZunMemory.Free(midiHdr->lpData);
                g_ZunMemory.Free(midiHdr);
                this->midiHeaders[this->midiHeadersCursor] = NULL;
            }
            this->midiHeadersCursor += 1;
            this->midiHeadersCursor = this->midiHeadersCursor % 32;
        }
        else if (opcode == MIDI_OPCODE_SYSTEM_RESET)
        {
            // Meta-Event. In a MIDI file, SYSTEM_RESET gets reused as a
            // sort of escape code to introducde its own meta-events system,
            // which are events that make sense in the context of a MIDI
            // file, but not in the context of the MIDI protocol itself.
            cVar1 = *track->curTrackDataCursor;
            track->curTrackDataCursor += 1;
            curTrackLength = MidiOutput::SkipVariableLength(&track->curTrackDataCursor);
            // End of Track meta-event.
            if (cVar1 == 0x2f)
            {
                track->trackPlaying = 0;
                return;
            }
            // Set Tempo meta-event.
            if (cVar1 == 0x51)
            {
                this->unk130 += (this->volume * this->divisions * 1000 / this->tempo);
                this->volume = 0;
                this->tempo = 0;
                for (idx = 0; idx < curTrackLength; idx += 1)
                {
                    this->tempo += this->tempo * 0x100 + *track->curTrackDataCursor;
                    track->curTrackDataCursor += 1;
                }
                unk24 = 60000000 / this->tempo;
                break;
            }
            track->curTrackDataCursor = track->curTrackDataCursor + curTrackLength;
        }
        break;
    case MIDI_OPCODE_NOTE_OFF:
    case MIDI_OPCODE_NOTE_ON:
    case MIDI_OPCODE_POLYPHONIC_AFTERTOUCH:
    case MIDI_OPCODE_MODE_CHANGE:
    case MIDI_OPCODE_PITCH_BEND_CHANGE:
        arg1 = *track->curTrackDataCursor;
        track->curTrackDataCursor += 1;
        arg2 = *track->curTrackDataCursor;
        track->curTrackDataCursor += 1;
        break;
    case MIDI_OPCODE_PROGRAM_CHANGE:
    case MIDI_OPCODE_CHANNEL_AFTERTOUCH:
        arg1 = *track->curTrackDataCursor;
        track->curTrackDataCursor += 1;
        arg2 = 0;
        break;
    }
    switch (opcodeHigh)
    {
    case MIDI_OPCODE_NOTE_ON:
        if (arg2 != 0)
        {
            arg1 += this->unk2c4;
            this->channels[opcodeLow].keyPressedFlags[arg1 >> 3] |= (u8)(1 << (arg1 & 7));
            break;
        }
    case MIDI_OPCODE_NOTE_OFF:
        arg1 += this->unk2c4;
        this->channels[opcodeLow].keyPressedFlags[arg1 >> 3] &= (u8)(~(1 << (arg1 & 7)));
        break;
    case MIDI_OPCODE_PROGRAM_CHANGE:
        // Program Change
        this->channels[opcodeLow].instrument = arg1;
        break;
    case MIDI_OPCODE_MODE_CHANGE:
        switch (arg1)
        {
        case 0:
            // Bank Select
            this->channels[opcodeLow].instrumentBank = arg2;
            break;
        case 7:
            // Channel Volume
            this->channels[opcodeLow].channelVolume = arg2;
            lVar5 = (f32)arg2 * this->fadeOutVolumeMultiplier;
            if (lVar5 < 0)
            {
                lVar5 = 0;
            }
            else if (0x7f < lVar5)
            {
                lVar5 = 0x7f;
            }
            arg2 = this->channels[opcodeLow].modifiedVolume = lVar5;
            break;
        case 91:
            // Effects 1 Depth
            this->channels[opcodeLow].effectOneDepth = arg2;
            break;
        case 93:
            // Effects 3 Depth
            this->channels[opcodeLow].effectThreeDepth = arg2;
            break;
        case 10:
            // Pan
            this->channels[opcodeLow].pan = arg2;
            break;
        case 2:
            // Breath control
            for (local_2c = &this->tracks[0], idx = 0; idx < this->numTracks; idx += 1, local_2c += 1)
            {
                local_2c->startTrackDataMaybe = local_2c->curTrackDataCursor;
                local_2c->unk1c = local_2c->trackLengthOther;
            }
            this->tempoAtLoopPoint = this->tempo;
            this->volumeAtLoopPoint = this->volume;
            this->unk2f8 = this->unk130;
            break;
        case 4:
            // Foot controller
            for (local_30 = &this->tracks[0], idx = 0; idx < this->numTracks; idx += 1, local_30 += 1)
            {
                local_30->curTrackDataCursor = (byte *)local_30->startTrackDataMaybe;
                local_30->trackLengthOther = local_30->unk1c;
            }
            this->tempo = this->tempoAtLoopPoint;
            this->volume = this->volumeAtLoopPoint;
            this->unk130 = this->unk2f8;
        }
        break;
    }
    if (opcode < MIDI_OPCODE_SYSTEM_EXCLUSIVE)
    {
        this->midiOutDev.SendShortMsg(opcode, arg1, arg2);
    }
    track->opcode = opcode;
    nextTrackLength = MidiOutput::SkipVariableLength(&track->curTrackDataCursor);
    track->trackLengthOther = track->trackLengthOther + nextTrackLength;
}

#pragma var_order(arg1, idx, volumeByte, midiStatus, volumeClamped)
void MidiOutput::FadeOutSetVolume(i32 volume)
{
    i32 volumeClamped;
    u32 volumeByte;
    i32 idx;
    i32 arg1;
    u32 midiStatus;

    if (this->unk2d4 != 0)
    {
        return;
    }
    arg1 = 7;
    for (idx = 0; idx < ARRAY_SIZE_SIGNED(this->channels); idx += 1)
    {
        midiStatus = (u8)(idx + 0xb0);
        volumeClamped = (i32)(this->channels[idx].channelVolume * this->fadeOutVolumeMultiplier) + volume;
        if (volumeClamped < 0)
        {
            volumeClamped = 0;
        }
        else if (volumeClamped > 127)
        {
            volumeClamped = 127;
        }
        volumeByte = (u8)volumeClamped;
        this->midiOutDev.SendShortMsg(midiStatus, arg1, volumeByte);
    }
    return;
}

void DummyMidiTimer::OnTimerElapsed()
{
    QueryPerformanceCounter(&g_DummyMidiTimerPerformanceCounter);
}

void MidiTimer::StartTimer()
{
    StartTimerImpl(16, NULL, NULL);
}

void MidiTimer::StopTimer()
{
    StopTimerImpl();
}

}; // namespace th08
