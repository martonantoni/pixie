#include "StdAfx.h"

#include "RiffFile.hpp"

cSoundPlayer theSoundPlayer;

void cSoundPlayer::Initialize()
{
    mEffectDestroyerThread = theThreadServer->GetThread("effect_destroyer"s);
    mListeningIDs.emplace_back(mEffectDestroyerThread->AddEventHandler(
        [this]() 
        { 
            {
                cMutexGuard guard(mDoneEffectMutex);
                std::swap(mDoneEffectsReading, mDoneEffectsReading);
            }
            for (auto doneEffect : mDoneEffectsReading)
            {
                delete doneEffect;
            }
            mDoneEffectsReading.clear();
        }, &mGotEffectToDestroy));
    HRESULT hr;
    if (FAILED(hr = XAudio2Create(&mXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
    {
        RELEASE_ASSERT_EXT(false, "XAudio2Create failed");
    }

    if (FAILED(hr =mXAudio2->CreateMasteringVoice(&mMasterVoice)))
    {
        RELEASE_ASSERT_EXT(false, "CreateMasteringVoice failed");
    }
}

void cSoundPlayer::Play(const char* filename)
{
    cRiffFile file(filename);
    
    RELEASE_ASSERT(file);
    auto wfxChunkData = file.ReadChunkData('fmt ');
    auto dataChunkData = file.ReadChunkData('data');
    if (!wfxChunkData.empty() && !dataChunkData.empty())
    {
        XAUDIO2_BUFFER buffer = { 0 };
        buffer.AudioBytes = dataChunkData.size();  //buffer containing audio data
        buffer.pAudioData = (const BYTE*)dataChunkData.data();  //size of the audio buffer in bytes
        buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

        auto activeEffect = std::make_unique<cActiveEffect>(*this);
        IXAudio2SourceVoice* pSourceVoice;
        if (FAILED(mXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)wfxChunkData.data(), 0, XAUDIO2_DEFAULT_FREQ_RATIO, activeEffect.get())))
            return;

        if (FAILED(pSourceVoice->SubmitSourceBuffer(&buffer)))
            return;

        if (FAILED(pSourceVoice->Start(0)))
            return;

        activeEffect->Init(pSourceVoice, std::move(dataChunkData));    
        activeEffect.release();
    }
}

void cSoundPlayer::effectPlayDone(cActiveEffect* effect)
{
    {
        cMutexGuard guard(mDoneEffectMutex);
        mDoneEffectsWriting.emplace_back(effect);
    }
    mGotEffectToDestroy.Set();
}

void cSoundPlayer::cActiveEffect::OnStreamEnd()
{
    mParent.effectPlayDone(this);
}

cSoundPlayer::cActiveEffect::cActiveEffect(cSoundPlayer& parent)
    : mParent(parent)
{
}

cSoundPlayer::cActiveEffect::~cActiveEffect()
{
    mSourceVoice->DestroyVoice();
}


