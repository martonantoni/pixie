#pragma once

class cTextureRootWindow : public cRootWindow
{
    tIntrusivePtr<cTexture> mTexture;
protected:
    virtual void PropertiesChanged(unsigned int properties) override;
public:
    cTextureRootWindow();
    virtual ~cTextureRootWindow() = default;

    tIntrusivePtr<cTexture> texture() const { return mTexture; }
};