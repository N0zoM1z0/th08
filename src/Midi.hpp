#pragma once

#include "ZunResult.hpp"
#include "inttypes.hpp"
#include <windows.h>

namespace th08
{
struct MidiTimer
{
  public:
    MidiTimer();
    ~MidiTimer();

    virtual void OnTimerElapsed();

    UINT StartTimerImpl(u32 delay, LPTIMECALLBACK cb, DWORD_PTR data);
    BOOL StopTimerImpl();

    static void CALLBACK DefaultTimerCallback(u32 uTimerID, u32 uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

    void StartTimer();
    void StopTimer();

  private:
    UINT timerId;
    TIMECAPS timeCaps;
};
C_ASSERT(sizeof(MidiTimer) == 0x10);

enum MidiOpcode
{
    MIDI_OPCODE_CHANNEL_1 = 0x01,
    MIDI_OPCODE_CHANNEL_2 = 0x02,
    MIDI_OPCODE_CHANNEL_3 = 0x03,
    MIDI_OPCODE_CHANNEL_4 = 0x04,
    MIDI_OPCODE_CHANNEL_5 = 0x05,
    MIDI_OPCODE_CHANNEL_6 = 0x06,
    MIDI_OPCODE_CHANNEL_7 = 0x07,
    MIDI_OPCODE_CHANNEL_8 = 0x08,
    MIDI_OPCODE_CHANNEL_9 = 0x09,
    MIDI_OPCODE_CHANNEL_A = 0x0A,
    MIDI_OPCODE_CHANNEL_B = 0x0B,
    MIDI_OPCODE_CHANNEL_C = 0x0C,
    MIDI_OPCODE_CHANNEL_D = 0x0D,
    MIDI_OPCODE_CHANNEL_E = 0x0E,
    MIDI_OPCODE_CHANNEL_F = 0x0F,
    MIDI_OPCODE_NOTE_OFF = 0x80,
    MIDI_OPCODE_NOTE_ON = 0x90,
    MIDI_OPCODE_POLYPHONIC_AFTERTOUCH = 0xA0,
    MIDI_OPCODE_CONTROL_CHANGE = 0xB0,
    MIDI_OPCODE_PROGRAM_CHANGE = 0xC0,
    MIDI_OPCODE_CHANNEL_AFTERTOUCH = 0xD0,
    MIDI_OPCODE_PITCH_BEND_CHANGE = 0xE0,
    MIDI_OPCODE_SYSTEM_EXCLUSIVE = 0xF0,
    MIDI_OPCODE_MIDI_TIME_CODE_QTR_FRAME = 0xF1,
    MIDI_OPCODE_SONG_POSITION_POINTER = 0xF2,
    MIDI_OPCODE_SONG_SELECT = 0xF3,
    MIDI_OPCODE_RESERVED_F4 = 0xF4,
    MIDI_OPCODE_RESERVED_F5 = 0xF5,
    MIDI_OPCODE_TUNE_REQUEST = 0xF6,
    MIDI_OPCODE_END_OF_SYSEX = 0xF7,
    MIDI_OPCODE_TIMING_CLOCK = 0xF8,
    MIDI_OPCODE_RESERVED_F9 = 0xF9,
    MIDI_OPCODE_START = 0xFA,
    MIDI_OPCODE_CONTINUE = 0xFB,
    MIDI_OPCODE_STOP = 0xFC,
    MIDI_OPCODE_RESERVED_FD = 0xFD,
    MIDI_OPCODE_ACTIVE_SENSING = 0xFE,
    MIDI_OPCODE_SYSTEM_RESET = 0xFF,
};

enum MidiController
{
    MIDI_CONTROLLER_BANK_SELECT = 0,
    MIDI_CONTROLLER_LOOP_START = 2,
    MIDI_CONTROLLER_LOOP_END = 4,
    MIDI_CONTROLLER_CHANNEL_VOLUME = 7,
    MIDI_CONTROLLER_PAN = 10,
    MIDI_CONTROLLER_EFFECT_ONE_DEPTH = 91,
    MIDI_CONTROLLER_EFFECT_THREE_DEPTH = 93,
};

enum MidiMetaEvent
{
    MIDI_META_END_OF_TRACK = 0x2f,
    MIDI_META_SET_TEMPO = 0x51,
};

struct MidiTrack
{
    BOOL trackPlaying;
    i32 nextEventTick;
    u32 dataSize;
    u8 runningStatus;
    LPBYTE data;
    u8 *cursor;
    u8 *loopCursor;
    u32 loopNextEventTick;
};
C_ASSERT(sizeof(MidiTrack) == 0x20);
C_ASSERT(offsetof(MidiTrack, nextEventTick) == 0x4);
C_ASSERT(offsetof(MidiTrack, runningStatus) == 0xc);
C_ASSERT(offsetof(MidiTrack, cursor) == 0x14);
C_ASSERT(offsetof(MidiTrack, loopCursor) == 0x18);
C_ASSERT(offsetof(MidiTrack, loopNextEventTick) == 0x1c);

class MidiDevice
{
  public:
    MidiDevice();
    ~MidiDevice();

