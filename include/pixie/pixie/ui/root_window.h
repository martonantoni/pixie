#pragma once

class cRootWindow : public cPixieWindow
{
    std::unique_ptr<cSpriteRenderer> mRenderer;
public:
    cRootWindow();
    virtual ~cRootWindow() = default;

    void render();
};