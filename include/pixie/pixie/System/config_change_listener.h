#pragma once

class cConfigChangeListener
{
    cRegisteredID mListenerID;
    std::function<void()> mCallback;
public:
    cConfigChangeListener()
    {
#ifdef _DEBUG
        mListenerID = theMessageCenter.registerListener("pixie.configuration_changed", [this]() 
            { 
                if (mCallback)
                    mCallback();
            });
#endif
    }
    void setup(std::function<void()> callback) { mCallback = std::move(callback); }
};