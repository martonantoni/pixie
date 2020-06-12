#pragma once

#include "xaudio2.h"

class cSoundPlayer final
{
    IXAudio2* mXAudio2;
    IXAudio2MasteringVoice* mMasterVoice;
public:
    void Initialize();
    void Play(const char* filaname);
};

extern cSoundPlayer theSoundPlayer;