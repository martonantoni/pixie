#pragma once

class cRootWindow : public cPixieWindow
{
    std::unique_ptr<cSpriteRenderer> mRenderer;
protected:
    void setRenderSurface(ID3D11RenderTargetView* renderSurface);
    void renderSurfaceSizeChanged(cPoint newSize);
public:
    cRootWindow();
    virtual ~cRootWindow() = default;

    void render();
};