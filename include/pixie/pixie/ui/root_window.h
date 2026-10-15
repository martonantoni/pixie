#pragma once

class cRootWindow : public cPixieWindow
{
    std::unique_ptr<cSpriteRenderer> mRenderer;
protected:
    cSpriteRenderer& spriteRenderer();
    void setRenderSurface(ID3D11RenderTargetView* renderSurface);
    void renderSurfaceSizeChanged(cPoint newSize);
public:
    cRootWindow();
    virtual ~cRootWindow() = default;

    virtual void render();
};