    BOOL OpenDevice(UINT uDeviceId);
    ZunResult Close();
    BOOL SendLongMsg(LPMIDIHDR pmh);
    BOOL SendShortMsg(u8 midiStatus, u8 firstByte, u8 secondByte);

    HMIDIOUT handle;

  private:
    u32 deviceId;
};
C_ASSERT(sizeof(MidiDevice) == 0x8);

struct MidiChannel
{
    u8 keyPressedFlags[16];
    u8 instrument;
    u8 instrumentBank;
    u8 pan;
    u8 effectOneDepth;
    u8 effectThreeDepth;
    u8 channelVolume;
    u8 modifiedVolume;
};
C_ASSERT(sizeof(MidiChannel) == 0x17);

class MidiOutput : MidiTimer
{
  public:
    MidiOutput();
    ~MidiOutput();

    void OnTimerElapsed();

    ZunResult UnprepareHeader(LPMIDIHDR pmh);

    ZunResult StopPlayback();
    void LoadTracks();
    void ClearTracks();
    ZunResult ReadFileData(int idx, LPCSTR path);
    void ReleaseFileData(int idx);
    void ProcessMsg(MidiTrack *track);

    ZunResult ParseFile(int fileIdx);
    ZunResult LoadFile(LPCSTR midiPath);
    ZunResult Play();

    ZunResult SetFadeOut(u32 ms);
    void FadeOutSetVolume(i32 volumeOffset);

    static u16 Ntohs(u16 val);
    static u32 SkipVariableLength(LPBYTE *curTrackDataCursor);

    void PlayFile(i32 idx)
    {
        this->StopPlayback();
        this->ParseFile(idx);
        this->Play();
    }

    static u32 Ntohl(u32 val);

  private:
    i32 activeFileIndex;
    LPMIDIHDR pendingLongMessageHeaders[32];
    i32 pendingLongMessageHeaderCursor;
    LPBYTE fileData[32];
    i32 numTracks;
    u32 fileFormat;
    i32 ticksPerQuarterNote;
    i32 microsecondsPerQuarterNote;
    ULONGLONG elapsedMillisecondsAtCurrentTempo;
    ULONGLONG elapsedTicksBeforeTempoChange;
    MidiTrack *tracks;
    MidiDevice outputDevice;
    u8 unconsumedPerChannelBytes144[16];
    MidiChannel channels[16];
    i8 noteTranspose;
    f32 fadeOutVolumeMultiplier;
    u32 fadeOutLastSetVolume;
    u32 constructorClearedDword2D0;
    u32 volumeUpdatesSuppressed;
    u32 constructorClearedDword2D8;
    u32 resetOnlyFadeState2DC;
    BOOL fadeOutActive;
    i32 fadeOutDurationMs;
    i32 fadeOutElapsedMs;
    u32 tempoAtLoopPoint;
    ULONGLONG elapsedMillisecondsAtLoopPoint;
    ULONGLONG elapsedTicksAtLoopPoint;
};
C_ASSERT(sizeof(MidiOutput) == 0x300);

class DummyMidiTimer : public MidiTimer
{
  public:
    void OnTimerElapsed();

  private:
    u32 unconsumedDword10;
};

C_ASSERT(sizeof(DummyMidiTimer) == 0x14);

}; // namespace th08
