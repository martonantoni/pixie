#pragma once

#include "xaudio2.h"

class cSoundPlayer final
{
    IXAudio2* mXAudio2;
    IXAudio2MasteringVoice* mMasterVoice;
    struct cEffectData
    {
        std::vector<char> mData;
    };
    std::vector<cEffectData> mEffects;
public:
    void Initialize();
    void Play(const char* filaname);
};

extern cSoundPlayer theSoundPlayer;