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

    class cActiveEffect: public IXAudio2VoiceCallback
    {
        cSoundPlayer& mParent;
        std::vector<char> mAudioData;
        IXAudio2SourceVoice* mSourceVoice;
        virtual void OnStreamEnd() override;
        virtual void OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}
        virtual void OnVoiceProcessingPassEnd() override {}
        virtual void OnBufferStart(void* pBufferContext) override {}
        virtual void OnBufferEnd(void* pBufferContext) override {}
        virtual void OnLoopEnd(void* pBufferContext) override {}
        virtual void OnVoiceError(void* pBufferContext, HRESULT Error) override {}
    public:
        cActiveEffect(cSoundPlayer& parent, IXAudio2SourceVoice* sourceVoice);
        ~cActiveEffect();
    };
    void effectPlayDone(cActiveEffect* effect);
    cThread* mEffectDestroyerThread;
    cNativeEvent mGotEffectToDestroy{ cNativeEvent::AutoReset };
    std::vector<cActiveEffect *> mDoneEffectsReading, mDoneEffectsWriting;
    cMutex mDoneEffectMutex;
    cRegisteredIDList mListeningIDs;
public:
    void Initialize();
    void Play(const char* filaname);
};

extern cSoundPlayer theSoundPlayer;