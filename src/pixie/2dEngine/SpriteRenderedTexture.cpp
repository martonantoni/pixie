#include "StdAfx.h"
#include "pixie/pixie/i_pixie.h"
cSpriteRenderedTexture::cSpriteRenderedTexture(cPoint Size)
	: cTexture(Size)
	, mSpriteContainerWindow(std::make_unique<cPixieWindow>())
{
	mNeedUpdateBeforeUse=true;
	mSpriteRenderer=std::make_unique<cSpriteRenderer>(*mSpriteContainerWindow);
}

cSpriteRenderedTexture::~cSpriteRenderedTexture()
{
}

tIntrusivePtr<cSpriteRenderedTexture> cSpriteRenderedTexture::Create(const cPoint &Size)
{
	tIntrusivePtr<cSpriteRenderedTexture> NewTexture(new cSpriteRenderedTexture(Size));
	NewTexture->InitForRenderTarget(Size);
	NewTexture->mSpriteRenderer->SetRenderSurface(NewTexture->mSurface);
	NewTexture->mSpriteRenderer->SetClearBeforeRender(true);
	return NewTexture;
}

void cSpriteRenderedTexture::AddSprite(std::unique_ptr<cSpriteBase> Sprite)
{
	Sprite->SetWindow(mSpriteContainerWindow.get());
	Sprite->Show();
	mOwnedSprites.push_back(std::move(Sprite));
	mNeedUpdateBeforeUse=true;
}

void cSpriteRenderedTexture::Update()
{
	mNeedUpdateBeforeUse=false;
	mSpriteRenderer->Render();
	Cleanup();
}

void cSpriteRenderedTexture::Cleanup()
{
	mOwnedSprites.clear();
	mSpriteContainerWindow.reset();
	mSpriteRenderer.reset();
}