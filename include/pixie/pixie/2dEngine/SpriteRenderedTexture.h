#pragma once

class cSpriteRenderedTexture: public cTexture
{
	std::unique_ptr<cPixieWindow> mSpriteContainerWindow;
	std::unique_ptr<cSpriteRenderer> mSpriteRenderer;
	typedef std::vector<std::unique_ptr<c2DRenderable>> cOwnedSprites;
	cOwnedSprites mOwnedSprites;
	cSpriteRenderedTexture(cPoint Size);
	void Cleanup();
public:
	virtual ~cSpriteRenderedTexture();
	void AddSprite(std::unique_ptr<c2DRenderable> Sprite);
	static tIntrusivePtr<cSpriteRenderedTexture> Create(const cPoint &Size);
	virtual void Update() override;
};